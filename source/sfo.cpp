

#include <algorithm>
#include <stdio.h>
#include <string.h>
#include "fileops.h"
#include "sfo.h"
#include "stringstuff.h"
#include "utils.h"

Sfo::Sfo( const string& path )
{
	if( !path.empty() )
		Load( path );
}

Sfo::~Sfo()
{
	entries.clear();
}

bool Sfo::Load( const u8* ptr, u32 size )
{
	//printf("loading:\n");
	//hexdump( ptr, size );
	entries.clear();
	u32 offset = 0;
	u32 tmp;
	u32 kOffset;
	u32 vOffset;
	u32 numEntries;
	memcpy( &tmp, ptr, 4 ); offset += 4; tmp = le32( tmp );
	if( tmp != 0x46535000 )
	{
		printf("Sfo::Load(): bad file magic word (0x%08x)\n", tmp );
		return false;
	}
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; tmp = le32( tmp );
	if( tmp != 0x101 )
	{
		printf("Sfo::Load(): bad file version (0x%08x)\n", tmp );
		return false;
	}
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; kOffset = le32( tmp );
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; vOffset = le32( tmp );
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; numEntries = le32( tmp );
	//printf("kOffset: %08x\nvOffset: %08x\nnumEntries: %08x\n", kOffset, vOffset, numEntries );

	//quick sanity check
	if( kOffset >= size
		|| vOffset >= size
		|| offset + ( numEntries * 16 ) >= size )
	{
		printf("Sfo::Load() out of range\n");
		return false;
	}

	//read entries
	//u32 test = 0;
	for( u32 i = 0; i < numEntries; i++ )
	{
		u8 type;
		u16 kNameOffset;
		u32 dSize;
		u32 dSizePadded;
		u32 dOffset;
		memcpy( &tmp, ptr + offset, 4 ); offset += 4; tmp = le32( tmp );
		memcpy( &dSize, ptr + offset, 4 ); offset += 4; dSize = le32( dSize );
		memcpy( &dSizePadded, ptr + offset, 4 ); offset += 4; dSizePadded = le32( dSizePadded );
		memcpy( &dOffset, ptr + offset, 4 ); offset += 4; dOffset = le32( dOffset );
		//printf("[%i] %08x %08x %08x %08x\n", i, tmp, dSize, dSizePadded, dOffset );
		type = ( ( tmp >> 24 ) & 0xff );
		kNameOffset = ( tmp & 0xffff );

		//sanity check
		if( kOffset + kNameOffset + 1 > size
			|| ( type != 0 && type != 2 && type != 4 )
			|| dSize + vOffset + dOffset > size
			|| ( type == 4 && dSize != 4 ) )
		{
			printf("Sfo::Load() entry error\n");
			entries.clear();
			return false;
		}
		//test += dSizePadded;
		//printf("type: %02x   len: %08x  maxlen: %08x  kName: test: %08x\"%s\"\n", type, dSize, dSizePadded, test, ptr + kOffset + kNameOffset );

		SfoEntry entry;
		entry.name = (const char*)( ptr + kOffset + kNameOffset );
		entry.dataType = type;
		if( type == 4 )
		{
			memcpy( &tmp, ptr + vOffset + dOffset, 4 );
			entry.num = le32( tmp );
		}
		else
		{
			entry.buf.SetData( ptr + vOffset + dOffset, dSize );
			if( entry.buf.IsEmpty() )
			{
				printf("Sfo::Load() out of memory\n");
				entries.clear();
				return false;
			}
		}
		entries.push_back( entry );
	}
	return true;
}

