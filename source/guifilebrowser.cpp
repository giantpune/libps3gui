
#include <iostream>

#include "devicethread.h"
#include "gui.h"
#include "filebrowser.h"
#include "stringstuff.h"
#include "utils.h"

extern GuiFont * font;

using namespace std;
using namespace FileBrowser;

/**
 * Constructor for the GuiFileBrowser class.
 */
GuiFileBrowser::GuiFileBrowser(int w, int h)
{
	width = w;
	height = h;
	numEntries = 0;
	selectedItem = 0;
	selectable = true;
	listChanged = true; // trigger an initial list update
	focus = 0; // allow focus

	trigX = new GuiTrigger;
	trigX->SetSimpleTrigger( -1, BTN_CROSS_ );

	trigHeldX = new GuiTrigger;
	trigHeldX->SetHeldTrigger( -1, BTN_CROSS_ );

	btnSoundOver = new GuiSound( Resource( "sounds/button_over.pcm" ), SOUND_PCM);
	btnSoundClick = new GuiSound( Resource( "sounds/button_click.pcm" ), SOUND_PCM);

	bgFileSelection = new GuiImageData( Resource( "images/bg_file_selection.png" ) );
	bgFileSelectionImg = new GuiImage( bgFileSelection );
	bgFileSelectionImg->SetParent(this);
	bgFileSelectionImg->SetAlignment(ALIGN_LEFT | ALIGN_MIDDLE);

	bgFileSelectionEntry = new GuiImageData( Resource( "images/bg_file_selection_entry.png" ) );
	fileFolder = new GuiImageData( Resource( "images/folder.png" ) );

	scrollbar = new GuiImageData( Resource( "images/scrollbar_png" ) );
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	scrollbarImg->SetPosition(0, 30);

	arrowDown = new GuiImageData( Resource( "images/scrollbar_arrowdown.png" ) );
	arrowDownImg = new GuiImage( arrowDown );
	arrowDownOver = new GuiImageData( Resource( "images/scrollbar_arrowdown_over.png" ) );
	arrowDownOverImg = new GuiImage( arrowDownOver );
	arrowUp = new GuiImageData( Resource( "images/scrollbar_arrowup.png" ) );
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData( Resource( "images/scrollbar_arrowup_over.png" ) );
	arrowUpOverImg = new GuiImage(arrowUpOver);
	scrollbarBox = new GuiImageData( Resource( "images/scrollbar_box.png" ) );
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOver = new GuiImageData( Resource( "images/scrollbar_box_over.png" ) );
	scrollbarBoxOverImg = new GuiImage( scrollbarBoxOver );

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	arrowUpBtn->SetPosition(0, -2);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetClickable(false);
	arrowUpBtn->SetHoldable(true);
	arrowUpBtn->SetTrigger(trigHeldX);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_RIGHT| ALIGN_BOTTOM);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetClickable(false);
	arrowDownBtn->SetHoldable(true);
	arrowDownBtn->SetTrigger(trigHeldX);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
	scrollbarBoxBtn->SetMinY(0);
	scrollbarBoxBtn->SetMaxY(130);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetClickable(false);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldX);

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		fileListText[i] = new GuiText( font, (char *)NULL, 20, 0x000000ff );
		fileListText[i]->SetAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
		fileListText[i]->SetPosition(5,0);
		fileListText[i]->SetMaxWidth(512);

		fileListBg[i] = new GuiImage(bgFileSelectionEntry);
		fileListFolder[i] = new GuiImage(fileFolder);

		fileList[i] = new GuiButton(512,30);
		fileList[i]->SetParent(this);
		fileList[i]->SetLabel(fileListText[i]);
		fileList[i]->SetImageOver(fileListBg[i]);
		fileList[i]->SetPosition(2,30*i+3);
		fileList[i]->SetTrigger(trigX);
		fileList[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Destructor for the GuiFileBrowser class.
 */
GuiFileBrowser::~GuiFileBrowser()
{
	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;

	delete bgFileSelectionImg;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;

	delete bgFileSelection;
	delete bgFileSelectionEntry;
	delete fileFolder;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;
	delete scrollbarBox;
	delete scrollbarBoxOver;

	while( btnSoundOver->IsPlaying() || btnSoundClick->IsPlaying() )
		usleep( THREAD_SLEEP );

	delete btnSoundOver;
	delete btnSoundClick;
	delete trigHeldX;
	delete trigX;

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		delete fileListText[i];
		delete fileList[i];
		delete fileListBg[i];
		delete fileListFolder[i];
	}
}

