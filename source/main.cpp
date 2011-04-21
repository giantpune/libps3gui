/*
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/
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
#include "rsxmem.h"
#include "settings.h"
#include "sfo.h"
#include "stringstuff.h"
#include "utils.h"
#include "warez.h"

#include <limits>
#include <iomanip>
#include "BoxMesh.hpp"


/*INC_FILE( PARAM_SFO );
INC_FILE( PARAM1_SFO );
INC_FILE( PARAM2_SFO );
INC_FILE( PARAM3_SFO );
INC_FILE( PARAM4_SFO );*/
using namespace std;
s32 main(s32 argc, const char* argv[])
{

	/*Sfo sfo;
	sfo.Load( PARAM_SFO, PARAM_SFO_size ); sfo.Print();
	sfo.Load( PARAM1_SFO, PARAM1_SFO_size ); sfo.Print();
	sfo.Load( PARAM2_SFO, PARAM2_SFO_size ); sfo.Print();
	sfo.Load( PARAM3_SFO, PARAM3_SFO_size ); sfo.Print();
	sfo.Load( PARAM4_SFO, PARAM4_SFO_size ); sfo.Print();
	exit( 0 );*/





	//////////////////////
	cout.precision( 2 );

	//init settings to default values and load a settings file from the HDD if it exists
	Settings::Load();


#ifdef WAREZZZ
	//load payload & whatnot
	if( !Warez::Init() )
		exit( 0 );
#endif

	//init the sound stuff
	if( !GuiSound::Init() )
		exit( 0 );

	tiny3d_Init( 1024 * 1024 );
	tiny3d_Project2D();

	AdjustViewport( Settings::viewportX, Settings::viewportY );

	//init muh pad stuff
	PadInit();

	//load fs, png & jpg sprx
	sysModuleLoad( SYSMODULE_PNGDEC );
	sysModuleLoad( SYSMODULE_JPGDEC );

	//this allocates the memory heap used to store fonts and textures
	//! lol @ 156 MiB.  though i dont suppose theres much else that memory needs to be reserved for
	RsxMem::Init( 156 * 1024 * 1024 );

	atexit( exiting ); // Tiny3D register the event 3 and do exit() call when you exit  to the menu

	//set tiny3d to work in 2d mode
	tiny3d_Project2D();

	//setup filebrowser
	FileBrowser::Init();

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

