/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 * giantpune 2011
 *
 * gui_keyboard.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <unistd.h>
#include "gui.h"
#include "menu.h"
#include "pad.h"
#include "utils.h"

INC_FILE( keyboard_backspace_over_png );
INC_FILE( keyboard_clear_over_png );
INC_FILE( keyboard_key_png );
INC_FILE( keyboard_key_over_png );
INC_FILE( keyboard_largekey_over_png );
INC_FILE( keyboard_mediumkey_over_png );
INC_FILE( keyboard_largekey_png );
INC_FILE( keyboard_mediumkey_png );
INC_FILE( keyboard_textbox_png );

INC_FILE( button_png );
INC_FILE( button_over_png );

INC_FILE( button_click_pcm );
INC_FILE( button_over_pcm );
INC_FILE( button_over_wav );
INC_FILE( button_click2_wav );

extern GuiWindow* mainWindow;
extern GuiFont * font;

static char tmptxt[MAX_KEYBOARD_DISPLAY];

static char * GetDisplayText(char * t)
{
	if(!t)
		return NULL;

	int len = strlen(t);

	if(len < MAX_KEYBOARD_DISPLAY)
		return t;

	strncpy(tmptxt, &t[len-MAX_KEYBOARD_DISPLAY], MAX_KEYBOARD_DISPLAY);
	tmptxt[MAX_KEYBOARD_DISPLAY-1] = 0;
	return &tmptxt[0];
}

/**
 * Constructor for the GuiKeyboard class.
 */

