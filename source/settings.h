#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#include "filebrowser.h"

//settings limits
#define VIEWPORT_MIN            -100
#define VIEWPORT_MAX            0


//functions dealing with saving and loading settings.
//! a file is stored in text format in the application's home directory on teh PS3 HDD.
//! if this directory doesnt not exist, saving and loading will fail
namespace Settings
{
	//setting variables
	//display viewport
	extern int viewportX;
	extern int viewportY;

	//where to install games
	extern std::string installDir;

	//init default settings
	void SetDefaults();

	//try to load the settings file from this program's home directory in the PS3 HDD
	void Load();

	//try to save these settings in a file in teh home directory
	bool Save();

};



#endif // SETTINGS_H