bool Sfo::Load( const string& path )
{
	entries.clear();
	Buffer buf = FileOps::ReadFileToBuffer( path );
	if( buf.IsEmpty() )
		return false;

	//printf("loading:\n");
	//buf.Dump();

	u8* ptr = buf.Data();
	u32 offset = 0;
	u32 tmp;
	u32 kOffset;
	u32 vOffset;
	u32 numEntries;
	memcpy( &tmp, ptr, 4 ); offset += 4; tmp = le32( tmp );
	if( tmp != 0x46535000 )
	{
		printf("Sfo::Load(): bad file magic word (0x%08x)\n", tmp );
		return false;
	}
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; tmp = le32( tmp );
	if( tmp != 0x101 )
	{
		printf("Sfo::Load(): bad file version (0x%08x)\n", tmp );
		return false;
	}
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; kOffset = le32( tmp );
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; vOffset = le32( tmp );
	memcpy( &tmp, ptr + offset, 4 ); offset += 4; numEntries = le32( tmp );
	//printf("kOffset: %08x\nvOffset: %08x\nnumEntries: %08x\n", kOffset, vOffset, numEntries );

	//quick sanity check
	if( kOffset >= buf.Size()
		|| vOffset >= buf.Size()
		|| offset + ( numEntries * 16 ) >= buf.Size() )
	{
		printf("Sfo::Load() out of range\n");
		return false;
	}

	//read entries
	//u32 test = 0;
	for( u32 i = 0; i < numEntries; i++ )
	{
		u8 type;
		u16 kNameOffset;
		u32 dSize;
		u32 dSizePadded;
		u32 dOffset;
		memcpy( &tmp, ptr + offset, 4 ); offset += 4; tmp = le32( tmp );
		memcpy( &dSize, ptr + offset, 4 ); offset += 4; dSize = le32( dSize );
		memcpy( &dSizePadded, ptr + offset, 4 ); offset += 4; dSizePadded = le32( dSizePadded );
		memcpy( &dOffset, ptr + offset, 4 ); offset += 4; dOffset = le32( dOffset );
		//printf("[%i] %08x %08x %08x %08x\n", i, tmp, dSize, dSizePadded, dOffset );
		type = ( ( tmp >> 24 ) & 0xff );
		kNameOffset = ( tmp & 0xffff );

		//sanity check
		if( kOffset + kNameOffset + 1 > buf.Size()
			|| ( type != 0 && type != 2 && type != 4 )
			|| dSize + vOffset + dOffset > buf.Size()
			|| ( type == 4 && dSize != 4 ) )
		{
			printf("Sfo::Load() entry error\n");
			entries.clear();
			return false;
		}
		//test += dSizePadded;
		//printf("type: %02x   len: %08x  maxlen: %08x  kName: test: %08x\"%s\"\n", type, dSize, dSizePadded, test, ptr + kOffset + kNameOffset );

		SfoEntry entry;
		entry.name = (const char*)( ptr + kOffset + kNameOffset );
		entry.dataType = type;
		if( type == 4 )
		{
			memcpy( &tmp, ptr + vOffset + dOffset, 4 );
			entry.num = le32( tmp );
		}
		else
		{
			entry.buf.SetData( ptr + vOffset + dOffset, dSize );
			if( entry.buf.IsEmpty() )
			{
				printf("Sfo::Load() out of memory\n");
				entries.clear();
				return false;
			}
		}
		entries.push_back( entry );
	}
	return true;
}

void Sfo::Print()
{
	printf("Sfo::Print()\n");
	printf("%u entries\n", (u32)entries.size() );
	u32 i = 0;
	//printf("\n");
	vector<SfoEntry>::iterator entry = entries.begin();
	while( entry < entries.end() )
	{
		switch( (*entry).dataType )
		{
		case 4:
			printf( "[%u] number \"%s\" 0x%08x %u\n", i, (*entry).name.c_str(), (*entry).num, (*entry).num );
			break;
		case 2:
			{
				char b[ (*entry).buf.Size() + 2 ];
				strcpy( b, (const char*)(*entry).buf.Data() );
				b[ (*entry).buf.Size() + 1 ] = 0;
				printf( "[%u] string \"%s\" \"%s\"\n", i, (*entry).name.c_str(), b );
			}
			break;
		case 0:
			printf( "[%u] binary data \"%s\" size: %08x\n", i, (*entry).name.c_str(), (*entry).buf.Size() );
			//if( (*entry).buf.Size() < 0x20 )
			//	(*entry).buf.Dump();
			break;
		default:
			printf( "[%u] unknown data type \"%s\"\n", i, (*entry).name.c_str() );
			break;
		}

		++entry;
		++i;
	}
}

static bool SfoSortPredicate( const SfoEntry &g1, const SfoEntry &g2 )
{
	return g1.name < g2.name;
}

void Sfo::Sort()
{
	std::sort( entries.begin(), entries.end(), SfoSortPredicate );
}