GuiKeyboard::GuiKeyboard(char * t, u32 max)
{
	width = 540;
	height = 400;
	shift = 0;
	caps = 0;
	selectable = true;
	focus = 0; // allow focus
	alignment = ALIGN_CENTRE | ALIGN_MIDDLE;
	//alignmentHor = ALIGN_CENTRE;
	//alignmentVert = ALIGN_MIDDLE;
	strncpy(kbtextstr, t, max);
	kbtextstr[max] = 0;
	kbtextmaxlen = max;

	Key thekeys[4][11] = {
	{
		{'1','!'},
		{'2','@'},
		{'3','#'},
		{'4','$'},
		{'5','%'},
		{'6','^'},
		{'7','&'},
		{'8','*'},
		{'9','('},
		{'0',')'},
		{'\0','\0'}
	},
	{
		{'q','Q'},
		{'w','W'},
		{'e','E'},
		{'r','R'},
		{'t','T'},
		{'y','Y'},
		{'u','U'},
		{'i','I'},
		{'o','O'},
		{'p','P'},
		{'-','_'}
	},
	{
		{'a','A'},
		{'s','S'},
		{'d','D'},
		{'f','F'},
		{'g','G'},
		{'h','H'},
		{'j','J'},
		{'k','K'},
		{'l','L'},
		{';',':'},
		{'\'','"'}
	},

	{
		{'z','Z'},
		{'x','X'},
		{'c','C'},
		{'v','V'},
		{'b','B'},
		{'n','N'},
		{'m','M'},
		{',','<'},
		{'.','>'},
		{'/','?'},
		{'\0','\0'}
	}
	};
	memcpy( keys, thekeys, sizeof( thekeys ) );


	keyTextbox = new GuiImageData( keyboard_textbox_png, keyboard_textbox_png_size );
	keyTextboxImg = new GuiImage( keyTextbox );
	keyTextboxImg->SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
	keyTextboxImg->SetPosition(0, 0);
	this->Append( keyTextboxImg );


    kbText = new GuiText( font, GetDisplayText(kbtextstr), 20, 0x000000ff );
	kbText->SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
    kbText->SetPosition( 0, 10 );
    this->Append(kbText);

	key = new GuiImageData( keyboard_key_png, keyboard_key_png_size );
	keyOver = new GuiImageData( keyboard_key_over_png, keyboard_key_over_png_size );
	keyMedium = new GuiImageData( keyboard_mediumkey_png, keyboard_mediumkey_png_size );
	keyMediumOver = new GuiImageData(keyboard_mediumkey_over_png, keyboard_mediumkey_over_png_size );
	keyLarge = new GuiImageData( keyboard_largekey_png, keyboard_largekey_png_size );
    keyLargeOver = new GuiImageData( keyboard_largekey_over_png, keyboard_largekey_over_png_size );

	keySoundOver = new GuiSound( button_over_wav, button_over_wav_size, SOUND_WAV );
	keySoundClick = new GuiSound( button_over_pcm, button_over_pcm_size, SOUND_PCM );
	keySoundClick->SetVolume( 50 );
    trigA = new GuiTrigger;
    trigSQ = new GuiTrigger;

    trigA->SetSimpleTrigger( -1, BTN_CROSS_ );
    trigSQ->SetButtonOnlyTrigger( -1, BTN_SQUARE_ );

    char txt[2] = { 0, 0 };

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<11; j++)
        {
            if(keys[i][j].ch != '\0')
            {
                txt[0] = keys[i][j].ch;
                keyImg[i][j] = new GuiImage(key);
                keyImgOver[i][j] = new GuiImage(keyOver);
                keyTxt[i][j] = new GuiText( font, txt, 20, 0x000000ff );
                keyTxt[i][j]->SetAlignment( ALIGN_CENTRE | ALIGN_BOTTOM );
                keyTxt[i][j]->SetPosition(0, -14);
                keyBtn[i][j] = new GuiButton(key->GetWidth(), key->GetHeight());
                keyBtn[i][j]->SetImage(keyImg[i][j]);
                keyBtn[i][j]->SetImageOver(keyImgOver[i][j]);
				keyBtn[i][j]->SetSoundOver(keySoundOver);
				keyBtn[i][j]->SetSoundClick(keySoundClick);
                keyBtn[i][j]->SetTrigger(trigA);
                keyBtn[i][j]->SetLabel(keyTxt[i][j]);
                keyBtn[i][j]->SetPosition(j*42+21*i+40, i*42+80);
                keyBtn[i][j]->SetEffectGrow();
                this->Append(keyBtn[i][j]);
            }
        }
    }

	keyBackImg = new GuiImage(keyMedium);
	keyBackOverImg = new GuiImage(keyMediumOver);
    keyBackText = new GuiText( font, "Back", 20, 0x000000ff );
    keyBackText->SetPosition( 0, -4 );
	keyBack = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyBack->SetImage(keyBackImg);
	keyBack->SetImageOver(keyBackOverImg);
	keyBack->SetLabel(keyBackText);
	keyBack->SetSoundOver(keySoundOver);
	keyBack->SetSoundClick(keySoundClick);
    keyBack->SetTrigger(trigA);
    keyBack->SetTrigger(trigSQ);
	keyBack->SetPosition(10*42+40, 0*42+80);
	keyBack->SetEffectGrow();
	this->Append(keyBack);

	keyCapsImg = new GuiImage(keyMedium);
	keyCapsOverImg = new GuiImage(keyMediumOver);
    keyCapsText = new GuiText( font, "Caps", 20, 0x000000ff);
    keyCapsText->SetPosition( 0, -4 );
	keyCaps = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyCaps->SetImage(keyCapsImg);
	keyCaps->SetImageOver(keyCapsOverImg);
	keyCaps->SetLabel(keyCapsText);
	keyCaps->SetSoundOver(keySoundOver);
	keyCaps->SetSoundClick(keySoundClick);
	keyCaps->SetTrigger(trigA);
	keyCaps->SetPosition(0, 2*42+80);
	keyCaps->SetEffectGrow();
	this->Append(keyCaps);

	keyShiftImg = new GuiImage(keyMedium);
	keyShiftOverImg = new GuiImage(keyMediumOver);
    keyShiftText = new GuiText( font, "Shift", 20, 0x000000ff );
    keyShiftText->SetPosition( 0, -4 );
	keyShift = new GuiButton(keyMedium->GetWidth(), keyMedium->GetHeight());
	keyShift->SetImage(keyShiftImg);
	keyShift->SetImageOver(keyShiftOverImg);
	keyShift->SetLabel(keyShiftText);
	keyShift->SetSoundOver(keySoundOver);
	keyShift->SetSoundClick(keySoundClick);
	keyShift->SetTrigger(trigA);
	keyShift->SetPosition(21, 3*42+80);
	keyShift->SetEffectGrow();
	this->Append(keyShift);

	keySpaceImg = new GuiImage(keyLarge);
	keySpaceOverImg = new GuiImage(keyLargeOver);
	keySpace = new GuiButton(keyLarge->GetWidth(), keyLarge->GetHeight());
	keySpace->SetImage(keySpaceImg);
	keySpace->SetImageOver(keySpaceOverImg);
	keySpace->SetSoundOver(keySoundOver);
	keySpace->SetSoundClick(keySoundClick);
	keySpace->SetTrigger(trigA);
	keySpace->SetPosition(0, 4*42+80);
	keySpace->SetAlignment( ALIGN_CENTRE | ALIGN_TOP );
	keySpace->SetEffectGrow();
	this->Append(keySpace);



}

/**
 * Destructor for the GuiKeyboard class.
 */
