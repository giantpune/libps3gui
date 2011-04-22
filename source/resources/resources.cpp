
#include <stdio.h>
#include <iostream>

#include "../fileops.h"
#include "resources.h"

Resource::Resource( const std::string &path )
{
	it = ResourceList::List().end();
	if( path.empty() )
	{
		printf( "Resource::Resource() path is empty\n" );
		return;
	}

	//first try to read from external file
	string home = FileOps::HomeDir();
	if( !home.empty() )
	{
		buf = FileOps::ReadFileToBuffer( home + path );
	}

	// for whatever reason, the buffer is still empty, try to use embedded resource
	if( buf.IsEmpty() )
	{
		it = ResourceList::List().find( path );
		if( it == ResourceList::List().end() )
		{
			printf( "Resource::Resource() error loading \"%s\"\n", path.c_str() );
		}
	}
}

const u8* Resource::Data()
{
	if( !buf.IsEmpty() )
		return buf.Data();
	if( it != ResourceList::List().end() )
		return (*it).second.first;
	return NULL;
}

const u8* Resource::Data() const
{
	if( !buf.IsEmpty() )
		return buf.ConstData();
	if( it != ResourceList::List().end() )
		return (*it).second.first;
	return NULL;
}

u32 Resource::Size()
{
	if( it != ResourceList::List().end() )
		return (*it).second.second;
	return buf.Size();
}

u32 Resource::Size() const
{
	if( it != ResourceList::List().end() )
		return (*it).second.second;
	return buf.Size();
}

void Resource::Init()
{
	ResourceList::BuildList();
}

