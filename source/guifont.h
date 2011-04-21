#ifndef GUIFONT_H
#define GUIFONT_H

#include <tiny3d.h>
#include <ft2build.h>
#include <freetype2/freetype/freetype.h>
#include <freetype2/freetype/ftglyph.h>
#include <map>
#include <wchar.h>

// the max abount of letters to store in rsx memory at 1 time
// once this number is hit, all cached letters are cleared and the caching starts again
// this is to keep a font from eating up all the rsx memory
#define FONT_MAX_CHARS 0x200

//not sure what a good preset size to load is, so set it here
#define FONT_LOAD_SIZE 32		//this makes each letter take up 2KiB in rsx memory


//these are built_in fonts i see in 3.41.
/*
 SCE_PS3_<font name>_<typeface>_<lang>

 names:
 CP: Capie
 DH: DFHeiW5
 MT: Maltisse				(serif)
 NR: NewRodin
 RD: Rodin					(sans-serif, square tips on letters)
 SR: Seurat					(sans-serif, round tips on letters)
 VR: VAGRundschriftDLig		(sans-serif, square tips, smaller than the other latin fonts)
 YG: YD gd

 face:
 B: bold
 I: italic
 L: light
 R: regular
*/
typedef enum PS3Font
{							//number of character stored in each font
	SCE_PS3_CP_R_KANA,		//221
	SCE_PS3_DH_R_CGB,		//30724
	SCE_PS3_MT_BI_LATIN,	//324
	SCE_PS3_MT_B_LATIN,		//324
	SCE_PS3_MT_I_LATIN,		//324
	SCE_PS3_MT_R_LATIN,		//324
	SCE_PS3_NR_B_JPN,		//9820
	SCE_PS3_NR_L_JPN,		//9820
	SCE_PS3_NR_R_EXT,		//767
	SCE_PS3_NR_R_JPN,		//9820
	SCE_PS3_RD_BI_LATIN,	//324
	SCE_PS3_RD_B_LATIN2,	//461
	SCE_PS3_RD_B_LATIN,		//324
	SCE_PS3_RD_I_LATIN,		//324
	SCE_PS3_RD_LI_LATIN,	//324
	SCE_PS3_RD_L_LATIN2,	//461
	SCE_PS3_RD_L_LATIN,		//324
	SCE_PS3_RD_R_LATIN2,	//461
	SCE_PS3_RD_R_LATIN,		//324
	SCE_PS3_SR_R_EXT,		//767
	SCE_PS3_SR_R_JPN,		//9820
	SCE_PS3_SR_R_LATIN2,	//461
	SCE_PS3_SR_R_LATIN,		//324
	SCE_PS3_VR_R_LATIN2,	//521
	SCE_PS3_VR_R_LATIN,		//248
	SCE_PS3_YG_B_KOR,		//12607
	SCE_PS3_YG_L_KOR,		//12612
	SCE_PS3_YG_R_KOR,		//12614
	PS3_FLASH_FONT_MAX		// not a font, but the number of available fonts
} PS3Font;

//struct to hold info about a letter needed to draw it
struct Letter
{
	short fw;
	short fy;
	u8* rsxMem;
	u32 rsxOffset;
};

class GuiFont
{
public:
	//create a font from a .ttf stored in memory
	//! mem is a pointer to the font in memory
	//! len is the length of the font buffer
	//! size is the size ( width & height) for each bitmap
	//! ( (size * size * 2 ) bytes are stored in rsx memory for each latter that is cached ).
	//! you should use a power of 2 from 8 and 256
	GuiFont( u8* mem, u32 len, u16 size = FONT_LOAD_SIZE );

	//use one of the fonts that comes already on the ps3
	GuiFont( PS3Font ps3Font, u16 size = FONT_LOAD_SIZE );

	~GuiFont();

	//draw some text
	//! x, y, and z are coords
	//! text is the text to draw
	//! rgba is the color to use
	//! size is the size on the screen the text should appear.  this is scaled from the size the font is loaded with
	//! textStyle is not used
	void DrawText( float x, float y, float z, wchar_t *text, u32 rgba, u32 size, uint16_t textStyle );

	//same as above except it accepts "len" if you already know it and avoids checking strlen
	void DrawText( float x, float y, float z, wchar_t *text, u32 len, u32 rgba, u32 size, uint16_t textStyle );

	//draw a letter on the screen
	//! parameters are the same as above
	void DrawCh( float x, float y, float z, u16 chr, u32 rgba, u32 size );

	//get the width of some text if drawn with a certain size
	u32 TextWidth( wchar_t *text, u32 size );

	//same as above, but accepts a length if you already know it
	u32 TextWidth( wchar_t *text, u32 len, u32 size );

	u32 TextWidth( wchar_t ch, u32 size );

	//cleare all cached letters from RSX memory
	void Clear();

	//just for testing
	//void CacheAll();

	//cache a range of letters to try to avoid fragmenting the rsx memory with 2KiB all over the place
	void CacheRange( wchar_t first, wchar_t last );

//private:
protected:

	//list of all loaded letters
	std::map<wchar_t, Letter> letters;

	//load a new letter into the above list and return a reference to it
	virtual Letter *CacheLetter( wchar_t ch );

	//get a reference to a letter in the list
	//! if it is not already in the list, it will be loaded and inserted
	Letter *Get( wchar_t ch );

	u32 GetTextureSize( u32 numchars, u16 size = FONT_LOAD_SIZE );
	void CreateTexture( u32 size );
	void TTF_to_Bitmap( u16 chr, u8 * bitmap, short *w, short *h, short *y_correction );

	FT_Face face;
	FT_Library freetype;
	FT_GlyphSlot slot;

	//draws a letter
	void DrawCh( Letter *ch, float x, float y, float z, u32 rgba, u32 size );
	u16 storedSize;

	u32 cached;

	//buffer used to hold a font if one is read from the PS3 flash
	u8* flashFontMem;
};

//subclass of the GuiFont to allow using 2 different fonts for different ranges of letters
//! ie ascii will be drawn with 1 font and jap will be drawn with another one
class GuiDualFont: public GuiFont
{
public:
	GuiDualFont( PS3Font ps3Font1 = SCE_PS3_RD_R_LATIN2,  	//font to use when drawing the lower range
				 PS3Font ps3Font2 = SCE_PS3_NR_R_JPN, 		//font to use when drawing the upper range
				u16 size = FONT_LOAD_SIZE,					//load size
				u16 splitPosition = 0x2e00 );				//start of the upper range.  i have no idea if this is a good default or not

	~GuiDualFont();
private:
	u16 upperRange;
	FT_Face face2;
	FT_Library freetype2;
	FT_GlyphSlot slot2;

	//buffer used to hold a font if one is read from the PS3 flash
	u8* flashFontMem2;
	Letter *CacheLetter( wchar_t ch );
	Letter *CacheLetterFromFont2( wchar_t ch );
};


#endif // GUIFONT_H