void GuiFileBrowser::SetFocus(int f)
{
	focus = f;

	for(int i=0; i<FILE_PAGESIZE; i++)
		fileList[i]->ResetState();

	if(f == 1)
		fileList[selectedItem]->SetState(STATE_SELECTED);
}

void GuiFileBrowser::ResetState()
{
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		fileList[i]->ResetState();
	}
}

void GuiFileBrowser::TriggerUpdate()
{
	listChanged = true;
}

/**
 * Draw the button on screen
 */
void GuiFileBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	bgFileSelectionImg->Draw();

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		fileList[i]->Draw();
	}

	scrollbarImg->Draw();
	arrowUpBtn->Draw();
	arrowDownBtn->Draw();
	scrollbarBoxBtn->Draw();

	this->UpdateEffects();
}

void GuiFileBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;

	int position = 0;
	int positionWiimote = 0;

	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);
	scrollbarBoxBtn->Update(t);

	// move the file listing to respond to wiimote cursor movement
	if(scrollbarBoxBtn->GetState() == STATE_HELD &&
		scrollbarBoxBtn->GetStateChan() == t->chan &&
		t->pad.showCursor &&
		browser.numEntries > FILE_PAGESIZE
		)
	{
		scrollbarBoxBtn->SetPosition(0,0);
		positionWiimote = t->pad.cursorY - 60 - scrollbarBoxBtn->GetTop();

		if(positionWiimote < scrollbarBoxBtn->GetMinY())
			positionWiimote = scrollbarBoxBtn->GetMinY();
		else if(positionWiimote > scrollbarBoxBtn->GetMaxY())
			positionWiimote = scrollbarBoxBtn->GetMaxY();

		browser.pageIndex = (positionWiimote * browser.numEntries)/130.0 - selectedItem;

		if(browser.pageIndex <= 0)
		{
			browser.pageIndex = 0;
		}
		else if(browser.pageIndex+FILE_PAGESIZE >= browser.numEntries)
		{
			browser.pageIndex = browser.numEntries-FILE_PAGESIZE;
		}
		listChanged = true;
		focus = false;
	}

	if(arrowDownBtn->GetState() == STATE_HELD && arrowDownBtn->GetStateChan() == t->chan )
	{
		t->pad.held |= BTN_DOWN_;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
	}
	else if(arrowUpBtn->GetState() == STATE_HELD && arrowUpBtn->GetStateChan() == t->chan )
	{
		t->pad.held |= BTN_UP_;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
	}

	// pad/joystick navigation
	if(!focus)
	{
		goto endNavigation; // skip navigation
		listChanged = false;
	}

	if(t->Right())
	{
		if(browser.pageIndex < browser.numEntries && browser.numEntries > FILE_PAGESIZE)
		{
			browser.pageIndex += FILE_PAGESIZE;
			if(browser.pageIndex+FILE_PAGESIZE >= browser.numEntries)
				browser.pageIndex = browser.numEntries-FILE_PAGESIZE;
			listChanged = true;
		}
	}
	else if(t->Left())
	{
		if(browser.pageIndex > 0)
		{
			browser.pageIndex -= FILE_PAGESIZE;
			if(browser.pageIndex < 0)
				browser.pageIndex = 0;
			listChanged = true;
		}
	}
	else if(t->Down())
	{
		if(browser.pageIndex + selectedItem + 1 < browser.numEntries)
		{
			if(selectedItem == FILE_PAGESIZE-1)
			{
				// move list down by 1
				browser.pageIndex++;
				listChanged = true;
			}
			else if(fileList[selectedItem+1]->IsVisible())
			{
				fileList[selectedItem]->ResetState();
				fileList[++selectedItem]->SetState(STATE_SELECTED, t->chan);
			}
		}
	}
	else if(t->Up())
	{
		if(selectedItem == 0 &&	browser.pageIndex + selectedItem > 0)
		{
			// move list up by 1
			browser.pageIndex--;
			listChanged = true;
		}
		else if(selectedItem > 0)
		{
			fileList[selectedItem]->ResetState();
			fileList[--selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
	}

	endNavigation:

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		if(listChanged || numEntries != browser.numEntries)
		{
			if(browser.pageIndex+i < browser.numEntries)
			{
				if(fileList[i]->GetState() == STATE_DISABLED)
					fileList[i]->SetState(STATE_DEFAULT);

				fileList[i]->SetVisible(true);

				fileListText[i]->SetText(browserList[browser.pageIndex+i].displayname);

				if(browserList[browser.pageIndex+i].isdir) // directory
				{
					fileList[i]->SetIcon(fileListFolder[i]);
					fileListText[i]->SetPosition(30,0);
				}
				else
				{
					fileList[i]->SetIcon(NULL);
					fileListText[i]->SetPosition(10,0);
				}
			}
			else
			{
				fileList[i]->SetVisible(false);
				fileList[i]->SetState(STATE_DISABLED);
			}
		}

		if(i != selectedItem && fileList[i]->GetState() == STATE_SELECTED)
			fileList[i]->ResetState();
		else if(focus && i == selectedItem && fileList[i]->GetState() == STATE_DEFAULT)
			fileList[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->pad.showCursor && !fileList[i]->IsInside( t->pad.cursorX, t->pad.cursorY ) )
			t->chan = -1;

		fileList[i]->Update(t);
		t->chan = currChan;

		if(fileList[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
			browser.selIndex = browser.pageIndex + i;
		}

		if(selectedItem == i)
			fileListText[i]->SetScroll(SCROLL_HORIZONTAL);
		else
			fileListText[i]->SetScroll(SCROLL_NONE);
	}

	// update the location of the scroll box based on the position in the file list
	if(positionWiimote > 0)
	{
		position = positionWiimote; // follow wiimote cursor
	}
	else
	{
		position = 130*(browser.pageIndex + FILE_PAGESIZE/2.0) / (browser.numEntries*1.0);

		if(browser.pageIndex/(FILE_PAGESIZE/2.0) < 1)
			position = 0;
		else if((browser.pageIndex+FILE_PAGESIZE)/(FILE_PAGESIZE*1.0) >= (browser.numEntries)/(FILE_PAGESIZE*1.0))
			position = 130;
	}

	scrollbarBoxBtn->SetPosition(0,position+36);

	listChanged = false;
	numEntries = browser.numEntries;

	if(updateCB)
		updateCB(this);
}

std::string BrowseForEntry( const std::string &browserRoot, const std::string &browserDir, u8 mode, u8 type, const std::string &message )
{
	std::string ret;

	char title[ MAXJOLIET ];
	char filename[ MAXJOLIET ];
	int i;
	u8 oldMode = browser.mode;
	u8 curType = DIR_DIRS;

	if( !browserRoot.size() || ( type != DIR_FILES && type != DIR_DIRS ) )
		return ret;

	if( !( mode & DIR_FILES ) )
		FileBrowser::SetDisplayMode( mode );
//	ShutoffRumble();

	// populate initial directory listing
	if( BrowseDevice( browserRoot, browserDir ) <= 0 )
	{
		cout << "failed to browse directory \"" << browserDir << "\" in \"" << browserRoot << "\". trying root";
		if( BrowseDevice( browserDir, "" ) <= 0 )
		{
			snprintf( title, sizeof( title ), "Couldn\'t get file listing in %s", browserRoot.c_str() );
			ErrorPrompt( title );
			browser.mode = oldMode;
			return ret;
		}
	}

	//set filename in case they try to exit before clicking any of the browser entries
	strncpy( title, browser.dir, sizeof( title ) );
	char* test = strtok( title, "/" );
	char* p = title;
	while( test != NULL)
	{
		test = strtok(NULL,"/");
		if( test )
			p = test;
	}
	strncpy( filename, p, sizeof( filename ) );

	strncpy( title, message.c_str(), sizeof( title ) );

	GuiWindow buttonWindow( WINDOW_WIDTH, WINDOW_HEIGHT );

	GuiText titleTxt( font, title, 28, GUI_TEXT_COLOR );
	titleTxt.SetAlignment( ALIGN_LEFT | ALIGN_TOP );
	titleTxt.SetPosition( 100, 50 );

	GuiTrigger trigA;
	trigA.SetSimpleTrigger( -1, BTN_CROSS_ );

	GuiFileBrowser fileBrowser(552, 248);
	fileBrowser.SetAlignment(ALIGN_CENTRE | ALIGN_TOP);
	fileBrowser.SetPosition(0, 140);

	GuiImageData btnOutline( Resource( "images/button.png" ) );
	GuiImageData btnOutlineOver( Resource( "images/button_over.png" ) );

	GuiText backBtnTxt( font, "Cancel", 24, GUI_TEXT_COLOR );
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT | ALIGN_BOTTOM);
	backBtn.SetPosition(60, -50);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiText okBtnTxt( font, "Ok", 24, GUI_TEXT_COLOR );
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	okBtn.SetAlignment(ALIGN_RIGHT | ALIGN_BOTTOM);
	okBtn.SetPosition( -60, -50);
	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

	//path text box
	GuiImageData keyTextbox( Resource( "images/filebrowser_textbox.png" ) );
	GuiImage keyTextboxImg( &keyTextbox );
	keyTextboxImg.SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
	keyTextboxImg.SetPosition(0, fileBrowser.GetTop() - ( keyTextboxImg.GetHeight() + 4 ) );
	buttonWindow.Append( &keyTextboxImg );

	snprintf( title, sizeof( title ) - 2, "%s%s", rootdir, browser.dir );
	if( title[ strlen( title ) - 1 ] != '/' )
		strcat( title, "/" );

	GuiText pathTxt( font, title, 12, 0x000000ff );
	pathTxt.SetAlignment( ALIGN_LEFT | ALIGN_TOP );
	pathTxt.SetPosition( keyTextboxImg.GetLeft() + 5, keyTextboxImg.GetTop() + 20 );
	pathTxt.SetMaxWidth( keyTextboxImg.GetWidth() - 40 );
	pathTxt.SetScroll( 1 );
	buttonWindow.Append( &pathTxt );

	buttonWindow.Append( &okBtn );
	buttonWindow.Append( &backBtn );

	HaltGui();
	mainWindow->Append(&titleTxt);
	mainWindow->Append(&fileBrowser);
	mainWindow->Append(&buttonWindow);
	ResumeGui();

	//list of current devices
	std::vector<std::string> devList = DeviceThread::CurrentDevList();

	while( 1 )
	{
		usleep( THREAD_SLEEP );

		// update file browser based on arrow buttons
		for( i = 0; i < FILE_PAGESIZE; i++ )
		{
			if(fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();
				// check corresponding browser entry
				if(browserList[browser.selIndex].isdir)
				{
					strncpy( filename, browserList[browser.selIndex].filename, sizeof( filename ) );
					if( FileBrowser::ChangeFolder() )
					{
						snprintf( title, sizeof( title ) - 2, "%s%s", rootdir, browser.dir );
						if( title[ strlen( title ) - 1 ] != '/' )
							strcat( title, "/" );

						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();

						HaltGui();
						pathTxt.SetText( title );
						ResumeGui();

						curType = DIR_DIRS;
					}
					else if( FileBrowser::GoToParentDir() )
					{

					}
					else
					{
						break;
					}
				}
				else
				{
					snprintf( title, sizeof( title ), "%s%s/%s", \
							  rootdir, browser.dir, browserList[browser.selIndex].filename );
					strncpy( filename, browserList[browser.selIndex].filename, sizeof( filename ) );
					HaltGui();
					pathTxt.SetText( title );
					ResumeGui();
					curType = DIR_FILES;
				}
			}
		}
		if( backBtn.GetState() == STATE_CLICKED )
		{
			break;
		}
		if( okBtn.GetState() == STATE_CLICKED )
		{
			okBtn.ResetState();
			if( curType == type )
			{
				ret = title;
				break;
			}

			if( type == DIR_FILES )
			{
				snprintf( title, sizeof( title ), "\"%s\" is not a file.", filename );
				ErrorPrompt( title );
			}
			else if( type == DIR_DIRS )
			{
				snprintf( title, sizeof( title ), "\"%s\" is not a directory.", filename );
				ErrorPrompt( title );
			}
		}

		//check inserted or removed devices
		if( DeviceThread::DevListChanged() )
		{
			//printf("list changed\n");
			std::vector<std::string> newlist = DeviceThread::CurrentDevList();
			std::vector<std::string> rmed = DeviceThread::DevicesRemoved( devList );

			devList = newlist;

			std::string str = browserRoot + browser.dir;
			printf("str: \"%s\"\n", str.c_str());

			//we are showing the root, and the list changed
			if( str == "/" )
			{
				if( !ParseDirectory() )
					break;

				fileBrowser.ResetState();
				fileBrowser.fileList[0]->SetState(STATE_SELECTED);
				fileBrowser.TriggerUpdate();
				continue;
			}

			//see if the device being browsed has been removed
			bool refresh = false;
			bool exit = false;
			std::vector<std::string>::iterator it = rmed.begin();
			while( it < rmed.end() )
			{
				if( StartsWith( browserRoot, *it ) )
				{
					snprintf( title, sizeof( title ), "The device \"%s\" has been removed.", browserRoot.c_str() );
					ErrorPrompt( title );
					exit = true;
					break;
				}
				if( StartsWith( str, *it ) )
				{
					if( BrowseDevice( "/", "" ) <= 0 )
					{
						ErrorPrompt( "Error parsing directory \"/\"." );
						exit = true;
						break;
					}
					refresh = true;

				}
				++it;
			}
			if( exit )
			{
				break;
			}
			if( refresh )
			{
				strcpy( title, "/" );
				strcpy( filename, "/" );
				curType = DIR_DIRS;

				HaltGui();
				pathTxt.SetText( title );
				ResumeGui();

				fileBrowser.ResetState();
				fileBrowser.fileList[0]->SetState(STATE_SELECTED);
				fileBrowser.TriggerUpdate();
				continue;
			}
		}
	}
	HaltGui();
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(&buttonWindow);
	mainWindow->Remove(&fileBrowser);
	ResumeGui();
	browser.mode = oldMode;

	return ret;
}

std::string FindFile( const std::string &browserRoot, const std::string &browserDir )
{
	return BrowseForEntry( browserRoot, browserDir, DIR_DIRS | DIR_FILES, DIR_FILES, "Select a file" );
}

std::string FindDir( const std::string &browserRoot, const std::string &browserDir )
{
	return BrowseForEntry( browserRoot, browserDir, DIR_DIRS, DIR_DIRS, "Select a folder" );
}