Buffer Sfo::Data()
{
	Buffer ret;
	if( !entries.size() )
		return ret;

	Sort();
	u32 kListSize = 0;
	u32 vListSize = 0;

	u32 vOffset[ entries.size() + 1 ];
	u32 vSize[ entries.size() + 1 ];
	u32 vSizePadded[ entries.size() + 1 ];
	u32 kOffset[ entries.size() + 1 ];

	u32 i = 0;
	u32 size = 0x14;					//size of header;
	size += ( 16 * entries.size() );	//size of entry list

	//calculate filesize
	vector<SfoEntry>::iterator entry = entries.begin();
	while( entry < entries.end() )
	{
		kOffset[ i ] = kListSize;
		vOffset[ i ] = vListSize;
		kListSize += (*entry).name.size() + 1;
		switch( (*entry).dataType )
		{
		case 4:
			vSize[ i ] = 4;
			vSizePadded[ i ] = 4;
			break;
		case 2:
			{
				vSize[ i ] = (*entry).buf.Size();
				if( (*entry).name == "LICENSE" )
				{
					vSizePadded[ i ] = RU( vSize[ i ], 0x200 );
				}
				else if( StartsWith( (*entry).name, "TITLE" ) )
				{
					if( (*entry).name == "TITLE_ID" )
						vSizePadded[ i ] = RU( vSize[ i ], 0x10 );
					else
						vSizePadded[ i ] = RU( vSize[ i ], 0x80 );
				}
				else
				{
					vSizePadded[ i ] = RU( vSize[ i ], 4 );
				}
			}
			break;
		case 0:
			vSize[ i ] = (*entry).buf.Size();
			vSizePadded[ i ] = RU( vSize[ i ], 4 );
			break;
		default:
			printf( "Sfo::Data(): unknown data type \"%s\"\n", (*entry).name.c_str() );
			return ret;
			break;
		}
		vListSize += vSizePadded[ i ];
		//printf("kOffset: %08x  vSize: %08x  vSizePadded: %08x  vListSize: %08x  %s\n", kOffset[ i ], vSize[ i ], vSizePadded[ i ], vListSize, (*entry).name.c_str() );

		++i;
		++entry;
	}

	kListSize = RU( kListSize, 4 );
	size += kListSize + vListSize;

	//allocate memory
	ret.Resize( size );
	if( ret.IsEmpty() )
		return ret;

	//write the data
	u32 tmp;
	u32 offset = 0;
	u32 ktOffset = 0x14 + ( 0x10 * entries.size() );
	u32 vtOffset = ktOffset + kListSize;
	u8* ptr = ret.Data();
	memset( ptr, 0, size );
	tmp = le32( 0x46535000 ); memcpy( ptr, &tmp, 4 ); offset += 4;							//magic word
	tmp = le32( 0x101 ); memcpy( ptr + offset, &tmp, 4 ); offset += 4;						//version
	tmp = le32( ktOffset ); memcpy( ptr + offset, &tmp, 4 ); offset += 4;					//key table offset
	tmp = le32( vtOffset ); memcpy( ptr + offset, &tmp, 4 ); offset += 4;					//value table offset
	tmp = le32( (u32)entries.size() ); memcpy( ptr + offset, &tmp, 4 ); offset += 4;		//number of entries

	i = 0;
	entry = entries.begin();
	while( entry < entries.end() )
	{
		tmp = le32( ( (*entry).dataType << 24 ) | ( 4 << 16 ) | ( kOffset[ i ] & 0xffff ) );
		memcpy( ptr + offset, &tmp, 4 ); offset += 4;

		tmp = le32( vSize[ i ] );memcpy( ptr + offset, &tmp, 4 ); offset += 4;			//value size
		tmp = le32( vSizePadded[ i ] );memcpy( ptr + offset, &tmp, 4 ); offset += 4;	//padded value size
		tmp = le32( vOffset[ i ] );memcpy( ptr + offset, &tmp, 4 ); offset += 4;		//value offset

		strcpy( (char*)( ptr + ktOffset + kOffset[ i ] ), (*entry).name.c_str() );
		switch( (*entry).dataType )
		{
		case 4:
			tmp = le32( (*entry).num );
			memcpy( ptr + vtOffset + vOffset[ i ], &tmp, 4 );
			break;
		case 2:
		case 0:
			memcpy( ptr + vtOffset + vOffset[ i ], (*entry).buf.Data(), (*entry).buf.Size() );
			break;
		}

		++i;
		++entry;
	}

	//printf("return:\n");
	//ret.Dump();

	return ret;

}

int Sfo::GetIndex( const string &key )
{
	int i = 0;
	vector<SfoEntry>::iterator entry = entries.begin();
	while( entry < entries.end() )
	{
		if( (*entry).name == key )
			return i;
		++i;
		++entry;
	}
	return -1;
}

string Sfo::GetValueStr( int idx )
{
	string ret;
	if( idx < 0 || idx > (int)entries.size() || entries.at( idx ).dataType != 2 )
		return ret;

	return (const char*)entries.at( idx ).buf.Data();
}

