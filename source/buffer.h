#ifndef BUFFER_H
#define BUFFER_H

#include <ppu-types.h>


//quick'n'dirty (tm) buffer class
//! uses malloc, so no data alignment is performed
//! no memory sharing takes place among buffers
class Buffer
{
public:

	//create a Buffer object with the given data up to the first NULL byte or until len
	//! this makes a copy of the data
	explicit Buffer( const u8* stuff = NULL, u32 size = 0 );
	Buffer( const Buffer &other );

	//! creates a buffer with the given size
	Buffer( u32 size );
	~Buffer();

	//de-allocate memory
	void Free();

	//returns true if len or ptr is 0
	bool IsEmpty() { return !( len && ptr ); }

	//for debugging purposes
	//! hexdump data to printf, from start to end.
	//! if size == 0, it just dumps till the end
	void Dump( u32 start = 0, u32 size = 0 );

	//number of bytes
	u32 Size() { return len; }

	//access functions to the data
	u8* Data() { return ptr; }
	const u8* ConstData() const { return ptr; }

	//copies the data to this buffer and returns a reference to itself
	Buffer &SetData( const u8* stuff, u32 size = 0 );

	//add more data to the end
	Buffer &Append( char c );
	Buffer &Append( const u8* stuff, u32 size = 0 );
	Buffer &Append( const char* stuff, u32 size = 0 ) { return Append( (u8*)stuff, size ); }
	Buffer &Append( const Buffer &other );

	//add data to the middle
	Buffer &Insert( u32 pos, char c );
	Buffer &Insert( u32 pos, const u8* stuff, u32 size = 0 );
	Buffer &Insert( u32 pos, const Buffer &other );

	//add data to the start
	Buffer &Prepend( char c ) { return Insert( 0, c ); }
	Buffer &Prepend( const u8* stuff, u32 size ) { return Insert( 0, stuff, size ); }
	Buffer &Prepend( const Buffer &other ) { return Insert( 0, other ); }


	//change the size of the allocated memory for this buffer
	Buffer &Resize( u32 size );

	//operators to append data to this buffer and return a reference it itself
	Buffer &operator=(const Buffer &);
	Buffer &operator=( const char* stuff );

	Buffer &operator+=(char c);
	Buffer &operator+=( const u8* stuff );
	Buffer &operator+=( const char* stuff );
	Buffer &operator+=( const Buffer & );

	Buffer &operator<<(char c);
	Buffer &operator<<( const u8* stuff );
	Buffer &operator<<( const char* stuff );
	Buffer &operator<<( const Buffer & );
private:
	u8* ptr;
	u32 len;
};
inline Buffer &Buffer::operator+=(char c)
{ return Append( c ); }
inline Buffer &Buffer::operator+=(const u8* stuff)
{ return Append( stuff ); }
inline Buffer &Buffer::operator+=(const char* stuff)
{ return Append( stuff ); }
inline Buffer &Buffer::operator+=( const Buffer &other )
{ return Append( other ); }

inline Buffer &Buffer::operator<<(char c)
{ return Append( c ); }
inline Buffer &Buffer::operator<<(const char* stuff)
{ return Append( stuff ); }
inline Buffer &Buffer::operator<<(const u8* stuff)
{ return Append( stuff ); }
inline Buffer &Buffer::operator<<( const Buffer &other )
{ return Append( other ); }

#endif // BUFFER_H
