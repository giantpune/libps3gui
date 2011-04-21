#ifndef SFO_H
#define SFO_H

#include <ppu-types.h>
#include <string>
#include <vector>

#include "buffer.h"

using namespace std;
struct SfoEntry
{
	string name;
	u8 dataType;
	Buffer buf;	//used when datatype = 0 or 2
	u32 num;	//used when datatype = 4
};

class Sfo
{
public:
	enum IdRegion//quick'n'dirty(tm) way to get the region from a given ID
	{
		Unknown,
		JAP,
		USA,
		EUR
	};
	static IdRegion RegionFromID( const string& id );

	Sfo( const string& path = "" );
	~Sfo();

	bool Load( const string& path );
	bool Load( const u8* mem, u32 size );

	u32 Count() { return entries.size(); }

	//debug stuff
	void Print();

	//get the Sfo ready for writing to a file
	//! this function goesnt through and assembles a buffer with all the listed entries
	//! it is somewhat expensive and not meant to be called repeatedly back to back.
	Buffer Data();

	//get the index for a given key
	//! stays valid until an entry is added or removed before ( alphabetically by key ) this one
	//! returns -1 on error
	int GetIndex( const string &key );

	//get the data type for a given key
	//! 0 = data
	//! 2 = string
	//! 4 = u32
	//! 0xff = error
	u8 GetType( int idx );
	u8 GetType( const string &key );

	//get the value for an entry of type 0 given the key name or index
	//! returns an empty Buffer on error
	Buffer GetValueBuf( int idx );
	Buffer GetValueBuf( const string &key );

	//get the value for an entry of type 2 given the key name or index
	//! returns an empty string on error
	string GetValueStr( int idx );
	string GetValueStr( const string &key );

	//get the value for an entry of type 4 given the key name or index
	//! returns 0xffffffff on error
	u32 GetValueU32( int idx );
	u32 GetValueU32( const string &key );

	//set a value for a given key
	//! the val type is determined by what type of variable is given for "val"
	void SetValue( const string &key, u32 val );
	void SetValue( const string &key, const string &val );
	void SetValue( const string &key, const Buffer &val );

	//delete a given key from the list
	void RemoveKey( const string &key );

	//get a list of all the keys
	vector<string>Keys();




private:
	vector<SfoEntry> entries;

	//sort the entries by name
	void Sort();
};



#endif // SFO_H
