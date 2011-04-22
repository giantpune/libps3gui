#include <tiny3d.h>

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sysmodule/sysmodule.h>


#include "boxbrowser.h"
#include "devicethread.h"
#include "filebrowser.h"
#include "gameinstalldialog.h"
#include "gamelist.h"
#include "gui.h"
#include "guiimageasync.h"
#include "menu.h"
#include "pad.h"
#include "settings.h"
#include "stringstuff.h"
#include "utils.h"

using namespace std;
extern GuiFont * font;

//global gui elements
GuiImageData *bgImgData = NULL;
GuiImage *bgImage = NULL;
GuiWindow *mainWindow = NULL;
GuiImageData * pointer[4] = { NULL, NULL, NULL, NULL };
GuiFont *font = NULL;
GuiSound *bgMusic = NULL;

//stuff dealing with the gui thread and syncronization
bool exitRequested = false;
static bool haltgui = true;
static bool guihalted = true;
static bool guiThreadRunning = false;

static sys_mutex_t hGuiMutex;
static sys_mutex_t h2GuiMutex;
static sys_mutex_t wGuiMutex;
static sys_cond_t guiWaitCondition;
static sys_cond_t haltWaitCondition;
sys_mutex_t exitRequestMutex;

static sys_ppu_thread_t guiThread;


u32 FrameTimer = 0;
void MenuRender();
static void UpdateGui( void* arg )
{
	guiThreadRunning = true;
	bool halt;

	while( 1 )
	{
		//check if GuiHalt() has been called
		sysMutexLock( hGuiMutex,  1000 * 1000 );
		halt = haltgui;
		sysMutexUnlock( hGuiMutex );
		if( halt )
		{
			bool exit = false;
			while( !exit )
			{
				sysMutexLock( exitRequestMutex,  1000 * 1000 );
				exit = exitRequested;
				sysMutexUnlock( exitRequestMutex );

				sysThreadYield();
				//signal GuiHalt() that this thread has halted
				if( sysMutexTryLock( h2GuiMutex ) )
					continue;

				sysCondSignal( haltWaitCondition );
				sysMutexUnlock( h2GuiMutex );

				//wait for ResumeGui() to be called
				if( sysMutexTryLock( wGuiMutex ) )
					continue;
				guihalted = true;
				sysCondWait( guiWaitCondition, 1000 * 1000 );
				guihalted = false;
				sysMutexUnlock( wGuiMutex );

				break;
			}
		}
		else
		{
			//check if exit() has been called
			sysMutexLock( exitRequestMutex, 1000 * 1000 );
			bool exit = exitRequested;
			sysMutexUnlock( exitRequestMutex );
			if( exit )
			{
				//fade out
				for( int i = 0; i < 255; i += 15 )
				{
					MenuPrepareToDraw();
					mainWindow->Draw();
					MenuDrawRectangle( i, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 1, 0, 1 );
					tiny3d_Flip();
				}
				break;
			}

			//read joypads
			PadRead();

			//draw the window
			MenuRender();

			//update gui elements with user input
			for( int i = 3; i >=0 ; i-- )
				mainWindow->Update( &userInput[ i ] );

		}
		sysThreadYield();
	}
	guiThreadRunning = false;

	//you must call this, kthx
	sysThreadExit( 0 );
}

void InitGuiThread()
{
	//thread parameters
	//u64 thread_arg = 0x1337;
	u64 priority = 2000;
	size_t stack_size = 0x2000;
	const char *thread_name = "Gui Thread";

	//initialize mutex & wait condition
	mutex_init( &exitRequestMutex );
	mutex_init( &hGuiMutex );
	mutex_init( &h2GuiMutex );
	mutex_init( &wGuiMutex );
	cond_init( &guiWaitCondition, &wGuiMutex );
	cond_init( &haltWaitCondition, &h2GuiMutex );

	int s = sysThreadCreate(	&guiThread, UpdateGui, NULL, priority, stack_size, THREAD_JOINABLE, (char *)thread_name );
	if( s )
	{
		printf("create gui thread returned %i.  exiting...\n", s );
		exit( 0 );
	}

}

void HaltGui()
{
	u16 dontfreeze = 0;
	while( 1 )
	{
		if( !++dontfreeze )
		{
			printf("HaltGui() threads deadlocked\n");
			DirtyExit();
		}

		sysThreadYield();
		//signal gui thread to wait
		if( sysMutexTryLock( hGuiMutex ) )
			continue;


		haltgui = true;
		sysMutexUnlock( hGuiMutex );

		//wait for gui thread to halt
		if( sysMutexTryLock( h2GuiMutex ) )
			continue;

		sysCondWait( haltWaitCondition, 0 );
		sysMutexUnlock( h2GuiMutex );

		break;
	}
}

