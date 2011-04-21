/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 * giantpune 2011
 *
 * gui_text.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <iostream>
#include "gui.h"
#include "utils.h"

static int currentSize = 0;
static int presetSize = 0;
static int presetMaxWidth = 0;
//static int presetAlignmentHor = 0;
//static int presetAlignmentVert = 0;
static u32 presetAlignment = 0;
static u16 presetStyle = 0;
static u32 presetColor = 0xffffffff;
GuiFont *presetFont = NULL;

//#define TEXT_SCROLL_DELAY			28
#define TEXT_SCROLL_DELAY			8
#define	TEXT_SCROLL_INITIAL_DELAY	6

#define FTGX_JUSTIFY_RIGHT ALIGN_RIGHT
#define FTGX_JUSTIFY_CENTER ALIGN_CENTER
#define FTGX_JUSTIFY_LEFT ALIGN_LEFT
#define FTGX_ALIGN_TOP ALIGN_TOP
#define FTGX_ALIGN_MIDDLE ALIGN_MIDDLE
#define FTGX_ALIGN_BOTTOM ALIGN_BOTTOM

using namespace std;

/**
 * Constructor for the GuiText class.
 */
GuiText::GuiText( const char * t, int s, u32 c )
{
	origText = NULL;
	text = NULL;
	font = presetFont;
	size = s;
	color = c;
	alpha = c & 0xff;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrap = false;
	textDyn = NULL;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	//alignmentHor = ALIGN_CENTRE;
	//alignmentVert = ALIGN_MIDDLE;

	alignment = ( ALIGN_CENTRE | ALIGN_MIDDLE );

	if(t)
	{
		origText = strdup(t);
		text = charToWideChar(t);
	}
}

GuiText::GuiText( GuiFont *fnt, const char * t, int s, u32 c )
{
	origText = NULL;
	text = NULL;
	size = s;
	color = c;
	alpha = c & 0xff;
	font = fnt;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrap = false;
	textDyn = NULL;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	//alignmentHor = ALIGN_CENTRE;
	//alignmentVert = ALIGN_MIDDLE;

	alignment = ( ALIGN_CENTER | ALIGN_MIDDLE );

	if(t)
	{
		origText = strdup(t);
		text = charToWideChar(t);
	}
}

GuiText::GuiText( GuiFont *fnt, const wchar_t * t, int s, u32 c )
{
	origText = NULL;
	text = NULL;
	size = s;
	color = c;
	alpha = c & 0xff;
	font = fnt;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrap = false;
	textDyn = NULL;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignment = ( ALIGN_CENTER | ALIGN_MIDDLE );

	if (t)
	{
		text = new (std::nothrow) wchar_t[wcslen(t) + 1];
		if (!text) return;

		wcscpy(text, t);

//		textWidth = fontSystem->getWidth(text, currentSize);
	}
}

GuiText::GuiText( GuiFont *fnt, const wString &t, int s, u32 c )
{
	origText = NULL;
	text = NULL;
	size = s;
	color = c;
	alpha = c & 0xff;
	font = fnt;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrap = false;
	textDyn = NULL;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignment = ( ALIGN_CENTER | ALIGN_MIDDLE );

	int len = t.size();
	if( len )
	{
		text = new (std::nothrow) wchar_t[ len ];
		if ( !text ) return;

		for( int i = 0; i < len; i++ )
		{
			text[ i ] = t.at( i );
		}

		//delete text;
		//text = NULL;

		//wcscpy( text, t );

//		textWidth = fontSystem->getWidth(text, currentSize);
	}

}

/**
 * Constructor for the GuiText class, uses presets
 */
GuiText::GuiText(const char * t)
{
	origText = NULL;
	text = NULL;
	size = presetSize;
	color = presetColor;
	alpha = presetColor & 0xff;
	style = presetStyle;
	maxWidth = presetMaxWidth;
	font = presetFont;
	wrap = false;
	textDyn = NULL;
	textScroll = SCROLL_NONE;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	//alignmentHor = presetAlignmentHor;
	//alignmentVert = presetAlignmentVert;

	alignment = presetAlignment;

	if(t)
	{
		origText = strdup(t);
		text = charToWideChar(t);
	}
}

/**
 * Destructor for the GuiText class.
 */
GuiText::~GuiText()
{
	if(origText)
		free(origText);
	if(text)
		delete[] text;
	if(textDyn)
		delete[] textDyn;
}

void GuiText::SetText(const char * t)
{
	if(origText)
		free(origText);
	if(text)
		delete[] text;
	if(textDyn)
		delete[] textDyn;

	origText = NULL;
	text = NULL;
	textDyn = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		origText = strdup(t);
		text = charToWideChar(t);
	}
}

void GuiText::SetText(const wchar_t * t)
{
	if(origText)
		free(origText);
	if(text)
		delete[] text;
	if(textDyn)
		delete[] textDyn;

	origText = NULL;
	text = NULL;
	textDyn = NULL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		text = new (std::nothrow) wchar_t[wcslen(t) + 1];
		if (!text) return;

		wcscpy(text, t);
	}
}

