#ifndef FILEOPS_H
#define FILEOPS_H

#include <ppu-types.h>
#include <fcntl.h>
#include <string>
#include <vector>

#include "buffer.h"
#include "utils.h"

namespace FileOps
{

//check if a path exists
bool Exists( const char* path );
bool Exists( const std::string &path );

//check if a path is a directory
bool IsDir( const char* path );
bool IsDir( const std::string &path );

//allocate memory, read a file into it, and get the out size
//! returns NULL on error
u8* ReadFile( const char* path, u32 *size = NULL );
u8* ReadFile( const std::string &path, u32 *size = NULL );
Buffer ReadFileToBuffer( const std::string &path );

//write data to some place
//! truncates the original file or creates it new
//! returns true if all the data was able to be written, otherwise false
bool WriteFile( const std::string &path, const u8* stuff, u32 size );
bool WriteFile( const std::string &path, const Buffer &buf );


#define DIR_SKIP_DOT			1			//dont include "."
#define DIR_SKIP_DOTDOT			2			//dont include ".."
#define DIR_FILES				4			//include files
#define DIR_DIRS				0x10		//include directories
#define DIR_SKIP_HIDDEN			0x20		//skip entries starting with "." besides dot and dotdot
#define DIR_NO_DOT_AND_DOTDOT	( DIR_SKIP_DOT | DIR_SKIP_DOTDOT )


//get a list of the entries in a directory
//! flags is a or'd together combination of the above options
std::vector<std::string> ReadDir( const std::string &path, u32 flags = DIR_NO_DOT_AND_DOTDOT | DIR_FILES | DIR_DIRS );


#define DEV_INTERNAL_HDD		1
#define DEV_USB_HDD				2			//just lump together all the /dev_usb* entries
#define DEV_INTERNAL_FLASH		4			//im just lumping all of the /dev_flash* together
#define DEV_SD					0x10
#define DEV_BDVD				0x20
#define DEV_MEMORY_STICK		0x40
#define DEV_COMPACT_FLASH		0x100
#define DEV_REMOVABLE			( DEV_USB_HDD | DEV_SD | DEV_BDVD | DEV_MEMORY_STICK | DEV_COMPACT_FLASH )
#define DEV_USER_STORAGE		( DEV_REMOVABLE | DEV_INTERNAL_HDD )
#define DEV_ALL					( DEV_INTERNAL_HDD | DEV_USB_HDD | DEV_INTERNAL_FLASH | DEV_SD | DEV_BDVD | DEV_MEMORY_STICK | DEV_COMPACT_FLASH )

//get a device list
//! returns a list of /dev_*   ( preceding '/' is included )
//! flags is an or'd combination of the above choices
std::vector<std::string> DevList( u32 flags = DEV_ALL );

//get the program's home directory on the internal HDD ( APPID is generated at compile time )
//! returns "/dev_hdd0/game/"<APPID>"/USRDIR"
//! returns an empty string if the folder doesnt already exist
std::string HomeDir();

//get free space for a given path
//! unit is the unit to return ( 1 = byte, 1024 = KiB... )
//! returns 0 on error
//! for some reason this function doesnt work for the SD card for me :(
float FreeSpace( const std::string &path, u64 unit = GiB );


#define MAX_RECURSE_DEPTH	15
//get the size of a file, or recurse a folder ( up to MAX_RECURSE_DEPTH ) and combine all the sizes
//! depth is used internally to count recursion - dont use it :D
u64 FileSize( std::string path, u8 depth = 0 );

bool RecurseDeletePath( std::string path, u8 depth = 0 );

}

#endif // FILEOPS_H