void ResumeGui()
{
	u16 wtf = 1;
	while( 1 )
	{
		if( !wtf++ )
		{
			printf("ResumeGui() threads deadlocked\n");
			DirtyExit();
		}
		sysThreadYield();
		//signal gui thread to wait
		if( sysMutexTryLock( hGuiMutex ) )
			continue;
		haltgui = false;
		sysMutexUnlock( hGuiMutex );

		if( guiThreadRunning && !guihalted && !exitRequested )
			continue;

		if( sysMutexTryLock( wGuiMutex ) )
			continue;
		sysCondSignal( guiWaitCondition );
		sysMutexUnlock( wGuiMutex );

		break;
	}
}

void DirtyExit()
{
	printf("dirty exit\n");
	_exit( 1 );
}

void exiting()
{
	printf("exiting()...\n");
	sysMutexLock( exitRequestMutex, 1000 * 1000 );
	exitRequested = true;						//signal threads to end
	sysMutexUnlock( exitRequestMutex );

	printf("killing gui thread...\n");
	if( guiThreadRunning )
	{
		sys_ppu_thread_t threadid;
		if( !sysThreadGetId( &threadid ) && threadid != guiThread )//dont try to join the gui thread if that is the thread that called exit
		{
			ResumeGui();                                //make sure the gui thread is not stuck in wait condition before trying to join

			u64 retval;
			int t = sysThreadJoin( guiThread, &retval );
			if( t )
				printf("gui thread tried to join with return: %llX, sysThreadJoin returned %d\n", (unsigned long long int)retval, t );
		}
	}
	//destroy gui mutexes & wait conditions
	sysMutexDestroy( exitRequestMutex );
	sysMutexDestroy( hGuiMutex );
	sysMutexDestroy( h2GuiMutex );
	sysMutexDestroy( wGuiMutex );
	sysCondDestroy( guiWaitCondition );
	sysCondDestroy( haltWaitCondition );

	printf("killing device thread...\n");
	DeviceThread::Shutdown();

	printf("deleting gui elements...\n");
	//delete global gui elements
	if( bgImgData )
		delete bgImgData;
	if( bgImage )
		delete bgImage;
	if( font )
		delete font;
	if( bgMusic )
		delete bgMusic;
	if( mainWindow )
		delete mainWindow;

	for( int i = 0; i < 4; i++ )
	{
		if( pointer[ i ] )
			delete pointer[ i ];
	}

	printf("closing drivers...\n");
	sysModuleUnload( SYSMODULE_PNGDEC );		//unload PNG
	sysModuleUnload( SYSMODULE_JPGDEC );
	ioPadEnd();									//close pad driver
	GuiSound::UnInit();							//unload sound spu and whatnot
	printf("saving settings...\n");
	Settings::Save();							//save settings to HDD

}

void ErrorPrompt( const char* message )
{
    WindowPrompt( "ERROR!", message, "Ok", NULL );
}

int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label)
{
    int choice = -1;

    GuiWindow promptWindow(448,288);
    promptWindow.SetAlignment(ALIGN_CENTRE | ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);
	//GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline( Resource( "images/button.png" ) );
	GuiImageData btnOutlineOver( Resource( "images/button_over.png" ) );
    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, BTN_CROSS_ );

	//button sounds
	GuiSound btnSndOver( Resource( "sounds/button_over.wav" ), SOUND_WAV );
	GuiSound btnSndClick2( Resource( "sounds/button_click2.wav" ), SOUND_WAV );
	btnSndClick2.SetVolume( 50 );

	GuiImageData dialogBox( Resource( "images/dialogue_box.png" ) );
    GuiImage dialogBoxImg(&dialogBox);

    GuiText titleTxt( font, title, 26, 0x000000ff );
    titleTxt.SetAlignment(ALIGN_CENTRE| ALIGN_TOP);
    titleTxt.SetPosition(0,40);

	GuiText msgTxt( font, msg, 18, 0x000000ff);
    msgTxt.SetAlignment( ALIGN_CENTRE | ALIGN_MIDDLE );
    msgTxt.SetPosition(0,-20);
    msgTxt.SetWrap(true, 400);

    GuiImage btn1Img(&btnOutline);
    GuiImage btn1ImgOver(&btnOutlineOver);
	GuiText btn1Txt( font, btn1Label, 22, 0x000000ff);
	btn1Txt.SetPosition( 0, -5 );
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

    if(btn2Label)
    {
        btn1.SetAlignment(ALIGN_LEFT | ALIGN_BOTTOM);
        btn1.SetPosition(20, -25);
    }
    else
    {
        btn1.SetAlignment(ALIGN_CENTRE | ALIGN_BOTTOM);
        btn1.SetPosition(0, -25);
    }

    btn1.SetLabel(&btn1Txt);
    btn1.SetImage(&btn1Img);
    btn1.SetImageOver(&btn1ImgOver);
	//btn1.SetSoundOver(&btnSoundOver);
    btn1.SetTrigger(&trigA);
    btn1.SetState(STATE_SELECTED);
    btn1.SetEffectGrow();
	btn1.SetSoundOver( &btnSndOver );
	btn1.SetSoundClick( &btnSndClick2 );

    GuiImage btn2Img(&btnOutline);
    GuiImage btn2ImgOver(&btnOutlineOver);
	GuiText btn2Txt( font, btn2Label, 22, 0x000000ff);
	btn2Txt.SetPosition( 0, -5 );
    GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn2.SetAlignment(ALIGN_RIGHT| ALIGN_BOTTOM);
    btn2.SetPosition(-20, -25);
    btn2.SetLabel(&btn2Txt);
    btn2.SetImage(&btn2Img);
    btn2.SetImageOver(&btn2ImgOver);
	//btn2.SetSoundOver(&btnSoundOver);
    btn2.SetTrigger(&trigA);
    btn2.SetEffectGrow();
	btn2.SetSoundOver( &btnSndOver );
	btn2.SetSoundClick( &btnSndClick2 );

    promptWindow.Append(&dialogBoxImg);
    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);
    promptWindow.Append(&btn1);

    if(btn2Label)
        promptWindow.Append(&btn2);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while(choice == -1)
    {
        usleep(THREAD_SLEEP);

        if(btn1.GetState() == STATE_CLICKED)
            choice = 1;
        else if(btn2.GetState() == STATE_CLICKED)
            choice = 0;
    }
	//let button sounds finish playing
	while( btnSndOver.IsPlaying() || btnSndClick2.IsPlaying() )
		usleep( THREAD_SLEEP );

	//slide window off the screen
    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while( promptWindow.GetEffect() > 0 )
		usleep(THREAD_SLEEP);

    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
    return choice;
}