void GuiText::SetPresets(int sz, u32 c, int w, u16 s, u32 align )
{
	presetSize = sz;
	presetColor = c;
	presetStyle = s;
	presetMaxWidth = w;
	presetAlignment = align;
}

void GuiText::SetFontSize(int s)
{
	size = s;
}

void GuiText::SetMaxWidth(int width)
{
	maxWidth = width;
}

void GuiText::SetWrap(bool w, int width)
{
	wrap = w;
	maxWidth = width;
}

void GuiText::SetScroll(int s)
{
	if(textScroll == s)
		return;

	if(textDyn)
	{
		delete[] textDyn;
		textDyn = NULL;
	}
	textScroll = s;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;
}

void GuiText::SetColor( u32 c )
{
	color = c;
	alpha = c & 0xff;
}

void GuiText::SetStyle(u16 s)
{
	style = s;
}

/*void GuiText::SetAlignment(int hor, int vert)
{
	style = 0;

	switch(hor)
	{
		case ALIGN_LEFT:
			style |= FTGX_JUSTIFY_LEFT;
			break;
		case ALIGN_RIGHT:
			style |= FTGX_JUSTIFY_RIGHT;
			break;
		default:
			style |= FTGX_JUSTIFY_CENTER;
			break;
	}
	switch(vert)
	{
		case ALIGN_TOP:
			style |= FTGX_ALIGN_TOP;
			break;
		case ALIGN_BOTTOM:
			style |= FTGX_ALIGN_BOTTOM;
			break;
		default:
			style |= FTGX_ALIGN_MIDDLE;
			break;
	}

	alignmentHor = hor;
	alignmentVert = vert;
}*/

/**
 * Draw the text on screen
 */
