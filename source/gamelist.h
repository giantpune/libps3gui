#ifndef GAMELIST_H
#define GAMELIST_H

#include <string>
#include <vector>

#include "gui.h"
#include "wstring.h"

using namespace std;

//class to hold info about each game
class Game
{
public:
	//constructor
	Game();

	//constructor
	//! if path is not empty, it will attempt to call Load() with the given path
	Game( const string &path );
	~Game();

	//get the game's name in a wString
	//! returns a wString of the game's name in wchar_t format
	const wString &WName() const { return name; }

	//get the game's name
	//! returns a utf8 encoded string of the game's name
	const string Name() const { return name.toUTF8(); }

	//get the game ID
	//! returns a string containing the game's ID with a '-' inserted in position 4
	const string &Id() const { return id; }

	//get the path of this game
	//! returns a '/'-terminated string containing the path of the folder containing this game's contents
	const string &Path() const { return path; }

	//check if this game info was loaded correctly
	//! useful when using the second constructor
	bool IsOk() { return ok; }

private:
	void Load( const string &path );
	void MakeNameFromPath();
	string path;
	wString name;
	string id;
	bool ok;
};

//class for handleing lists of games
//! the basic idea here is to use Get() to pass a list of device paths to check for games
//! use RemoveGamesOnDevices() to pass a list of devices to clear games from (if a HDD has been disconnected, for example)
//! check ListDirty() to see if the list has changed since last time ListDirty() was called

//! this is NOT threadsafe.  there is no mutex or anything going on here
namespace GameList
{


	//pass a list of devices to check for games
	void Get( const vector<string> &devices );

	//clear the list
	void Clear();

	//add games to the list
	bool AddGame( const Game& game, bool sort = false );
	bool AddGame( const string &path, bool sort = false );
	bool RemoveGame( const string& path, bool sort = false );

	//remove all games on some devices
	void RemoveGamesOnDevices( const vector<string> &devices );

	//search a path and add games
	void AddFromPath( string path );

	//how many games are in the list?
	int Count();

	//get a reference to the gamelist
	const vector<Game> &List();

	//sort the list alphabetically by name
	void Sort();

	//this assumes that i is less than Count()
	const Game &At( int i );

	//check if the list has been changed since the last time this function was called
	bool ListDirty();

	//returns the index of a game or -1 if not found
	int FindByPath( const string &path );

	//get the name of a game
	wString Name( u32 idx );
}

#endif // GAMELIST_H