void ProgressWindow( const char *title, const char *msg )
{
	GuiWindow promptWindow( 448,288 );
	promptWindow.SetAlignment( ALIGN_CENTRE | ALIGN_MIDDLE );
	promptWindow.SetPosition( 0, -10 );

	GuiImageData dialogBox( Resource( "images/dialogue_box.png" ) );
	GuiImage dialogBoxImg(&dialogBox);

	GuiImageData progressbarOutline( Resource( "images/progressbar_outline.png" ) );
	GuiImage progressbarOutlineImg(&progressbarOutline);
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	progressbarOutlineImg.SetPosition(23, 170);

	GuiImageData progressbarEmpty( Resource( "images/progressbar_empty.png" ) );
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	progressbarEmptyImg.SetPosition(23, 170);
	progressbarEmptyImg.SetTile(100);

	GuiImageData progressbar( Resource( "images/progressbar.png" ) );
	GuiImage progressbarImg(&progressbar);
	progressbarImg.SetAlignment(ALIGN_LEFT | ALIGN_TOP);
	progressbarImg.SetPosition(23, 170);

	/*GuiImageData throbber(throbber_png);
	GuiImage throbberImg(&throbber);
	throbberImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	throbberImg.SetPosition(0, 40);*/

	GuiText titleTxt( font, title, 26, 0x000000ff );
	titleTxt.SetAlignment(ALIGN_CENTRE | ALIGN_TOP);
	titleTxt.SetPosition(0,40);

	GuiText msgTxt( font, msg, 22, 0x000000ff );
	msgTxt.SetAlignment(ALIGN_CENTRE | ALIGN_MIDDLE);
	msgTxt.SetPosition(0, -20);
	msgTxt.SetWrap(true, 400);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

	promptWindow.Append(&progressbarEmptyImg);
	promptWindow.Append(&progressbarImg);
	promptWindow.Append(&progressbarOutlineImg);


	HaltGui();
	int oldState = mainWindow->GetState();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	float workdone = 0.0;
	float workTotal = 400.0;//some arbatrary abount of something to do

	while( workdone < workTotal )
	{
		//show progress
		progressbarImg.SetTile( 100 * workdone / workTotal );

		//do some imaginary work
		workdone += 0.75f;
		if( workdone > workTotal )
			workdone = workTotal;
		usleep( 5000 );

	}

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(oldState);
	ResumeGui();
}

void MenuRender()
{
	//tiny3d_Flip();
	MenuPrepareToDraw();

	//draw main window
	mainWindow->Draw();

	//draw cursors
	for( int i = 3; i >= 0; i-- )
	{
		if( !userInput[ i ].pad.showCursor )
			continue;

		MenuDrawImage( pointer[ i ]->GetRsxTexOffset(), (u32)TINY3D_TEX_FORMAT_A8R8G8B8, pointer[ i ]->GetWidth(), pointer[ i ]->GetHeight(),\
					   pointer[ i ]->GetWPitch(), userInput[ i ].pad.cursorX - ( pointer[ i ]->GetWidth() / 2 ),\
					   userInput[ i ].pad.cursorY - ( pointer[ i ]->GetHeight() / 2 ), 0, 0, 0xff, 1.0 );

	}

	tiny3d_Flip();
	FrameTimer++;
}