//must of this is untested in the ps3 version, expect bugz
void GuiText::Draw()
{
	if( !text || !font )
		return;

	if(!this->IsVisible())
		return;

	u32 c = color & 0xffffff00;
	c |= ( this->GetAlpha() & 0xff );

	int newSize = size * this->GetScale();

	if( newSize > MAX_FONT_SIZE )
		newSize = MAX_FONT_SIZE;

	currentSize = newSize;

	if(maxWidth > 0)
	{
		//char * tmpText = strdup( origText );
		int textlen = wcslen( text );
		wchar_t *tmpText = new (std::nothrow) wchar_t[ textlen + 1 ];
		if( !tmpText )
			return;
		wcscpy( tmpText, text );

		u32 maxChar = GetMaxChars();
		int width = 0;

		if( textScroll == SCROLL_HORIZONTAL )
		{
			if( ( textlen > (int)maxChar && ( FrameTimer % textScrollDelay == 0 ) )
				|| !textDyn )//make sure to process the wrap text stuff below for the first time this text is drawn
			{
				if( textDyn && textScrollInitialDelay )
				{
					textScrollInitialDelay--;
				}
				else
				{
					if( textDyn )
					{
						delete[] textDyn;
						textDyn = NULL;
					}
					textDyn = new (std::nothrow) wchar_t[ textlen + 1 ];
					if( textDyn )
					{
						//textScrollPos++;
						if( textScrollPos > textlen - 1 )
						{
							textScrollPos = 0;
							textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
						}
						int off = textScrollPos;
						int idx = 0;

						//copy the first part of the  string
						while( width <= maxWidth && off < textlen )
						{
							width += this->font->TextWidth( text[ off ], currentSize );
							textDyn[ idx++ ] = text[ off ];
							off++;
						}
						//we copied to the end of the string and didnt reach the max width, make it wrap
						if( off == textlen && width < maxWidth )
						{
							width += this->font->TextWidth( ' ', currentSize ) << 1;
							//add same spaces to seperate the string is it wraps
							for( int i = 0;i < 2 && idx < textlen; idx++, i++ )
							{
								textDyn[ idx ] = ' ';
							}

							//draw the beginning of the scring at the end
							off = 0;
							while( width <= maxWidth && idx < textlen )
							{
								width += this->font->TextWidth( text[ off ], currentSize );
								textDyn[ idx++ ] = text[ off++ ];
							}
						}
						textDyn[ idx ] = 0;
						textScrollPos++;
					}
				}
			}
			if( textDyn )
			{
				wcsncpy( tmpText, textDyn, textlen );
				tmpText[ textlen ] = 0;
			}

			int left = this->GetLeft();
			int top = this->GetTop();
			if( !width )
				width = font->TextWidth( tmpText, currentSize );

			if( alignment & ALIGN_RIGHT )
				left -= width;
			else if( alignment & ALIGN_CENTER )
				left -= width/2;
			if( alignment & ALIGN_BOTTOM )
				top -= currentSize;
			else if( alignment & ALIGN_MIDDLE )
				top -= currentSize/2;

			font->DrawText( left, top, 0, tmpText, c, currentSize, style );
		}
		else if( wrap )
		{
			int lineheight = newSize + 6;
			int txtlen = wcslen( text );
			int i = 0;
			int ch = 0;
			int linenum = 0;
			int lastSpace = -1;
			int lastSpaceIndex = -1;
			wchar_t * textrow[ 20 ];
			int rowlen[ 20 ];

			while( ch < txtlen )
			{
				if( i == 0)
				{
					textrow[ linenum ] = new (std::nothrow) wchar_t[ txtlen + 1 ];
					if( !textrow[ linenum ] )
					{
						for( int i = 0; i < linenum; i++ )
							delete textrow[ i ];
						return;
					}
				}

				textrow[ linenum ][ i ] = text[ ch ];
				textrow[ linenum ][ i + 1 ] = 0;


				if( text[ ch ] == ' ' || ch == txtlen - 1 )
				{
					if( wcslen( textrow[ linenum ] ) >= maxChar )
					{
						if( lastSpace >= 0 )
						{
							textrow[ linenum ][ lastSpaceIndex ] = 0; // discard space, and everything after
							rowlen[ linenum ] = lastSpaceIndex;
							ch = lastSpace; // go backwards to the last space
							lastSpace = -1; // we have used this space
							lastSpaceIndex = -1;

						}
						linenum++;
						i = -1;
					}
					else if( ch == txtlen - 1 )
					{
						rowlen[ linenum ] = i;
						linenum++;
					}
				}
				if( text[ ch ] == ' ' && i >= 0 )
				{
					lastSpace = ch;
					lastSpaceIndex = i;
				}
				ch++;
				i++;
			}

			int voffset = 0;

			int left = this->GetLeft();
			int top = this->GetTop();

			if( alignment & ALIGN_BOTTOM )
			{
				voffset = -( lineheight * linenum ) + lineheight;
				top -= currentSize;
			}
			else if( alignment & ALIGN_MIDDLE )
			{
				voffset = -( lineheight * linenum ) / 2 + lineheight / 2;
				top -= currentSize/2;
			}



			for( i = 0; i < linenum; i++ )
			{
				int width = font->TextWidth( textrow[ i ], rowlen[ i ], currentSize );

				int tmpLeft = left;
				if( alignment & ALIGN_RIGHT )
					tmpLeft -= width;
				else if( alignment & ALIGN_CENTER )
					tmpLeft -= width/2;
				font->DrawText( tmpLeft, top + voffset + i * lineheight, 0, textrow[ i ], c, currentSize, style );
				delete[] textrow[ i ];
			}
		}
		else
		{
			int left = this->GetLeft();
			int top = this->GetTop();
			width = font->TextWidth( tmpText, currentSize );

			//the text is too big, truncate it and add ...
			if( width > maxWidth )
			{
				int idx = 0;
				width = this->font->TextWidth( '.', currentSize ) * 3;

				//get the last letter to draw
				while( width <= maxWidth && idx < textlen - 3 )
				{
					width += this->font->TextWidth( tmpText[ idx++ ], currentSize );
				}

				//add ...
				for( int i = 0; i < 3; i++ )
				{
					tmpText[ idx++ ] = '.';
				}
				tmpText[ idx++ ] = 0;
			}

			if( alignment & ALIGN_RIGHT )
				left -= width;
			else if( alignment & ALIGN_CENTER )
				left -= width/2;
			if( alignment & ALIGN_BOTTOM )
				top -= currentSize;
			else if( alignment & ALIGN_MIDDLE )
				top -= currentSize/2;
			font->DrawText( left, top, 0, tmpText, c, currentSize, style );
		}
		delete[] tmpText;
	}
	else
	{
		int left = this->GetLeft();
		int top = this->GetTop();
		int width = font->TextWidth( text, currentSize );

		if( alignment & ALIGN_RIGHT )
			left -= width;
		else if( alignment & ALIGN_CENTER )
			left -= width/2;
		if( alignment & ALIGN_BOTTOM )
			top -= currentSize;
		else if( alignment & ALIGN_MIDDLE )
			top -= currentSize/2;
		font->DrawText( left, top, 0, text, c, currentSize, style );
	}
	this->UpdateEffects();
}

u32 GuiText::GetMaxChars()
{
	u32 ret = 0;
	u32 w = 0;
	if( !text )
		return ret;
	if( maxWidth <= 0 )
		return 0xffffffff;

	wchar_t *tmp = text;
	while( *tmp )
	{
		u32 size = this->font->TextWidth( *tmp, currentSize );
		if( w + size <= (u32)maxWidth )
			w += size;
		else
			break;
		tmp++;
		ret++;
	}
	return ret;
}

wchar_t* charToWideChar(const char* strChar)
{
	wchar_t *strWChar = NULL;
	strWChar = new (std::nothrow) wchar_t[strlen(strChar) + 1];
	if( !strWChar )
		return NULL;

	char *tempSrc = (char *)strChar;
	wchar_t *tempDest = strWChar;
	while((*tempDest++ = *tempSrc++));

	return strWChar;
}
