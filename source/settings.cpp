
#include <iostream>
#include <malloc.h>
#include <sstream>
#include <stdio.h>

#include "fileops.h"
#include "settings.h"
#include "stringstuff.h"
#include "utils.h"

#define SETTINGS_FILENAME "/GlobalSettings.txt"


using namespace std;

namespace Settings
{
int viewportX;
int viewportY;
std::string installDir;

bool ParseLine( const std::string &line );

void SetDefaults()
{
	viewportX = -42.0f;
	viewportY = -42.0f;

	installDir = "/dev_hdd0/GAMES";
}

void Load()
{
	printf("Settings::Load()\n");
	u8* buf;
	u32 size;

	//make sure all settings are initialized
	SetDefaults();

	//read settings file
	string path = FileOps::HomeDir();
	if( path.empty() )
		return;

	path += SETTINGS_FILENAME;
	buf = FileOps::ReadFile( path, &size );
	if( !buf )
		return;

	string str( (const char*)buf );
	str.resize( size );
	free( buf );

	//split into lines
	vector<string>lines = Split( str, '\n' );
	vector<string>::iterator line = lines.begin();

	//parse each line
	while( line < lines.end() )
	{
		ParseLine( *line );
		cout << *line << endl;
		++line;
	}
}

bool Save()
{
	string str;
	ostringstream stream;
	string path = FileOps::HomeDir();
	if( path.empty() )
		return false;
	path += SETTINGS_FILENAME;

	stream
	<< "viewportX: " << Settings::viewportX << "\n"
	<< "viewportY: " << Settings::viewportY << "\n"
	<< "installDir: " << installDir << "\n"
	;

	str = stream.str();
	//cout << "save file: \n" << str << endl;

	u32 size = str.size();
	return FileOps::WriteFile( path, (const u8*)str.c_str(), size );
}

bool ParseLine( const std::string &line )
{
	int num = 0;
	float fl = 0;
	u32 size = 0;

	//sikp empty lines or ones starting with #
	if( !line.size() || line.at( 0 ) == '#' )
		return true;
	if( !line.compare( 0, 11, "viewportX: " ) )
	{
		sscanf( line.c_str(), "viewportX: %i%n", &num, &size);
		if( size == line.size() )
		{
			Settings::viewportX = num;
			return true;
		}
	}
	if( !line.compare( 0, 11, "viewportY: " ) )
	{
		sscanf( line.c_str(), "viewportY: %i%n", &num, &size);
		if( size == line.size() )
		{
			Settings::viewportY = num;
			return true;
		}
	}
	if( !line.compare( 0, 12, "installDir: " ) )
	{
		installDir = line.substr( 12 );
		return true;
	}

	cout << "failed to parse setting line: \"" << line << "\"" << endl;
	return false;
}
}