int MenuInstall()
{
	int ret = MENU_COVERFLOW;

	DeviceThread::Halt();
	//trigger
	GuiTrigger trigX;
	trigX.SetSimpleTrigger( -1, BTN_CROSS_ );

	//button image
	GuiImageData btnImgData( Resource( "images/button.png" ) );
	GuiImageData btnImgOverData( Resource( "images/button_over.png" ) );

	//button sound
	GuiSound btnSndOver( Resource( "sounds/button_over.wav" ), SOUND_WAV );
	GuiSound btnSndClick2( Resource( "sounds/button_click2.wav" ), SOUND_WAV );
	btnSndClick2.SetVolume( 50 );

	//title text
	char t[ MAX_KEYBOARD_DISPLAY + 1 ];
	strncpy( t, "libps3gui demo", MAX_KEYBOARD_DISPLAY );

	GuiText titleTxt( font, t, 32, GUI_TEXT_COLOR );
	titleTxt.SetPosition( 0, 0 );
	titleTxt.SetAlignment( ALIGN_TOP | ALIGN_CENTER );

	int buttonTop = 110;
	int buttonLeft = 80;
	int buttonX = buttonLeft;
	int buttonY = buttonTop;
	int buttonSpacing = 20;
	int btnTxtY = -5;
	u32 buttonAlignment = ( ALIGN_TOP | ALIGN_LEFT );
	//buttons

	GuiImage buttonImg( &btnImgData );
	GuiImage buttonOverImg( &btnImgOverData );
	GuiText txt( font, "Cancel", buttonImg.GetHeight() - 22, GUI_TEXT_COLOR );
	txt.SetPosition( 0, btnTxtY );
	GuiButton exitBtn( buttonImg.GetWidth(), buttonImg.GetHeight() );
	exitBtn.SetTrigger( &trigX );
	exitBtn.SetImage( &buttonImg );
	exitBtn.SetImageOver( &buttonOverImg );
	exitBtn.SetPosition( buttonX, buttonY );
	exitBtn.SetAlignment( buttonAlignment );
	exitBtn.SetLabel( &txt );
	exitBtn.SetSoundOver( &btnSndOver );
	exitBtn.SetSoundClick( &btnSndClick2 );
	exitBtn.SetEffect( EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50 );
	buttonY += buttonImg.GetHeight() + buttonSpacing;


	GameInstallDialog installWindow( WINDOW_WIDTH, WINDOW_HEIGHT );
	installWindow.SetPosition( 0, 0 );
	installWindow.SetAlignment( ALIGN_CENTER | ALIGN_MIDDLE );

	//installWindow.Append( &exitBtn );


	HaltGui();
	bgImage->SetVisible( false );
	mainWindow->Append( &installWindow );
	ResumeGui();


	while( 1 )
	{
		if( exitBtn.GetState() == STATE_CLICKED )
		{
			exitBtn.ResetState();
			if( WindowPrompt( "Cancel?", "Do you really want to abort installing?", "Yes", "No" ) )
			{
				installWindow.Cancel( true );
				ret = MENU_COVERFLOW;
				break;
			}
		}
		int run = installWindow.Run();
		if( run == GameInstallDialog::CopyDone
			|| run == GameInstallDialog::CopyCanceled )
		{
			//printf( "mmmkay\n" );
			ret = MENU_COVERFLOW;
			break;
		}
		else if( run == GameInstallDialog::CopyStarting )
		{
			installWindow.Append( &exitBtn );
		}
		else if( run != GameInstallDialog::CopyOk )//some error happened here
		{
			ErrorPrompt( "The game was not copied." );
			printf("run: %i\n", run );
			ret = MENU_COVERFLOW;
			break;
		}


		//usleep( THREAD_SLEEP );
	}
	while( btnSndOver.IsPlaying() || btnSndClick2.IsPlaying() )
		usleep( THREAD_SLEEP );

	HaltGui();
	mainWindow->Remove( &installWindow );
	bgImage->SetVisible( true );
	//mainWindow->Remove( &w );
	ResumeGui();
	DeviceThread::Resume();


	return ret;

}

