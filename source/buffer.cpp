#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "buffer.h"
#include "utils.h"

Buffer::Buffer( const u8* stuff, u32 size )
{
	ptr = NULL;
	len = 0;
	SetData( stuff, size );
}

Buffer::Buffer( u32 size )
{
	ptr = (u8*)malloc( size );
	if( !ptr )
	{
		printf("Buffer::Buffer() failed to allocate memory (0x%08x bytes)\n", size );
		len = 0;
		return;
	}
	len = size;
}

Buffer::Buffer( const Buffer &other )
{
	ptr = NULL;
	len = 0;
	SetData( other.ptr, other.len );
}

Buffer::~Buffer()
{
	if( ptr )
		free( ptr );
}

void Buffer::Free()
{
	if( ptr )
	{
		free( ptr );
		ptr = NULL;
	}
	len = 0;
}

void Buffer::Dump( u32 start, u32 size )
{
	if( !size )
		size = len - start;

	if( ( start + size ) > len || !ptr )
		return;

	hexdump( ptr + start, size );
}

Buffer &Buffer::SetData( const u8* stuff, u32 size )
{
	if( ptr )
	{
		free( ptr );
		ptr = NULL;
		len = 0;
	}
	if( !stuff && !size )
		return *this;

	if( !size )
	{
		len = strlen( (const char*)stuff );
	}
	else
	{
		len = size;
	}
	if( !len )//wtf.  this probably will never happen
		return *this;

	ptr = (u8*)malloc( len );
	if( !ptr )
	{
		printf("Buffer::Buffer() failed to allocate memory (0x%08x bytes)\n", len );
		len = 0;
		return *this;
	}
	memset( ptr, 0, len );
	if( stuff )
	{
		memcpy( ptr, stuff, len );
	}
	return *this;
}

Buffer &Buffer::Append( const u8* stuff, u32 size )
{
	if( !stuff )
		return *this;
	if( !size )
	{
		size = strlen( (const char*)stuff );
	}
	ptr = (u8*)realloc( ptr, len + size );
	if( !ptr )
	{
		printf( "Buffer::Append(): failed to allocate memory\n" );
		len = 0;
		return *this;
	}
	memset( ptr + len, 0, size );
	memcpy( ptr + len, stuff, size );
	len += size;
	return *this;
}

Buffer &Buffer::Append( const Buffer &other )
{
	return Append( other.ptr, other.len );
}

Buffer &Buffer::Append( char c )
{
	ptr = (u8*)realloc( ptr, len + 1 );
	if( !ptr )
	{
		printf( "Buffer::Append(): failed to allocate memory\n" );
		len = 0;
		return *this;
	}
	ptr[ len ] = c;
	len++;
	return *this;
}

Buffer &Buffer::Resize( u32 size )
{
	if( !size )
	{
		printf("!size\n");
		Free();
		return *this;
	}
	ptr = (u8*)realloc( ptr, size );
	if( !ptr )
	{
		printf( "Buffer::Resize(): failed to allocate memory\n" );
		len = 0;
		return *this;
	}
	len = size;
	return *this;
}

Buffer &Buffer::Insert( u32 pos, char c )
{
	ptr = (u8*)realloc( ptr, len + 1 );
	if( !ptr )
	{
		printf( "Buffer::Insert(): failed to allocate memory\n" );
		len = 0;
		return *this;
	}

	//copy the chunk after the insertion point
	u32 p1 = len - 1;
	u32 p2 = p1 + 1;
	while( p1 >= pos + 1 )
	{
		ptr[ p2-- ] = ptr[ p1-- ];
	}
	//copy the new data
	ptr[ pos ] = c;
	len++;
	return *this;
}

Buffer &Buffer::Insert( u32 pos, const u8* stuff, u32 size )
{
	if( !stuff )
	{
		printf( "Buffer::Insert(): stuff = NULL\n" );
		return *this;
	}
	if( pos > len )
	{
		printf( "Buffer::Insert(): pos > len [ %08x > %08x ]\n", pos, len );
		return *this;
	}
	ptr = (u8*)realloc( ptr, len + size );
	if( !ptr )
	{
		printf( "Buffer::Insert(): failed to allocate memory\n" );
		len = 0;
		return *this;
	}

	//copy the chunk after the insertion point
	u32 p1 = len - 1;
	u32 p2 = ( len + size ) -1;
	while( p1 >= pos + 1 )
	{
		ptr[ p2-- ] = ptr[ p1-- ];
	}
	//copy the new data
	memcpy( ptr + pos, stuff, size );
	len += size;
	return *this;
}

Buffer &Buffer::Insert( u32 pos, const Buffer &other )
{
	return Insert( pos, other.ptr, other.len );
}

Buffer &Buffer::operator=( const Buffer &other )
{
	Free();
	if( other.len )
	{
		ptr = (u8*)malloc( other.len );
		if( !ptr )
		{
			printf( "Buffer::operator=: failed to allocate memory\n" );
			len = 0;
			return *this;
		}
		len = other.len;
		memcpy( ptr, other.ptr, len );
	}
	return *this;
}

Buffer &Buffer::operator=( const char* stuff )
{
	Free();
	if( !stuff )
		return *this;

	len = strlen( stuff );
	ptr = (u8*)malloc( len );
	if( !ptr )
	{
		printf( "Buffer::operator=: failed to allocate memory\n" );
		len = 0;
		return *this;
	}
	memcpy( ptr, stuff, len );
	return *this;
}
