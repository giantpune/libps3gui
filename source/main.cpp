/*
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include <sysmodule/sysmodule.h>
#include <tiny3d.h>

#include "buffer.h"
#include "devicethread.h"
#include "filebrowser.h"
#include "fileops.h"
#include "gamelist.h"
#include "gui.h"
#include "pad.h"
#include "menu.h"
#include "resources/resources.h"
#include "rsxmem.h"
#include "settings.h"
#include "sfo.h"
#include "stringstuff.h"
#include "utils.h"


using namespace std;
s32 main(s32 argc, const char* argv[])
{
	cout.precision( 2 );

	//init settings to default values and load a settings file from the HDD if it exists
	Settings::Load();

	//init the sound stuff
	assert( GuiSound::Init() );

	tiny3d_Init( 1024 * 1024 );
	tiny3d_Project2D();

	AdjustViewport( Settings::viewportX, Settings::viewportY );

	//init muh pad stuff
	PadInit();

	//load fs, png & jpg sprx
	assert( !sysModuleLoad( SYSMODULE_PNGDEC ) );
	assert( !sysModuleLoad( SYSMODULE_JPGDEC ) );

	//this allocates the memory heap used to store fonts and textures
	//! lol @ 156 MiB.  though i dont suppose theres much else that memory needs to be reserved for
	RsxMem::Init( 156 * 1024 * 1024 );

	atexit( exiting ); // Tiny3D register the event 3 and do exit() call when you exit  to the menu

	//set tiny3d to work in 2d mode
	tiny3d_Project2D();

	//setup filebrowser
	FileBrowser::Init();

	//build list of available resources
	Resources::Init();

	//start checking for devices
	DeviceThread::Init();
	DeviceThread::Resume();

	//create thread to handle gui stuff
	InitGuiThread();

	//start looping and crap
	MainMenu( MENU_COVERFLOW );

	exit( 0 );

	return 0;
}