int MenuCoverFlow()
{
	int ret = MENU_EXIT;

	//get the game list
	std::vector<std::string> devs = DeviceThread::CurrentDevList();
	GameList::Get( devs );
	GameList::ListDirty();//call this now to let it know that we have the current list

	//triggers
	GuiTrigger trigX;
	trigX.SetSimpleTrigger( -1, BTN_CROSS_ );
	GuiTrigger trigStart;
	trigStart.SetButtonOnlyTrigger( 0, BTN_START_ );

	//button image
	GuiImageData btnImgData( Resource( "images/button.png" ) );
	GuiImageData btnImgOverData( Resource( "images/button_over.png" ) );

	//button sound
	GuiSound btnSndOver( Resource( "sounds/button_over.wav" ), SOUND_WAV );
	GuiSound btnSndClick2( Resource( "sounds/button_click2.wav" ), SOUND_WAV );
	btnSndClick2.SetVolume( 50 );

	//title text
	char t[ MAX_KEYBOARD_DISPLAY + 1 ];
	strncpy( t, "Pune Warez", MAX_KEYBOARD_DISPLAY );

	//wString stuff;
	//stuff.fromUTF8( "大乱闘スマッシュブラザーズX™" );
	//GuiText titleTxt( font, stuff, 32, GUI_TEXT_COLOR );
	GuiText titleTxt( font,(char*)NULL, 32, GUI_TEXT_COLOR );
//	titleTxt.SetText( stuff.c_str() );
	titleTxt.SetPosition( 0, 0 );
	titleTxt.SetAlignment( ALIGN_TOP | ALIGN_CENTER );

	int buttonTop = 20;
	int buttonLeft = 35;
	int buttonX = buttonLeft;
	int buttonY = buttonTop;
	int buttonSpacing = 20;
	int btnTxtY = -5;
	u32 buttonAlignment = ( ALIGN_TOP | ALIGN_LEFT );
	//button window
	GuiImageData bwImgData( Resource( "images/bottomButtonWindow.png" ) );
	GuiImage bwImg( &bwImgData );
	//append buttons to this window to keep them seperate from the coverflow
	GuiWindow w( bwImgData.GetWidth(), bwImgData.GetHeight() );
	w.SetParent( mainWindow );
	w.SetPosition( 0, 0 );
	w.SetAlignment( ALIGN_CENTER | ALIGN_BOTTOM );
	w.Append( &bwImg );
	w.SetVisible( false );
	bool btnWindow = false;

	//buttons
	GuiImage buttonImg3( &btnImgData );
	GuiImage buttonOverImg3( &btnImgOverData );
	GuiText optionBtnTxt( font, "Options", buttonImg3.GetHeight() - 22, GUI_TEXT_COLOR );
	optionBtnTxt.SetPosition( 0, btnTxtY );
	GuiButton optionBtn( buttonImg3.GetWidth(), buttonImg3.GetHeight() );
	optionBtn.SetTrigger( &trigX );
	optionBtn.SetImage( &buttonImg3 );
	optionBtn.SetImageOver( &buttonOverImg3 );
	optionBtn.SetPosition( buttonX, buttonY );
	optionBtn.SetAlignment( buttonAlignment );
	optionBtn.SetLabel( &optionBtnTxt );
	optionBtn.SetSoundOver( &btnSndOver );
	optionBtn.SetSoundClick( &btnSndClick2 );
	//optionBtn.SetEffect( EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50 );
	buttonX += buttonImg3.GetWidth() + buttonSpacing;

	GuiImage installBtnImg( &btnImgData );
	GuiImage installBtnOverImg( &btnImgOverData );
	GuiText installBtnTxt( font, "Install", installBtnImg.GetHeight() - 22, GUI_TEXT_COLOR );
	installBtnTxt.SetPosition( 0, btnTxtY );
	GuiButton installBtn( installBtnImg.GetWidth(), installBtnImg.GetHeight() );
	installBtn.SetTrigger( &trigX );
	installBtn.SetImage( &installBtnImg );
	installBtn.SetImageOver( &installBtnOverImg );
	installBtn.SetPosition( buttonX, buttonY );
	installBtn.SetAlignment( buttonAlignment );
	installBtn.SetLabel( &installBtnTxt );
	installBtn.SetSoundOver( &btnSndOver );
	installBtn.SetSoundClick( &btnSndClick2 );
	//installBtn.SetEffect( EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50 );
	buttonX += installBtnImg.GetWidth() + buttonSpacing;


	GuiImage buttonImg( &btnImgData );
	GuiImage buttonOverImg( &btnImgOverData );
	GuiText txt( font, "Exit", buttonImg.GetHeight() - 22, GUI_TEXT_COLOR );
	txt.SetPosition( 0, btnTxtY );
	GuiButton exitBtn( buttonImg.GetWidth(), buttonImg.GetHeight() );
	exitBtn.SetTrigger( &trigX );
	exitBtn.SetTrigger( &trigStart );
	exitBtn.SetImage( &buttonImg );
	exitBtn.SetImageOver( &buttonOverImg );
	exitBtn.SetPosition( buttonX, buttonY );
	exitBtn.SetAlignment( buttonAlignment );
	exitBtn.SetLabel( &txt );
	exitBtn.SetSoundOver( &btnSndOver );
	exitBtn.SetSoundClick( &btnSndClick2 );
	buttonX += buttonImg.GetWidth() + buttonSpacing;
	//exitBtn.SetEffect( EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50 );
	//buttonY += buttonImg.GetHeight() + buttonSpacing;

	//coverflow
	BoxBrowser coverflow;

	if( ListContains( devs, "/dev_bdvd" ) )
		w.Append( &installBtn );
	//w.Append( &installBtn );
	w.Append( &optionBtn );
	w.Append( &exitBtn );
	coverflow.Append( &titleTxt );

	HaltGui();
	mainWindow->Append( &coverflow );
	mainWindow->Append( &w );
	ResumeGui();
	//RsxMem::PrintInfo( true );

	while( 1 )
	{
		//the button window is showing
		if( w.IsFocused() )
		{
			if( !btnWindow )//slide the button window onto the screen
			{
				w.SetVisible( true );
				coverflow.IgnoreInput();
				w.SetEffect( EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50 );
				while( w.GetEffect() > 0 )
					usleep( 500 );
				btnWindow = true;
			}
			if( exitBtn.GetState() == STATE_CLICKED )
			{
				exitBtn.ResetState();
				if( WindowPrompt( "Confirm", "Do you want to exit?", "Yes", "No" ) )
					break;
			}
			else if( optionBtn.GetState() == STATE_CLICKED )
			{
				optionBtn.ResetState();
				ret = MENU_SETTINGS;
				break;
			}
			else if( installBtn.GetState() == STATE_CLICKED )
			{
				installBtn.ResetState();
				ret = MENU_INSTALL;
				break;
			}

		}
		//the coverflow is in focus
		else
		{
			if( btnWindow )//slide the button window off the screen
			{
				w.SetEffect( EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50 );
				while( w.GetEffect() > 0 )
					usleep( 500 );
				w.SetVisible( false );
				coverflow.IgnoreInput( false );
				btnWindow = false;
			}


		}
		//check for added and removed devices
		if( DeviceThread::DevListChanged() )
		{
			std::vector<std::string> newDevs = DeviceThread::CurrentDevList();
			std::vector<std::string> added = DeviceThread::DevicesAdded( devs );
			std::vector<std::string> rmed = DeviceThread::DevicesRemoved( devs );
			devs = newDevs;

			//if a device has been added, stop the gui thread to the coverflow
			//wont try to access the gamelist while it is being changed
			bool changed = !rmed.empty() || !added.empty();
			if( changed )
			{
				HaltGui();
				GameList::RemoveGamesOnDevices( rmed );
				GameList::Get( added );

				//if the gamelist changed, clear the coverflow cache
				if( GameList::ListDirty() )
				{
					coverflow.ClearCache();
				}
				ResumeGui();
			}

			//only show install button if there is a game in the drive
			if( ListContains( added, "/dev_bdvd" ) )
				w.Append( &installBtn );
			else if( ListContains( rmed, "/dev_bdvd" ) )
				w.Remove( &installBtn );
		}
		usleep( THREAD_SLEEP );
	}
	while( btnSndOver.IsPlaying() || btnSndClick2.IsPlaying() )
		usleep( THREAD_SLEEP );

	HaltGui();
	mainWindow->Remove( &coverflow );
	mainWindow->Remove( &w );
	printf("leaving coverflow\n");
	ResumeGui();
	printf("really leaving coverflow\n");
	//RsxMem::PrintInfo( true );

	return ret;
}