string Sfo::GetValueStr( const string &key )
{
	return GetValueStr( GetIndex( key ) );
}

u32 Sfo::GetValueU32( int idx )
{
	if( idx < 0 || idx > (int)entries.size() || entries.at( idx ).dataType != 4 )
		return 0xffffffff;

	return entries.at( idx ).num;
}

u32 Sfo::GetValueU32( const string &key )
{
	return GetValueU32( GetIndex( key ) );
}

u8 Sfo::GetType( int idx )
{
	if( idx < 0
		|| idx > (int)entries.size()
		|| ( entries.at( idx ).dataType != 0 && entries.at( idx ).dataType != 2 && entries.at( idx ).dataType != 4 ) )
		return 0xff;
	return entries.at( idx ).dataType;
}

u8 Sfo::GetType( const string &key )
{
	return GetType( GetIndex( key ) );
}

Buffer Sfo::GetValueBuf( int idx )
{
	if( idx < 0 || idx > (int)entries.size() || entries.at( idx ).dataType != 4 )
		return Buffer();

	return entries.at( idx ).buf;
}

Buffer Sfo::GetValueBuf( const string &key )
{
	return GetValueBuf( GetIndex( key ) );
}

void Sfo::SetValue( const string &key, u32 val )
{
	vector<SfoEntry>::iterator entry = entries.begin();
	while( entry < entries.end() )
	{
		if( (*entry).name == key )
		{
			(*entry).dataType = 4;
			(*entry).num = val;
			(*entry).buf.Free();
			return;
		}
		++entry;
	}
	//key doesnt already exist, create a new one and insert it
	SfoEntry n;
	n.name = key;
	n.num = val;
	n.dataType = 4;

	entries.push_back( n );

	//sort the list
	Sort();
}

void Sfo::SetValue( const string &key, const string &val )
{
	vector<SfoEntry>::iterator entry = entries.begin();
	while( entry < entries.end() )
	{
		if( (*entry).name == key )
		{
			(*entry).dataType = 2;
			(*entry).buf.Resize( val.size() + 1 );
			if( (*entry).buf.IsEmpty() )
			{
				printf("Sfo::SetValue() ENOMEM\n");
				return;
			}
			memcpy( (*entry).buf.Data(), val.c_str(), val.size() );
			return;
		}
		++entry;
	}
	//key doesnt already exist, create a new one and insert it
	SfoEntry n;
	n.buf.Resize( val.size() + 1 );
	if( n.buf.IsEmpty() )
	{
		printf("Sfo::SetValue() ENOMEM\n");
		return;
	}
	memcpy( n.buf.Data(), val.c_str(), val.size() );
	n.name = key;
	n.dataType = 2;

	entries.push_back( n );

	//sort the list
	Sort();
}

void Sfo::SetValue( const string &key, const Buffer &val )
{
	vector<SfoEntry>::iterator entry = entries.begin();
	while( entry < entries.end() )
	{
		if( (*entry).name == key )
		{
			(*entry).dataType = 0;
			(*entry).buf = val;
			if( (*entry).buf.IsEmpty() )
			{
				printf("Sfo::SetValue() ENOMEM\n");
			}
			return;
		}
		++entry;
	}
	//key doesnt already exist, create a new one and insert it
	SfoEntry n;
	n.buf = val;
	if( n.buf.IsEmpty() )
	{
		printf("Sfo::SetValue() ENOMEM\n");
		return;
	}
	n.name = key;
	n.dataType = 0;

	entries.push_back( n );

	//sort the list
	Sort();
}

void Sfo::RemoveKey( const string &key )
{
	int i = GetIndex( key );
	if( i < 0 )
		return;
	entries.erase( entries.begin() + i );
}

vector<string>Sfo::Keys()
{
	vector<string> ret;
	vector<SfoEntry>::iterator entry = entries.begin();
	while( entry < entries.end() )
	{
		ret.push_back( (*entry).name );
		++entry;
	}
	return ret;
}

Sfo::IdRegion Sfo::RegionFromID( const string& id )
{
	if( id.size() < 4 )
		return Unknown;

	if( StartsWith( id, "BLUS" ) )
		return USA;

	if( StartsWith( id, "BLES" )
		|| StartsWith( id, "BLAS" ) )
		return EUR;

	if( StartsWith( id, "BLJS" )
		|| StartsWith( id, "BLJM" ) )
		return JAP;

	return Unknown;
}
