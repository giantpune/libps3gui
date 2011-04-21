#include <sys/file.h>
//#include <sys/sysfs.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "devicethread.h"
#include "fileops.h"
#include "stringstuff.h"
#include "utils.h"

namespace FileOps
{

bool Exists( const std::string &path )
{
	return Exists( path.c_str() );
}

bool Exists( const char* path )
{
	//printf("Exists( %s )\n", path );
	sysFSStat entry;
	return( sysFsStat( path, &entry ) == 0 );
}

bool IsDir( const std::string &path )
{
	return IsDir( path.c_str() );
}

bool IsDir( const char* path )
{
	sysFSStat entry;
	if( sysFsStat( path, &entry ) )
		return false;
	bool ret = ( ( entry.st_mode & S_IFDIR ) != 0 );
	return ret;
}

u8* ReadFile( const std::string &path, u32 *size )
{
	return ReadFile( path.c_str(), size );
}

u8* ReadFile( const char* path, u32 *size )
{
	u8* ret = NULL;
	sysFSStat stat;
	s32 fd;
	u64 read;
	if( size )
		*size = 0;

	int i = sysFsStat( path, &stat );

	if( i < 0 || !stat.st_size || ( stat.st_mode & S_IFDIR ) )
	{
		//printf("sysFsStat( %s ): %i\n", path, i );
		return ret;
	}
	ret = (u8*)malloc( stat.st_size );
	if( !ret )
	{
		printf("failed to allocate %u bytes\n", (unsigned int)stat.st_size );
		return NULL;

	}
	i = sysFsOpen( path, SYS_O_RDONLY, &fd, NULL, 0 );
	if( i )
	{
		printf("sysFsOpen( %s ) %i\n", path, i );
		free( ret );
		return NULL;
	}
	i = sysFsRead( fd, ret, stat.st_size, &read );
	if( i || read != stat.st_size )
	{
		printf("sysFsRead( %u ): %i ( %u )\n", (unsigned int)stat.st_size, i, (unsigned int)read );
		free( ret );
		sysFsClose( fd );
		return NULL;
	}
	sysFsClose( fd );

	if( size )
		*size = stat.st_size;

	return ret;
}

Buffer ReadFileToBuffer( const std::string &path )
{
	Buffer ret;
	sysFSStat stat;
	s32 fd;
	u64 read;

	int i = sysFsStat( path.c_str(), &stat );

	if( i < 0 || !stat.st_size || ( stat.st_mode & S_IFDIR ) )
	{
		//printf("sysFsStat( %s ): %08x\n", path.c_str(), i );
		return ret;
	}
	ret.Resize( stat.st_size );
	if( ret.IsEmpty() )
	{
		printf("ReadFileToBuffer() failed to resize buffer\n");
		return ret;
	}
	i = sysFsOpen( path.c_str(), SYS_O_RDONLY, &fd, NULL, 0 );
	if( i )
	{
		printf("sysFsOpen( %s ) %i\n", path.c_str(), i );
		ret.Free();
		return ret;
	}
	i = sysFsRead( fd, ret.Data(), stat.st_size, &read );
	if( i || read != stat.st_size )
	{
		printf("sysFsRead( %u ): %i ( %u )\n", (unsigned int)stat.st_size, i, (unsigned int)read );
		ret.Free();
		sysFsClose( fd );
		return ret;
	}
	sysFsClose( fd );
	return ret;
}

bool WriteFile( const std::string &path, const u8* stuff, u32 size )
{
	//hexdump( stuff, size );
	s32 fd;
	int i;
	u64 written;

	i = sysFsOpen( path.c_str(), SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd, NULL, 0 );
	if( i )
	{
		printf("sysFsOpen( %s ): %i\n", path.c_str(), i );
		return false;
	}
	i = sysFsWrite( fd, (void*)stuff, size, &written );
	if( i )
	{
		sysFsClose( fd );
		sysFsUnlink( path.c_str() );
		printf("sysFsWrite( %s ): %i\n", path.c_str(), i );
		return false;
	}
	sysFsClose( fd );
	if( written != size )
	{
		printf("WriteFile() failed to write all the data to \"%s\"\n", path.c_str() );
		sysFsUnlink( path.c_str() );
		return false;
	}
	return true;
}

std::vector<std::string> ReadDir( const std::string &path, u32 flags )
{
	int i;
	s32 fd;
	sysFSDirent entry;
	u64 read;

	std::vector<std::string> ret;
	std::vector<std::string> files;
	std::vector<std::string> dirs;

	//open dir
	i = sysFsOpendir( path.c_str(), &fd );
	if( i )
	{
		printf("sysFsOpendir( %s ): %i\n", path.c_str(), i );
		return ret;
	}

	while( !sysFsReaddir( fd, &entry , &read ) && read > 0 )
	{
		//printf("namelen: %i  type: %u  name: %s\n", entry.d_namlen, entry.d_type, entry.d_name );
		if( !entry.d_namlen )//wtf?!
			continue;

		std::string str = entry.d_name;
		if( entry.d_name[ 0 ] == '.' )
		{
			//dot
			if( entry.d_name[ 1 ] == 0 )
			{
				if( !( flags & DIR_SKIP_DOT ) )
					dirs.push_back( str );

				continue;
			}
			//dotdot
			if( entry.d_name[ 1 ] == '.' && entry.d_name[ 2 ] == 0 )
			{
				if( !( flags & DIR_SKIP_DOTDOT ) )
					dirs.push_back( str );

				continue;
			}

			//must be a hidden file if we made it this far
			if( flags & DIR_SKIP_HIDDEN )
				continue;
		}


		//sub dirs
		if( entry.d_type == 1 && ( flags & DIR_DIRS ) )
		{
			dirs.push_back( str );
			continue;
		}
		//files
		if( entry.d_type == 2 && ( flags & DIR_FILES ) )
		{
			files.push_back( str );
			continue;
		}
	}

	//close dir
	sysFsClosedir( fd );

	//sort
	std::sort( dirs.begin(), dirs.end() );
	std::sort( files.begin(), files.end() );
	ret.insert( ret.begin(), dirs.begin(), dirs.end() );
	ret.insert( ret.end(), files.begin(), files.end() );

	return ret;
}

std::vector<std::string> DevList( u32 flags )
{
	int i;
	s32 fd;
	sysFSDirent entry;
	u64 read;

	std::vector<std::string> ret;

	//open dir
	i = sysFsOpendir( "/", &fd );
	if( i )
	{
		printf("sysFsOpendir( \"/\" ): %i\n", i );
		return ret;
	}

	while( !sysFsReaddir( fd, &entry , &read ) && read > 0 )
	{
		//printf("namelen: %i  type: %u  name: %s\n", entry.d_namlen, entry.d_type, entry.d_name );
		if( !entry.d_namlen )//wtf?!
			continue;

		std::string str = "/";
		str += entry.d_name;
		if( !strncmp( entry.d_name, "dev_flash", 9 ) )
		{
			if( flags & DEV_INTERNAL_FLASH )
				ret.push_back( str );
			continue;
		}
		if( !strncmp( entry.d_name, "dev_hdd", 7 ) )
		{
			if( flags & DEV_INTERNAL_HDD )
				ret.push_back( str );
			continue;
		}
		if( !strncmp( entry.d_name, "dev_usb", 7 ) )
		{
			if( flags & DEV_USB_HDD )
				ret.push_back( str );
			continue;
		}
		if( !strcmp( entry.d_name, "dev_sd" ) )
		{
			if( flags & DEV_SD )
				ret.push_back( str );
			continue;
		}
		if( !strcmp( entry.d_name, "dev_bdvd" ) )
		{
			if( flags & DEV_BDVD )
				ret.push_back( str );
			continue;
		}
		if( !strcmp( entry.d_name, "dev_ms" ) )
		{
			if( flags & DEV_MEMORY_STICK )
				ret.push_back( str );
			continue;
		}
		if( !strcmp( entry.d_name, "dev_cf" ) )
		{
			if( flags & DEV_COMPACT_FLASH )
				ret.push_back( str );
			continue;
		}
	}

	//close dir
	sysFsClosedir( fd );

	//sort
	std::sort( ret.begin(), ret.end() );

	return ret;
}

float FreeSpace( const std::string &path, u64 unit )
{
	//cout << "FreeSpace( " << path << ", " << hex << unit << " )" << endl;
	float ret;
	u32 blockSize;
	u64 freeSize;
	u64 bytes;

	if( !unit )
		return 0;

	int i = sysFsGetFreeSize( path.c_str(), &blockSize, &freeSize );
	if( i )
	{
		//cout << "sysFsGetFreeSize( " << path << " ): " << i << endl;
		return 0;
	}

	bytes = ( (u64) blockSize * freeSize );

	//cout << hex << "bl: " << blockSize << "\tfr: " << freeSize << "\tby: " << bytes << endl;
	ret = (float)bytes / (float)unit;

	return ret;
}

std::string HomeDir()
{
	std::string ret = "/dev_hdd0/game/";
	ret += APP_ID;
	ret += "/USRDIR/";
	if( !Exists( ret ) )
		ret.clear();
	return ret;
}

u64 FileSize( std::string path, u8 depth )
{
	u64 ret = 0;
	sysFSStat stat;
	if( sysFsStat( path.c_str(), &stat ) )
		return 0;
	//its a directory
	if( stat.st_mode & S_IFDIR )
	{
		if( path.at( path.size() - 1 ) != '/' )
			path += "/";

		int i;
		s32 fd;
		sysFSDirent entry;
		u64 read;
		//open dir
		i = sysFsOpendir( path.c_str(), &fd );
		if( i )
		{
			printf("sysFsOpendir( %s ): %i\n", path.c_str(), i );
			return ret;
		}
		while( !sysFsReaddir( fd, &entry , &read ) && read > 0 )
		{
			if( !entry.d_namlen || !strcmp( entry.d_name, "." ) || !strcmp( entry.d_name, ".." ) )
			{
				continue;
			}

			std::string subPath = path + entry.d_name;

			if( sysFsStat( subPath.c_str(), &stat ) )
				continue;

			if( stat.st_mode & S_IFDIR )
			{
				if( depth < MAX_RECURSE_DEPTH )
					ret += FileSize( subPath, depth + 1 );
				else
				{
					printf("FileSize( \"%s\" ): max recursion depth reached\n", subPath.c_str() );
				}
			}
			else
			{
				ret += stat.st_size;
			}
		}
		sysFsClosedir( fd );
	}
	else//its a file
	{
		ret = stat.st_size;
	}
	return ret;
}

bool RecurseDeletePath( std::string path, u8 depth )
{
	bool ret = true;
	int i;
	sysFSStat stat;
	if( sysFsStat( path.c_str(), &stat ) )
		return false;

	//its a directory
	if( stat.st_mode & S_IFDIR )
	{
		if( path.at( path.size() - 1 ) != '/' )
			path += "/";

		s32 fd;
		sysFSDirent entry;
		u64 read;
		//open dir
		i = sysFsOpendir( path.c_str(), &fd );
		if( i )
		{
			printf("sysFsOpendir( %s ): %i\n", path.c_str(), i );
			return false;
		}
		while( !sysFsReaddir( fd, &entry , &read ) && read > 0 )
		{
			if( !entry.d_namlen || !strcmp( entry.d_name, "." ) || !strcmp( entry.d_name, ".." ) )
			{
				continue;
			}

			std::string subPath = path + entry.d_name;

			if( sysFsStat( subPath.c_str(), &stat ) )
				continue;

			if( stat.st_mode & S_IFDIR )
			{
				if( depth < MAX_RECURSE_DEPTH )
				{
					if( !RecurseDeletePath( subPath, depth ) )
					{
						ret = false;
						break;
					}
				}
				else
				{
					printf("RecurseDeletePath( \"%s\" ): max recursion depth reached\n", subPath.c_str() );
					ret = false;
					break;
				}
			}
			else
			{
				i = sysFsUnlink( subPath.c_str() );
				if( i )
				{
					printf("sysFsUnlink( %s ): %i\n", subPath.c_str(), i );
					ret = false;
					break;
				}
			}
		}
		sysFsClosedir( fd );
		if( ret )
		{
			i = sysFsRmdir( path.c_str() );
			if( i )
			{
				printf("sysFsRmdir( %s ): %08x\n", path.c_str(), i );
				ret = false;
			}
		}
	}
	else//its a file
	{
		i = sysFsUnlink( path.c_str() );
		if( i )
		{
			printf("sysFsUnlink( %s ): %i\n", path.c_str(), i );
			ret = false;
		}
	}
	return ret;
}


}