int MenuSettings()
{
	int ret = MENU_NONE;
	int page = 0;
	//trigger
	GuiTrigger trigX;
	trigX.SetSimpleTrigger( -1, BTN_CROSS_ );

	//button image data
	GuiImageData btnImgData( Resource( "images/button.png" ) );
	GuiImageData btnImgOverData( Resource( "images/button_over.png" ) );

	//button sounds
	GuiSound btnSndOver( Resource( "sounds/button_over.wav" ), SOUND_WAV );
	GuiSound btnSndClick2( Resource( "sounds/button_click2.wav" ), SOUND_WAV );
	btnSndClick2.SetVolume( 50 );

	//title text
	GuiText titleTxt( font, "Settings", 32, GUI_TEXT_COLOR );
	titleTxt.SetPosition( 0, 0 );
	titleTxt.SetAlignment( ALIGN_TOP | ALIGN_CENTER );

	//back button
	GuiImage buttonImg( &btnImgData );
	GuiImage buttonOverImg( &btnImgOverData );
	GuiText txt( font, "Back", buttonImg.GetHeight() - 22, GUI_TEXT_COLOR );
	txt.SetPosition( 0, -5 );
	GuiButton exitBtn( buttonImg.GetWidth(), buttonImg.GetHeight() );
	exitBtn.SetTrigger( &trigX );
	exitBtn.SetImage( &buttonImg );
	exitBtn.SetImageOver( &buttonOverImg );
	exitBtn.SetPosition( 100, -80 );
	exitBtn.SetAlignment( ALIGN_BOTTOM | ALIGN_LEFT );
	exitBtn.SetLabel( &txt );
	exitBtn.SetSoundOver( &btnSndOver );
	exitBtn.SetSoundClick( &btnSndClick2 );

	//main option list
	OptionList options;
	int numOptions = 0;

	options.SetName( numOptions, "Gui");
	options.SetValue( numOptions, " " );

	options.SetName( ++numOptions, "Paths");
	options.SetValue( numOptions, " " );

	//gui opnions
	OptionList guiOptions;
	int numGuiOptions = 0;

	guiOptions.SetName( numGuiOptions, "Horizontal Correction");
	guiOptions.SetValue( numGuiOptions, "%i", Settings::viewportX );

	guiOptions.SetName( ++numGuiOptions, "Verticle Correction");
	guiOptions.SetValue( numGuiOptions, "%i", Settings::viewportY );

	//path settings
	OptionList pathOptions;
	int numPathOptions = 0;

	pathOptions.SetName( numPathOptions, "Install to");
	pathOptions.SetValue( numPathOptions++, "%s", Settings::installDir.c_str() );


	//option browser
#define COL_2_POS 240
	GuiOptionBrowser * optionBrowser = new GuiOptionBrowser( 552, 248, &options );
	optionBrowser->SetPosition( 0, 120 );
	optionBrowser->SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
	optionBrowser->SetCol2Position( COL_2_POS );
	optionBrowser->SetFocus( 1 );

	GuiWindow bw( WINDOW_WIDTH, WINDOW_HEIGHT );
	bw.Append( &exitBtn );
	bw.Append( &titleTxt );

	HaltGui();
	mainWindow->Append( &bw );
	mainWindow->Append( optionBrowser );
	mainWindow->ChangeFocus( optionBrowser );
	ResumeGui();


	while( ret == MENU_NONE )
	{

		int cl = optionBrowser->GetClickedOption();
		//if( cl > -1 )
		//	printf("cl: %i  page: %i\n", cl, page );
		switch( page )
		{
		case 0://we are on the main option page
			switch( cl )
			{
			case 0://change to gui page
				page = 1;
				HaltGui();
				mainWindow->Remove( optionBrowser );
				delete optionBrowser;
				optionBrowser = new GuiOptionBrowser( 552, 248, &guiOptions );
				optionBrowser->SetPosition( 0, 120 );
				optionBrowser->SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
				optionBrowser->SetCol2Position( COL_2_POS );
				mainWindow->Append( optionBrowser );
				mainWindow->ChangeFocus( optionBrowser );
				ResumeGui();
				break;
			case 1://change to path page
				page = 2;
				HaltGui();
				mainWindow->Remove( optionBrowser );
				delete optionBrowser;;
				optionBrowser = new GuiOptionBrowser( 552, 248, &pathOptions );
				optionBrowser->SetPosition( 0, 120 );
				optionBrowser->SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
				optionBrowser->SetCol2Position( COL_2_POS );
				mainWindow->Append( optionBrowser );
				mainWindow->ChangeFocus( optionBrowser );
				ResumeGui();
				break;
			}
			break;
		case 1://we are on the gui settings page
			switch( cl )
			{
			case 0://change viewport h
				Settings::viewportX += 5;
				if( Settings::viewportX > VIEWPORT_MAX )
					Settings::viewportX = VIEWPORT_MIN;
				AdjustViewport( Settings::viewportX, Settings::viewportY );
				guiOptions.SetValue( cl, "%i", Settings::viewportX );
				break;
			case 1://change viewport v
				Settings::viewportY += 5;
				if( Settings::viewportY > VIEWPORT_MAX )
					Settings::viewportY = VIEWPORT_MIN;
				AdjustViewport( Settings::viewportX, Settings::viewportY );
				guiOptions.SetValue( cl, "%i", Settings::viewportY );
				break;
			}
			optionBrowser->TriggerUpdate();
			break;
		case 2://we are on the path page
			switch( cl )
			{
			case 0://change game install folder
				{
					HaltGui();
					mainWindow->Remove( optionBrowser );
					mainWindow->Remove( &bw );
					ResumeGui();

					string loc = BrowseForEntry( "/", Settings::installDir, DIR_DIRS, DIR_DIRS, "Select Game Install Location" );
					if( !loc.empty() )
					{
						Settings::installDir = loc;
						pathOptions.SetValue( cl, "%s", Settings::installDir.c_str() );
					}

					HaltGui();
					mainWindow->Append( optionBrowser );
					mainWindow->Append( &bw );
					mainWindow->ChangeFocus( optionBrowser );
					ResumeGui();
				}
				break;
			}
			break;
			optionBrowser->TriggerUpdate();
		}

		if( exitBtn.GetState() == STATE_CLICKED )
		{
			exitBtn.ResetState();
			switch( page )
			{
			case 0:
				ret = MENU_COVERFLOW;
				break;
			default:
				page = 0;
				HaltGui();
				mainWindow->Remove( optionBrowser );
				delete optionBrowser;
				optionBrowser = new GuiOptionBrowser( 552, 248, &options );
				optionBrowser->SetPosition( 0, 120 );
				optionBrowser->SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
				optionBrowser->SetCol2Position( COL_2_POS );
				mainWindow->Append( optionBrowser );
				mainWindow->ChangeFocus( optionBrowser );
				ResumeGui();
				break;
			}
		}

		usleep( THREAD_SLEEP );
	}
	//wait for button sound effects to play before destroying them
	while( btnSndOver.IsPlaying() || btnSndClick2.IsPlaying() )
		usleep( THREAD_SLEEP );

	HaltGui();
	mainWindow->Remove( optionBrowser );
	mainWindow->Remove( &bw );
	delete optionBrowser;
	ResumeGui();

	return ret;

}

