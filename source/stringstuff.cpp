
#include <iostream>
#include <sstream>


#include <ppu-types.h>
#include "stringstuff.h"
#include "wstring.h"

bool HasEnding( std::string const &fullString, std::string const &ending )
{
	unsigned int lastMatchPos = fullString.rfind(ending); // Find the last occurrence of ending
	bool isEnding = lastMatchPos != std::string::npos; // Make sure it's found at least once

	// If the string was found, make sure that any characters that follow it are the ones we're trying to ignore
	for( u32 i = lastMatchPos + ending.length(); (i < fullString.length()) && isEnding; i++)
	{
		if( (fullString[i] != '\n') &&
			(fullString[i] != '\r') )
		{
			isEnding = false;
		}
	}

	return isEnding;
}

using namespace std;
void PrintStringList( std::vector<std::string> &list )
{
	cout << "list has " << list.size() << " entries" << endl;
	int i = 0;
	std::vector<string>::iterator it = list.begin();
	while( it != list.end() )
	{
		cout << i << "\t" << *it << endl;
		++it;
		++i;
	}

}

bool StartsWith( string const &fullString, string const &beginning )
{
	return ( fullString.size() >= beginning.size() && !fullString.compare( 0, beginning.size(), beginning ) );
}

std::vector<std::string> &Split( const std::string &s, char delim, std::vector<std::string> &elems, bool skipEmpty )
{
	std::stringstream ss( s );
	std::string item;
	while( std::getline( ss, item, delim ) )
	{
		if( skipEmpty && item.empty() )
			continue;

		elems.push_back( item );
	}
	return elems;
}

std::vector<std::string> Split( const std::string &s, char delim, bool skipEmpty )
{
	std::vector<std::string> elems;
	return Split( s, delim, elems, skipEmpty );
}

string ToString(const int& number )
{
	ostringstream oss;
	oss << number;
	return oss.str();
}

string ToString(const float& number )
{
	ostringstream oss;
	oss << number;
	return oss.str();
}

std::string ToFatName( const std::string &str )
{
	// ? < > \ : * | ” ^
	wString wstr;
	wstr.fromUTF8( str.c_str() );
	int len = wstr.size();
	for( int i = len - 1; i >= 0; i-- )
	{
		if( //wstr.at( i ) >= 0x20
			//&& wstr.at( i ) <= 0x7e
			//&&
			wstr.at( i ) == '?'
			|| wstr.at( i ) == '<'
			|| wstr.at( i ) == '>'
			|| wstr.at( i ) == '\\'
			|| wstr.at( i ) == ':'
			|| wstr.at( i ) == '*'
			|| wstr.at( i ) == '|'
			|| wstr.at( i ) == '\"'
			|| wstr.at( i ) == '^' )
			wstr.erase( i, 1 );
	}

	while( wstr.size() && wstr.at( wstr.size() - 1 ) == ' ' )
		wstr.resize( wstr.size() - 1 );

	while( wstr.size() && wstr.at( 0 ) == ' ' )
		wstr.erase( 0, 1 );

	if( wstr.size() > 255 )
		wstr.resize( 255 );

	string ret = wstr.toUTF8();
	return ret;
}

std::string Utf8ToAnsi( const std::string &utf8 )
{
	u8 *ch = (u8 *)utf8.c_str();
	u8 c;
	int len = utf8.size();
	std::string ansi;

	while( *ch != 0 && len > 0 )
	{
		// 3, 4 bytes utf-8 code
		if( ( ( *ch & 0xF1 ) == 0xF0 || ( *ch & 0xF0 ) == 0xe0 ) && ( *( ch + 1 ) & 0xc0 ) == 0x80 )
		{
			ansi += ' ';
			len--;
			ch += 2 + 1 *(( *ch & 0xF1 ) == 0xF0 );

		}// 2 bytes utf-8 code
		else if( ( *ch & 0xE0 ) == 0xc0 && ( *( ch + 1 ) & 0xc0 ) == 0x80 )
		{
			c= (((*ch & 3)<<6) | (*(ch+1) & 63));

			if(c>=0xC0 && c<=0xC5) c='A';
			else if(c==0xc7) c='C';
			else if(c>=0xc8 && c<=0xcb) c='E';
			else if(c>=0xcc && c<=0xcf) c='I';
			else if(c==0xd1) c='N';
			else if(c>=0xd2 && c<=0xd6) c='O';
			else if(c>=0xd9 && c<=0xdc) c='U';
			else if(c==0xdd) c='Y';
			else if(c>=0xe0 && c<=0xe5) c='a';
			else if(c==0xe7) c='c';
			else if(c>=0xe8 && c<=0xeb) c='e';
			else if(c>=0xec && c<=0xef) c='i';
			else if(c==0xf1) c='n';
			else if(c>=0xf2 && c<=0xf6) c='o';
			else if(c>=0xf9 && c<=0xfc) c='u';
			else if(c==0xfd || c==0xff) c='y';
			else if(c>127) c=*(++ch+1); //' ';
			ansi += c;
			len--;
			ch++;
		}
		else
		{
			if( *ch < 32 )
				*ch = 32;
			ansi += *ch;
			len--;
		}
		ch++;
	}
	return ansi;
}

bool ListContains( const std::vector<std::string>& list, const std::string &text )
{
	std::vector<std::string>::const_iterator it = list.begin();
	while( it < list.end() )
	{
		if( *it == text )
			return true;
		++it;
	}
	return false;
}
