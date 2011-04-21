#include <sys/file.h>


#include <algorithm>
#include <iostream>
#include <malloc.h>
#include <stdio.h>

#include "fileops.h"
#include "gamelist.h"
#include "sfo.h"
#include "stringstuff.h"
#include "utils.h"

static bool GameSortPredicate( const Game &g1, const Game &g2 )
{
	return g1.Name() < g2.Name();
}

Game::Game()
{
	ok = false;
}

Game::Game( const string &path )
{
	ok = false;
//	iconData = NULL;
	Load( path );
}

Game::~Game()
{
	//if( iconData )
	//	delete iconData;
}

void Game::Load( const string &path )
{
	//cout << "Game::Load( \"" << path << "\" )" << endl;
	this->path = path;
	if( !HasEnding( this->path, "/" ) )
		this->path += "/";

	u32 sfoSize;
	u32 pos;
	u32 str;
//	u32 imageSize = 0;

	u32 indx = 0;

	//load param.sfo and read name & ID
	string sfoPath = this->path + "PS3_GAME/PARAM.SFO";

	u8 *mem = FileOps::ReadFile( sfoPath, &sfoSize );
	if( !mem )
		return;

	str = ( mem[ 8 ] + ( mem[ 9 ] << 8 ) );
	pos = ( mem[ 0xc ] + ( mem[ 0xd ] << 8 ) );

	while( str < sfoSize )
	{
		if( mem[ str ] == 0 )
			break;

		if( !strcmp( (char *)&mem[ str ], "TITLE" ) )
		{
			string tmp = (char *)&mem[ pos ];
			name.fromUTF8( tmp.c_str() );
		}
		else if( !strcmp((char *)&mem[ str ], "TITLE_ID"))
		{
			id = (char *)&mem[ pos ];
			id.insert( 4, "-" );
		}
		while( mem[ str ] )
			str++;

		str++;
		pos += ( mem[ 0x1c + indx ] + ( mem[ 0x1d + indx ] << 8 ) );
		indx += 16;
	}

	free( mem );
	mem = NULL;
	if( name.empty() )
		MakeNameFromPath();

	//u64 totalSize = FileOps::FileSize( this->path );
	//float gbSize = (float)totalSize/(float)GiB;

	cout << "id: " << id << "\ttitle: " << name.toUTF8() << endl;//"\tpath: " << this->path << endl;//"\t" << gbSize << endl;
	//Sfo sfo( this->Path() + "PS3_GAME/PARAM.SFO" );
	//sfo.Print();

	ok = ( !name.empty() && !id.empty() );
}

void Game::MakeNameFromPath()
{
	name.clear();
	if( path.empty() )
		return;

	name = path;

	//remove trailing slash
	name.resize( name.size() - 1 );

	//remove leading folders
	u32 slash = name.rfind( '/' );
	if( slash == string::npos )
	{
		name.clear();
		return;
	}
	name.erase( 0, slash + 1 );
}

namespace GameList
{
static vector<Game>games;

bool listDirty = false;

void Get( const vector<string> &devices )
{
	cout << "Get()" << endl;
	vector<string>::const_iterator device = devices.begin();
	while( device < devices.end() )
	{
		//cout << "derp: \"" << (*device) + "/GAMES/\"" << endl;
		if( FileOps::Exists( (*device) + "/GAMES/" ) )
		{
			AddFromPath( (*device) + "/GAMES/" );
		}
		++device;
	}
}

void RemoveGamesOnDevices( const vector<string> &devices )
{
	//cout << "RemoveGamesOnDevices() " << games.size() << endl;
	vector<Game>::iterator game = games.begin();
	vector<Game>keepers;

	//cycle through each game in the list
	while( game < games.end() )
	{
		//see if this game is on any of the listed devices
		bool found = false;
		string path = (*game).Path();
		vector<string>::const_iterator device = devices.begin();
		while( device < devices.end() )
		{
			if( !path.compare( 0, (*device).size(), (*device) ) )
			{
				//cout << "deleting :" << path << endl;
				listDirty = true;
				found = true;
				break;
			}
			++device;
		}
		if( !found )
		{
			keepers.push_back( (*game ));
		}
		++game;
	}
	games = keepers;
}

void Clear()
{
	if( !listDirty && games.size() )
		listDirty = true;
	games.clear();
}

bool AddGame( const Game &game, bool sort )
{
	if( FindByPath( game.Path() ) != -1 )
		return false;

	games.push_back( game );
	listDirty = true;
	if( sort )
		Sort();
	return true;
}

bool AddGame( const string &path, bool sort )
{
	Game game( path );
	if( game.IsOk() )
	{
		return AddGame( game, sort );
	}
	return false;
}

bool RemoveGame( const string& path, bool sort )
{
	int i = FindByPath( path );
	if( i == -1 )
		return false;
	games.erase( games.begin() + i );
	if( sort )
		Sort();
	listDirty = true;
	return true;
}

void AddFromPath( string path )
{
	cout << "AddFromPath( " << path << " )" << endl;
	if( path.at( path.size() -1 ) != '/' )
		path += "/";

	//get entries
	vector<string> dirs = FileOps::ReadDir( path, DIR_NO_DOT_AND_DOTDOT | DIR_DIRS );

	//try to load each directory as a game
	vector<string>::iterator it = dirs.begin();
	while( it < dirs.end() )
	{
		string str = path + (*it) + "/";
		if( FileOps::Exists( str + "PS3_GAME/" ) )
		{
			AddGame( str );
		}
		++it;
	}
	if( listDirty )
		Sort();
}

void Sort()
{
	cout << "Sort()" << endl;
	std::sort( games.begin(), games.end(), GameSortPredicate );
	cout << "Sort() done" << endl;
}

int Count()
{
	return games.size();
}

const Game &At( int i )
{
	return games.at( i );
}

const vector<Game> &List()
{
	return games;
}

int FindByPath( const string &path )
{
	int i = 0;
	vector<Game>::iterator it = games.begin();
	while( it < games.end() )
	{
		if( (*it).Path() == path )
			return i;
		i++;
		++it;
	}
	return -1;
}

bool ListDirty()
{
	bool ret = listDirty;
	listDirty = false;
	return ret;
}

wString Name( u32 idx )
{
	if( idx >= games.size() )
		return wString();
	return games.at( idx ).WName();
}
}