int MenuAbout()
{
	GuiWindow promptWindow( WINDOW_WIDTH, WINDOW_HEIGHT );
	promptWindow.SetAlignment( ALIGN_CENTRE | ALIGN_MIDDLE );


	GuiTrigger trigA;
	trigA.SetButtonOnlyInFocusTrigger( -1, BTN_CROSS_ );

	GuiImageData dialogBox( Resource( "images/aboutWindow.png" ) );
	GuiImage dialogBoxImg( &dialogBox );
	dialogBoxImg.SetPosition( -5, -5 );//the image is not centered due to the shadow i included in it
	dialogBoxImg.SetAlignment( ALIGN_CENTER | ALIGN_MIDDLE );

	GuiText titleTxt( font, "libps3gui demo", 30, 0x000000ff  );
	titleTxt.SetAlignment(ALIGN_CENTRE| ALIGN_TOP);
	titleTxt.SetPosition( 0,40 );


	char msg[ 2048 ];
	snprintf( msg, sizeof( msg ), \
			  "This is a quick demo for libps3gui.  The library is based off libwiigui ((c) Tantric 2009).  "\
			  "This ps3 port is done by giantpune, including code from hermes and dimok.  "\
			  "It uses tiny3d for drawing.  Controller input and drawing is done on 1 thread.  "\
			  "Sound is played via hermes\' spu module and the background music is being converted from ogg "\
			  "to pcm on a thread.  The API is very similar to libwiigui.  "\
			  "The connect 4 game engine is written by Keith Pomakis.  "\
			  "This code is licensed under the GPLv2 license.");
	GuiText msgTxt( font, msg, 24, 0x000000ff );
	msgTxt.SetAlignment( ALIGN_CENTER | ALIGN_TOP );
	msgTxt.SetPosition( 0, 120 );
	msgTxt.SetWrap( true, dialogBoxImg.GetWidth() - 50 );


	GuiButton btn1( 0, 0 );
	btn1.SetTrigger( &trigA );


	promptWindow.Append( &dialogBoxImg );
	promptWindow.Append( &titleTxt );
	promptWindow.Append( &msgTxt );
	promptWindow.Append( &btn1 );

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	while( 1 )
	{
		usleep( THREAD_SLEEP );

		if( btn1.GetState() == STATE_CLICKED )
			break;
	}

	//slide window off the screen
	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while( promptWindow.GetEffect() > 0 )
		usleep(THREAD_SLEEP);

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return MENU_COVERFLOW;
}

