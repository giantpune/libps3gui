/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 * giantpune 2011
 *
 * gui_optionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "utils.h"

#define OPTION_TEXT_COLOR 0xf0f0f0ff
#define COL_2_START_X	150

extern GuiFont * font;

OptionList::OptionList()
{
}

OptionList::~OptionList()
{
    ClearList();
}

void OptionList::SetName(int i, const char *format, ...)
{
	if(i < (int) name.size())
		name[i].clear();

	if(!format)
		return;

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		if(i >= (int) name.size())
		{
			Resize(i+1);
		}

		name[i].assign(tmp);

		listChanged = true;
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

void OptionList::SetName(int i, const std::string &text )
{
	if(i < (int) name.size())
		name[i].clear();

	if(i >= (int) name.size())
	{
		Resize( i + 1 );
	}

	name[ i ].assign( text );
	listChanged = true;
}

void OptionList::SetValue(int i, const char *format, ...)
{
	if(i < (int) value.size())
		value[i].clear();

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		if(i >= (int) value.size())
		{
			Resize(i+1);
		}

		value[i].assign(tmp);

		listChanged = true;
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

void OptionList::SetValue( int i, const std::string &text )
{
	if(i < (int) value.size())
		value[i].clear();

	if(i >= (int) value.size())
	{
		Resize(i+1);
	}
	value[i].assign( text );
	listChanged = true;
}

const char * OptionList::GetName(int i)
{
	if(i < 0 || i >= (int) name.size())
		return NULL;

	return name.at(i).c_str();
}

const char * OptionList::GetValue(int i)
{
	if(i < 0 || i >= (int) value.size())
		return NULL;

	return value.at(i).c_str();
}

void OptionList::Resize(int size)
{
    name.resize(size);
    value.resize(size);
    listChanged = true;
}

void OptionList::RemoveOption(int i)
{
    if(i < 0 || i >= (int) name.size())
        return;

    name.erase(name.begin()+i);
    value.erase(value.begin()+i);
    listChanged = true;
}

void OptionList::ClearList()
{
    name.clear();
    value.clear();
	std::vector<std::string>().swap(name);
	std::vector<std::string>().swap(value);
    listChanged = true;
}

GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l)
{
    width = w;
    height = h;
    options = l;
    selectable = true;
    listOffset = this->FindMenuItem(-1, 1);
    listChanged = true; // trigger an initial list update
    selectedItem = 0;
    focus = 0; // allow focus

    trigA = new GuiTrigger;
    trigA->SetSimpleTrigger( -1, BTN_CROSS_ );

	btnSoundOver = new GuiSound( Resource( "sounds/button_over.wav" ), SOUND_WAV );
	btnSoundClick = new GuiSound( Resource( "sounds/button_click.pcm" ), SOUND_PCM );
	btnSoundClick->SetVolume( 10 );

	bgOptions = new GuiImageData( Resource( "images/bg_options.png" ) );
    bgOptionsImg = new GuiImage(bgOptions);
    bgOptionsImg->SetParent(this);
    bgOptionsImg->SetAlignment( ALIGN_LEFT | ALIGN_MIDDLE );

	bgOptionsEntry = new GuiImageData( Resource( "images/bg_options_entry.png" ) );

	scrollbar = new GuiImageData( Resource( "images/scrollbar.png" ) );
    scrollbarImg = new GuiImage(scrollbar);
    scrollbarImg->SetParent(this);
    scrollbarImg->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
    scrollbarImg->SetPosition(0, 30);

	arrowDown = new GuiImageData( Resource( "images/scrollbar_arrowdown.png" ) );
    arrowDownImg = new GuiImage(arrowDown);
	arrowDownOver = new GuiImageData( Resource( "images/scrollbar_arrowdown_over.png" ) );
    arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUp = new GuiImageData( Resource( "images/scrollbar_arrowup.png" ) );
    arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData( Resource( "images/scrollbar_arrowup_over.png" ) );
    arrowUpOverImg = new GuiImage(arrowUpOver);

    arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
    arrowUpBtn->SetParent(this);
    arrowUpBtn->SetImage(arrowUpImg);
    arrowUpBtn->SetImageOver(arrowUpOverImg);
    arrowUpBtn->SetAlignment(ALIGN_RIGHT | ALIGN_TOP);
    arrowUpBtn->SetSelectable(false);
    arrowUpBtn->SetTrigger(trigA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);

    arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
    arrowDownBtn->SetParent(this);
    arrowDownBtn->SetImage(arrowDownImg);
    arrowDownBtn->SetImageOver(arrowDownOverImg);
    arrowDownBtn->SetAlignment(ALIGN_RIGHT | ALIGN_BOTTOM);
    arrowDownBtn->SetSelectable(false);
    arrowDownBtn->SetTrigger(trigA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);

    for(int i=0; i<PAGESIZE; i++)
    {
		optionTxt[i] = new GuiText( font, (char *)NULL, 20, OPTION_TEXT_COLOR );
        optionTxt[i]->SetAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
        optionTxt[i]->SetPosition(8,0);
		optionTxt[i]->SetMaxWidth( COL_2_START_X - 20 );
		optionTxt[i]->SetScroll( SCROLL_HORIZONTAL );

		optionVal[i] = new GuiText( font, (char *)NULL, 20, OPTION_TEXT_COLOR );
        optionVal[i]->SetAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
		optionVal[i]->SetPosition( COL_2_START_X, 0 );
		optionVal[i]->SetMaxWidth( ( bgOptionsEntry->GetWidth() - COL_2_START_X ) - 40 );
		optionVal[i]->SetScroll( SCROLL_HORIZONTAL );

        optionBg[i] = new GuiImage(bgOptionsEntry);

        optionBtn[i] = new GuiButton(512,30);
        optionBtn[i]->SetParent(this);
        optionBtn[i]->SetLabel(optionTxt[i], 0);
        optionBtn[i]->SetLabel(optionVal[i], 1);
        optionBtn[i]->SetImageOver(optionBg[i]);
        optionBtn[i]->SetPosition(2,30*i+3);
        optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetSoundClick(btnSoundClick);
    }
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
    delete arrowUpBtn;
    delete arrowDownBtn;

    delete bgOptionsImg;
    delete scrollbarImg;
    delete arrowDownImg;
    delete arrowDownOverImg;
    delete arrowUpImg;
    delete arrowUpOverImg;

    delete bgOptions;
    delete bgOptionsEntry;
    delete scrollbar;
    delete arrowDown;
    delete arrowDownOver;
    delete arrowUp;
    delete arrowUpOver;

    delete trigA;
	delete btnSoundOver;
	delete btnSoundClick;

    for(int i=0; i<PAGESIZE; i++)
    {
        delete optionTxt[i];
        delete optionVal[i];
        delete optionBg[i];
        delete optionBtn[i];
    }
}

void GuiOptionBrowser::SetCol2Position(int x)
{
	bool scroll = false;
	int col2Width = ( bgOptionsEntry->GetWidth() - x );
	if( col2Width > 20 )
	{
		scroll = true;
		col2Width -= 20;
	}
    for(int i=0; i<PAGESIZE; i++)
	{
		if( x > 20 )
		{
			optionTxt[i]->SetMaxWidth( x - 20 );
		}
		optionVal[i]->SetPosition( x, 0 );
		if( scroll )
			optionVal[i]->SetMaxWidth( col2Width - 10 );
	}
}

void GuiOptionBrowser::SetFocus(int f)
{
    focus = f;

    for(int i=0; i<PAGESIZE; i++)
        optionBtn[i]->ResetState();

    if(f == 1)
        optionBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiOptionBrowser::ResetState()
{
    if(state != STATE_DISABLED)
    {
        state = STATE_DEFAULT;
        stateChan = -1;
    }

    for(int i=0; i<PAGESIZE; i++)
    {
        optionBtn[i]->ResetState();
    }
}

int GuiOptionBrowser::GetClickedOption()
{
	int found = -1;
	for(int i=0; i<PAGESIZE; i++)
	{
		if(optionBtn[i]->GetState() == STATE_CLICKED)
		{
			optionBtn[i]->SetState(STATE_SELECTED);
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

int GuiOptionBrowser::GetSelectedOption()
{
	int found = -1;
	for(int i=0; i<PAGESIZE; i++)
	{
		if( optionBtn[i]->GetState() == STATE_SELECTED )
		{
			found = optionIndex[ i ];
			break;
		}
	}
	return found;
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiOptionBrowser::FindMenuItem(int currentItem, int direction)
{
    int nextItem = currentItem + direction;

    if (nextItem < 0 || nextItem >= options->GetLength()) return -1;

    if (strlen(options->GetName(nextItem)) > 0)
        return nextItem;

    return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiOptionBrowser::Draw()
{
    if(!this->IsVisible())
        return;

    bgOptionsImg->Draw();

    int next = listOffset;

    for(int i=0; i<PAGESIZE; i++)
    {
        if(next >= 0)
        {
            optionBtn[i]->Draw();
            next = this->FindMenuItem(next, 1);
        }
        else
            break;
    }

    scrollbarImg->Draw();
    arrowUpBtn->Draw();
    arrowDownBtn->Draw();

    this->UpdateEffects();
}

void GuiOptionBrowser::TriggerUpdate()
{
    listChanged = true;
}

void GuiOptionBrowser::Update(GuiTrigger * t)
{
    if(state == STATE_DISABLED || !t)
        return;

    int next, prev;

    arrowUpBtn->Update(t);
    arrowDownBtn->Update(t);

    next = listOffset;

    if(listChanged)
    {
        listChanged = false;
        for(int i=0; i<PAGESIZE; i++)
        {
            if(next >= 0)
            {
                if(optionBtn[i]->GetState() == STATE_DISABLED)
                {
                    optionBtn[i]->SetVisible(true);
                    optionBtn[i]->SetState(STATE_DEFAULT);
                }

				wString ws;
				ws.fromUTF8( options->GetName( next ) );
				optionTxt[i]->SetText( ws.c_str() );
				ws.fromUTF8( options->GetValue( next ) );
				optionVal[i]->SetText( ws.c_str() );
				//optionTxt[i]->SetText( options->GetName( next ) );
				//optionVal[i]->SetText( options->GetValue( next ) );
                optionIndex[i] = next;
                next = this->FindMenuItem(next, 1);
            }
            else
			{
                optionBtn[i]->SetVisible(false);
                optionBtn[i]->SetState(STATE_DISABLED);
            }
        }
    }

    for(int i=0; i<PAGESIZE; i++)
    {
        if(i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED)
            optionBtn[i]->ResetState();
        else if(focus && i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT)
            optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);

        int currChan = t->chan;

        if(t->pad.showCursor && !optionBtn[i]->IsInside( t->pad.cursorX, t->pad.cursorY ) )
            t->chan = -1;

        optionBtn[i]->Update(t);
        t->chan = currChan;

        if(optionBtn[i]->GetState() == STATE_SELECTED)
		{
			//printf("selected: %i\n", i );
			selectedItem = i;
			optionVal[i]->SetScroll( SCROLL_HORIZONTAL );
			optionTxt[i]->SetScroll( SCROLL_HORIZONTAL );
		}
		else
		{
			optionTxt[i]->SetScroll( SCROLL_NONE );
			optionVal[i]->SetScroll( SCROLL_NONE );
		}
    }

    // pad/joystick navigation
    if(!focus)
        return; // skip navigation

    if(t->Down() || arrowDownBtn->GetState() == STATE_CLICKED)
    {
        next = this->FindMenuItem(optionIndex[selectedItem], 1);

        if(next >= 0)
        {
            if(selectedItem == PAGESIZE-1)
            {
                // move list down by 1
                listOffset = this->FindMenuItem(listOffset, 1);
                listChanged = true;
            }
            else if(optionBtn[selectedItem+1]->IsVisible())
            {
                optionBtn[selectedItem]->ResetState();
                optionBtn[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
                selectedItem++;
            }
        }
        arrowDownBtn->ResetState();
    }
    else if(t->Up() || arrowUpBtn->GetState() == STATE_CLICKED)
    {
        prev = this->FindMenuItem(optionIndex[selectedItem], -1);

        if(prev >= 0)
        {
            if(selectedItem == 0)
            {
                // move list up by 1
                listOffset = prev;
                listChanged = true;
            }
            else
            {
                optionBtn[selectedItem]->ResetState();
                optionBtn[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
                selectedItem--;
            }
        }
        arrowUpBtn->ResetState();
    }

    if(updateCB)
        updateCB(this);
}
