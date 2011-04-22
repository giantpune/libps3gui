#ifndef RESOURCES_H
#define RESOURCES_H

#include <ppu-types.h>

#include "../buffer.h"
#include "resourcelist.h"

// this class is designed to make it possible to ask for a filename such as "images/button.png"
// and if that file is found inside this application's home directory, it will be loaded from there
// otherwise it will try to use a file that was linked with the executable during compile time

//! basic usege...  first call the static member Init() which initializes the list of files that are included in
//! the executable.  then create an instance of Resource and pass it a path.  you can access the data it has to
//! offer with Data() and Size().  both of those return 0  (NULL) if there was an error in loading the resource
class Resource
{
public:
	Resource( const std::string &path );

	const u8* Data();
	const u8* Data() const;
	u32 Size();
	u32 Size() const;


	// creates a list of the different png, wav, pcm, ect files that are included in this executable
	static void Init();

private:
	// this is used if this resource is using embedded file
	map< string, pair< u8*, u32 > >::const_iterator it;

	// this is used if this resource is using a file read from a file
	Buffer buf;

};


#endif // RESOURCES_H