int MainMenu( int menu )
{
	//create cursors
	pointer[ 0 ] = new GuiImageData( Resource( "images/player1_point.png" ) );
	pointer[ 1 ] = new GuiImageData( Resource( "images/player2_point.png" ) );
	pointer[ 2 ] = new GuiImageData( Resource( "images/player3_point.png" ) );
	pointer[ 3 ] = new GuiImageData( Resource( "images/player4_point.png" ) );

	//create main window, font, background sound, and background
	mainWindow = new GuiWindow( WINDOW_WIDTH, WINDOW_HEIGHT );
	//bgImgData = new GuiImageData( background_png, background_png_size, TINY3D_TEX_FORMAT_R5G6B5 );
	bgImgData = new GuiImageData( Resource( "images/background.png" ), TINY3D_TEX_FORMAT_R5G6B5 );
	bgImage = new GuiImage( bgImgData );
	bgImage->SetPosition( 0, 0, 0xffff );
	bgImage->SetAlignment( ALIGN_CENTER | ALIGN_MIDDLE );
	//font = new GuiFont( SCE_PS3_NR_R_JPN );
	//font = new GuiDualFont( SCE_PS3_VR_R_LATIN2 );
	font = new GuiDualFont();

	//store characters commonly found in titles up front to avoid fragmenting the rsx memory
	font->CacheRange( ' ', '~' );		//ascii set
	font->CacheRange( 0x2122, 0x2122 );	//tm sign
	font->CacheRange( 0xae, 0xae );		//(R) sign

	bgMusic = new GuiSound( Resource( "sounds/bg_music.ogg" ), SOUND_OGG );
	bgMusic->SetLoop( true );
	bgMusic->SetVolume( 10 );
	bgMusic->Play();

	mainWindow->Append( bgImage );

	//printf("all global elements created.  starting up gui thread\n");
	ResumeGui();


	while( menu != MENU_EXIT )
	{
		switch( menu )
		{
		case MENU_SETTINGS:
			menu = MenuSettings();
			break;
		case MENU_ABOUT:
			menu = MenuAbout();
			break;
		default:
		case MENU_COVERFLOW:
			menu = MenuCoverFlow();
			break;
		case MENU_INSTALL:
			menu = MenuInstall();
			break;
		}
	}
	return menu;
}