GuiKeyboard::~GuiKeyboard()
{
	delete kbText;
	delete keyTextbox;
	delete keyTextboxImg;
	delete keyCapsText;
	delete keyCapsImg;
	delete keyCapsOverImg;
	delete keyCaps;
	delete keyShiftText;
	delete keyShiftImg;
	delete keyShiftOverImg;
	delete keyShift;
	delete keyBackText;
	delete keyBackImg;
	delete keyBackOverImg;
	delete keyBack;
	delete keySpaceImg;
	delete keySpaceOverImg;
	delete keySpace;
	delete key;
	delete keyOver;
	delete keyMedium;
	delete keyMediumOver;
	delete keyLarge;
	delete keyLargeOver;
	delete keySoundOver;
	delete keySoundClick;
    delete trigA;
    delete trigSQ;

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<11; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				delete keyImg[i][j];
				delete keyImgOver[i][j];
				delete keyTxt[i][j];
				delete keyBtn[i][j];
			}
		}
	}
}

void GuiKeyboard::Update(GuiTrigger * t)
{
	if(_elements.size() == 0 || (state == STATE_DISABLED && parentElement))
		return;

	for (u8 i = 0; i < _elements.size(); i++)
	{
		try	{ _elements.at(i)->Update(t); }
		catch (const std::exception& e) { }
    }

	bool update = false;

	if(keySpace->GetState() == STATE_CLICKED)
	{
		if(strlen(kbtextstr) < kbtextmaxlen)
		{
			kbtextstr[strlen(kbtextstr)] = ' ';
			kbText->SetText(kbtextstr);
		}
		keySpace->SetState(STATE_SELECTED, t->chan);
	}
	else if(keyBack->GetState() == STATE_CLICKED)
	{
		kbtextstr[strlen(kbtextstr)-1] = 0;
		kbText->SetText(GetDisplayText(kbtextstr));
		keyBack->SetState(STATE_SELECTED, t->chan);
	}
	else if(keyShift->GetState() == STATE_CLICKED)
	{
		shift ^= 1;
		keyShift->SetState(STATE_SELECTED, t->chan);
		update = true;
	}
	else if(keyCaps->GetState() == STATE_CLICKED)
	{
		caps ^= 1;
		keyCaps->SetState(STATE_SELECTED, t->chan);
		update = true;
	}

	char txt[2] = { 0, 0 };

	startloop:

	for(int i=0; i<4; i++)
	{
		for(int j=0; j<11; j++)
		{
			if(keys[i][j].ch != '\0')
			{
				if(update)
				{
					if(shift || caps)
						txt[0] = keys[i][j].chShift;
					else
						txt[0] = keys[i][j].ch;

					keyTxt[i][j]->SetText(txt);
				}

				if(keyBtn[i][j]->GetState() == STATE_CLICKED)
				{
					if(strlen(kbtextstr) < kbtextmaxlen)
					{
						if(shift || caps)
						{
							kbtextstr[strlen(kbtextstr)] = keys[i][j].chShift;
						}
						else
						{
							kbtextstr[strlen(kbtextstr)] = keys[i][j].ch;
						}
					}
					kbText->SetText(GetDisplayText(kbtextstr));
					keyBtn[i][j]->SetState(STATE_SELECTED, t->chan);

					if(shift)
					{
						shift ^= 1;
						update = true;
						goto startloop;
					}
				}
			}
		}
	}

	this->ToggleFocus(t);

	if(focus) // only send actions to this window if it's in focus
	{
		// pad/joystick navigation
		if(t->Right())
			this->MoveSelectionHor(1);
		else if(t->Left())
			this->MoveSelectionHor(-1);
		else if(t->Down())
			this->MoveSelectionVert(1);
		else if(t->Up())
			this->MoveSelectionVert(-1);
	}
}

void OnScreenKeyboard( char * var, u16 maxlen )
{
	int save = -1;

	GuiKeyboard keyboard( var, maxlen );

	//button sound
	GuiSound btnSndOver( button_over_wav, button_over_wav_size, SOUND_WAV );
	GuiSound btnSndClick2( button_click2_wav, button_click2_wav_size, SOUND_WAV );
	btnSndClick2.SetVolume( 50 );

	GuiImageData btnOutline( button_png, button_png_size );
	GuiImageData btnOutlineOver( button_over_png, button_over_png_size );
	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, BTN_CROSS_ );

    GuiText okBtnTxt( font, "OK", 22, 0x000000ff );
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment( ALIGN_LEFT | ALIGN_BOTTOM );
	okBtn.SetPosition( 25, -25 );

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSndOver);
	okBtn.SetSoundClick( &btnSndClick2 );
	okBtn.SetTrigger(&trigA);
	okBtn.SetEffectGrow();

    GuiText cancelBtnTxt( font, "Cancel", 22, 0x000000ff );
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    cancelBtn.SetAlignment( ALIGN_RIGHT | ALIGN_BOTTOM );
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSndOver);
	cancelBtn.SetSoundClick( &btnSndClick2 );
	cancelBtn.SetTrigger(&trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}
	while( btnSndOver.IsPlaying() || btnSndClick2.IsPlaying() )
		usleep( THREAD_SLEEP );

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}
