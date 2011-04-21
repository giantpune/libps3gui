/****************************************************************************
 * giantpune 2011
 ***************************************************************************/


#include <stdio.h>
#include <string>
#include <iostream>
#include <malloc.h>

#include "fileops.h"
#include "guifont.h"
#include "rsxmem.h"
#include "utils.h"
#include "wstring.h"


using namespace std;

GuiFont::GuiFont( u8* mem, u32 len, u16 size )
{
	//init variables
	storedSize = 0;
	cached = 0;
	flashFontMem = NULL;

	//load font with freetype
	FT_Init_FreeType( &freetype );
	if( FT_New_Memory_Face( freetype, (const FT_Byte*)mem, len, 0, &face ) )
	{
		printf( "GuiFont::GuiFont(): FT_New_Memory_Face() failed.  be prepared to crash\n");
		return;
	}
	FT_Set_Pixel_Sizes( face, size, size );
	slot = face->glyph;
	storedSize = size;
}

GuiFont::GuiFont( PS3Font ps3Font, u16 size )
{
	//init variables
	storedSize = 0;
	cached = 0;

	std::string path = "/dev_flash/data/font/";
	//read selected font from flash
	switch( ps3Font )
	{
	case SCE_PS3_DH_R_CGB:		path += "SCE-PS3-DH-R-CGB.TTF"; break;
	case SCE_PS3_NR_R_EXT:		path += "SCE-PS3-NR-R-EXT.TTF"; break;
	case SCE_PS3_SR_R_EXT:		path += "SCE-PS3-SR-R-EXT.TTF"; break;
	case SCE_PS3_NR_B_JPN:		path += "SCE-PS3-NR-B-JPN.TTF"; break;
	case SCE_PS3_NR_L_JPN:		path += "SCE-PS3-NR-L-JPN.TTF"; break;
	case SCE_PS3_NR_R_JPN:		path += "SCE-PS3-NR-R-JPN.TTF"; break;
	case SCE_PS3_SR_R_JPN:		path += "SCE-PS3-SR-R-JPN.TTF"; break;
	case SCE_PS3_CP_R_KANA:		path += "SCE-PS3-CP-R-KANA.TTF"; break;
	case SCE_PS3_YG_B_KOR:		path += "SCE-PS3-YG-B-KOR.TTF"; break;
	case SCE_PS3_YG_L_KOR:		path += "SCE-PS3-YG-L-KOR.TTF"; break;
	case SCE_PS3_YG_R_KOR:		path += "SCE-PS3-YG-R-KOR.TTF"; break;
	case SCE_PS3_MT_B_LATIN:	path += "SCE-PS3-MT-B-LATIN.TTF"; break;
	case SCE_PS3_MT_BI_LATIN:	path += "SCE-PS3-MT-BI-LATIN.TTF"; break;
	case SCE_PS3_MT_I_LATIN:	path += "SCE-PS3-MT-I-LATIN.TTF"; break;
	case SCE_PS3_MT_R_LATIN:	path += "SCE-PS3-MT-R-LATIN.TTF"; break;
	case SCE_PS3_RD_B_LATIN:	path += "SCE-PS3-RD-B-LATIN.TTF"; break;
	case SCE_PS3_RD_B_LATIN2:	path += "SCE-PS3-RD-B-LATIN2.TTF"; break;
	case SCE_PS3_RD_BI_LATIN:	path += "SCE-PS3-RD-BI-LATIN.TTF"; break;
	case SCE_PS3_RD_I_LATIN:	path += "SCE-PS3-RD-I-LATIN.TTF"; break;
	case SCE_PS3_RD_L_LATIN:	path += "SCE-PS3-RD-L-LATIN.TTF"; break;
	case SCE_PS3_RD_L_LATIN2:	path += "SCE-PS3-RD-L-LATIN2.TTF"; break;
	case SCE_PS3_RD_LI_LATIN:	path += "SCE-PS3-RD-LI-LATIN.TTF"; break;
	case SCE_PS3_RD_R_LATIN:	path += "SCE-PS3-RD-R-LATIN.TTF"; break;
	case SCE_PS3_RD_R_LATIN2:	path += "SCE-PS3-RD-R-LATIN2.TTF"; break;
	case SCE_PS3_SR_R_LATIN:	path += "SCE-PS3-SR-R-LATIN.TTF"; break;
	case SCE_PS3_SR_R_LATIN2:	path += "SCE-PS3-SR-R-LATIN2.TTF"; break;
	case SCE_PS3_VR_R_LATIN:	path += "SCE-PS3-VR-R-LATIN.TTF"; break;
	case SCE_PS3_VR_R_LATIN2:	path += "SCE-PS3-VR-R-LATIN2.TTF"; break;

	default:
		printf("GuiFont() unrecognized case.  no font loaded\n" );
		return;
		break;
	}

	//read font from flash
	u32 len;
	flashFontMem = FileOps::ReadFile( path, &len );
	if( !flashFontMem )
		return;

	//load font with freetype
	FT_Init_FreeType( &freetype );
	if( FT_New_Memory_Face( freetype, (const FT_Byte*)flashFontMem, len, 0, &face ) )
	{
		printf( "GuiFont::GuiFont(): FT_New_Memory_Face() failed.  be prepared to crash\n");
		free( flashFontMem );
		flashFontMem = NULL;
		return;
	}
	FT_Set_Pixel_Sizes( face, size, size );
	slot = face->glyph;
	storedSize = size;

}

GuiFont::~GuiFont()
{
	Clear();
	FT_Done_FreeType( freetype );
	if( flashFontMem )
		free( flashFontMem );
}

void GuiFont::Clear()
{
	if( letters.size() )
	{
		for( std::map<wchar_t, Letter>::iterator i = letters.begin(), iEnd = letters.end(); i != iEnd; ++i )
		{
			if( (*i).second.rsxMem )
				RsxMem::Free( (*i).second.rsxMem );
			//printf("GuiFont::Clear() -> destroying: %c\n", (*i).first );
		}
		letters.clear();
	}
	cached = 0;
}

Letter *GuiFont::CacheLetter( wchar_t ch )
{
	//printf("CacheLetter( %04x  %c )\n", ch, ch );
	static u8 letter_bitmap[ 257 * 256 ];

	u8 i;
	Letter newLetter;
	newLetter.fw = 0;
	newLetter.fy = 0;
	newLetter.rsxOffset = 0;
	newLetter.rsxMem = NULL;
	int ww = 0;
	u8* texture;
	u8* font;

	//load the glyph
	FT_UInt gIndex;
	gIndex = FT_Get_Char_Index( face, ch );
	if( FT_Load_Glyph( face, gIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER ) )
	{
		printf("FT_Load_Glyph() failed  %04x\n", ch );
		goto done;
	}

	//convert to bitmap
	newLetter.rsxMem = RsxMem::Alloc( storedSize * storedSize << 1, &newLetter.rsxOffset );
	if( !newLetter.rsxMem )
		goto done;
	newLetter.fw = storedSize;
	newLetter.fy = storedSize - 1 - slot->bitmap_top;

	texture = newLetter.rsxMem;
	font = (u8*)&letter_bitmap;

	memset( font, 0, storedSize * storedSize );
	for( int n = 0; n < slot->bitmap.rows; n++ )
	{
		for ( int m = 0; m < slot->bitmap.width; m++ )
		{
			if( m >= storedSize || n >= storedSize )
				continue;

			font[ m ] = (u8)slot->bitmap.buffer[ ww + m ];
		}
		font += storedSize;
		ww += slot->bitmap.width;
	}

	newLetter.fw = ( ( slot->advance.x + 31 ) >> 6 ) + ( ( slot->bitmap_left < 0 ) ? -slot->bitmap_left : 0 );

	font = (u8*)&letter_bitmap;
	for( u32 a = 0; a < storedSize; a++ )
	{
		for( u32 b = 0; b < storedSize; b++ )
		{
			i = font[ b ];

			i >>= ( b & ( 7 / 8 ) ) * 8;

			i = ( i & ( ( 1 << 8 ) - 1 ) ) * 255 / ( ( 1 << 8 ) - 1 );

			if( i )
			{	//TINY3D_TEX_FORMAT_A4R4G4B4
				i>>=4;
				*((u16 *)texture) = ( i<<12 ) | 0xfff;
			}
			else
			{
				texture[ 0 ] = texture[ 1 ] = 0x0; //texture[2] = 0x0;
				//texture[3] = 0x0; // alpha
			}
			texture += 2;
		}
		font += storedSize;
	}
	if( cached++ >= FONT_MAX_CHARS )
		Clear();

done:
	letters[ ch ] = newLetter; //insert the letter in the list even if it wasnt loaded so we wont try to load it again
	return &letters[ ch ];
}

Letter *GuiFont::Get( wchar_t ch )
{
	if( letters.find( ch ) != letters.end() )
	{
		return &letters[ ch ];
	}
	return CacheLetter( ch );
}

void GuiFont::DrawText( float x, float y, float z, wchar_t *text, u32 rgba, u32 size, uint16_t textStyle )
{
	if( !text )
		return;
	int len = wcslen( text );
	DrawText( x, y, z, text, len, rgba, size, textStyle );
}

void GuiFont::DrawText( float x, float y, float z, wchar_t *text, u32 len, u32 rgba, u32 size, uint16_t textStyle )
{
	if( !text )
		return;

	for( u32 i = 0; i < len; i++ )
	{
		Letter *letter  = Get( text[ i ] );
		if( !letter )
			continue;
		DrawCh( letter, x, y, z, rgba, size );
		x += (float) ( ( letter->fw * size ) / storedSize );
	}
}

void GuiFont::DrawCh( float x, float y, float z, u16 chr, u32 rgba, u32 size )
{
	Letter *letter = Get( chr );
	if( !letter || !letter->rsxMem )
		return;

	DrawCh( letter, x, y, z, rgba, size );
}

void GuiFont::DrawCh( Letter *letter, float x, float y, float z, u32 rgba, u32 size )
{
	if( !letter || !letter->rsxMem )
		return;

	y += (float) ( ( letter->fy * size ) / storedSize );

	float dx  = x + size,
	dy = y + size;

	// Load sprite texture
	tiny3d_SetTexture( 0, letter->rsxOffset, storedSize, storedSize, storedSize * 2, TINY3D_TEX_FORMAT_A4R4G4B4, 1 );

	tiny3d_SetPolygon( TINY3D_QUADS );

	tiny3d_VertexPos( x, y, z );
	tiny3d_VertexColor( rgba );
	tiny3d_VertexTexture( 0.0f, 0.0f );

	tiny3d_VertexPos( dx, y, z );
	tiny3d_VertexTexture( 0.95f, 0.0f );

	tiny3d_VertexPos( dx, dy, z );
	tiny3d_VertexTexture( 0.95f, 0.95f );

	tiny3d_VertexPos( x, dy, z );
	tiny3d_VertexTexture( 0.0f, 0.95f );

	tiny3d_End();
}

u32 GuiFont::TextWidth( wchar_t *text, u32 size )
{
	if( !text )
		return 0;
	int len = wcslen( text );
	return TextWidth( text, len, size );
}

u32 GuiFont::TextWidth( wchar_t *text, u32 len, u32 size )
{
	if( !text )
		return 0;
	u32 ret = 0;
	for( u32 i = 0; i < len; i++ )
	{
		Letter *letter  = Get( text[ i ] );
		if( !letter )
			continue;
		ret += (float) ( ( letter->fw * size ) / storedSize );
	}
	return ret;
}

u32 GuiFont::TextWidth( wchar_t ch, u32 size )
{
	Letter *letter  = Get( ch );
	if( !letter )
		return 0;
	return (float) ( ( letter->fw * size ) / storedSize );
}

void GuiFont::CacheRange( wchar_t first, wchar_t last )
{
	for( wchar_t c = first; c <= last; c++ )
		CacheLetter( c );
}

#if 0
void GuiFont::CacheAll()
{
	FT_UInt gIndex;
	FT_ULong charCode = FT_Get_First_Char( face, &gIndex );
	while( gIndex != 0 )
	{
		CacheLetter( charCode );
		charCode = FT_Get_Next_Char( face, charCode, &gIndex );
	}
	printf("cached: %u\n", cached);
}
#endif
GuiDualFont::GuiDualFont( PS3Font ps3Font1, PS3Font ps3Font2, u16 size, u16 splitPosition ):
		GuiFont( ps3Font1, size ), upperRange( splitPosition )
{
	//the parent constructor is called for the lower range
	//now load the font for the upper range

	std::string path = "/dev_flash/data/font/";
	//read selected font from flash
	switch( ps3Font2 )
	{
	case SCE_PS3_DH_R_CGB:		path += "SCE-PS3-DH-R-CGB.TTF"; break;
	case SCE_PS3_NR_R_EXT:		path += "SCE-PS3-NR-R-EXT.TTF"; break;
	case SCE_PS3_SR_R_EXT:		path += "SCE-PS3-SR-R-EXT.TTF"; break;
	case SCE_PS3_NR_B_JPN:		path += "SCE-PS3-NR-B-JPN.TTF"; break;
	case SCE_PS3_NR_L_JPN:		path += "SCE-PS3-NR-L-JPN.TTF"; break;
	case SCE_PS3_NR_R_JPN:		path += "SCE-PS3-NR-R-JPN.TTF"; break;
	case SCE_PS3_SR_R_JPN:		path += "SCE-PS3-SR-R-JPN.TTF"; break;
	case SCE_PS3_CP_R_KANA:		path += "SCE-PS3-CP-R-KANA.TTF"; break;
	case SCE_PS3_YG_B_KOR:		path += "SCE-PS3-YG-B-KOR.TTF"; break;
	case SCE_PS3_YG_L_KOR:		path += "SCE-PS3-YG-L-KOR.TTF"; break;
	case SCE_PS3_YG_R_KOR:		path += "SCE-PS3-YG-R-KOR.TTF"; break;
	case SCE_PS3_MT_B_LATIN:	path += "SCE-PS3-MT-B-LATIN.TTF"; break;
	case SCE_PS3_MT_BI_LATIN:	path += "SCE-PS3-MT-BI-LATIN.TTF"; break;
	case SCE_PS3_MT_I_LATIN:	path += "SCE-PS3-MT-I-LATIN.TTF"; break;
	case SCE_PS3_MT_R_LATIN:	path += "SCE-PS3-MT-R-LATIN.TTF"; break;
	case SCE_PS3_RD_B_LATIN:	path += "SCE-PS3-RD-B-LATIN.TTF"; break;
	case SCE_PS3_RD_B_LATIN2:	path += "SCE-PS3-RD-B-LATIN2.TTF"; break;
	case SCE_PS3_RD_BI_LATIN:	path += "SCE-PS3-RD-BI-LATIN.TTF"; break;
	case SCE_PS3_RD_I_LATIN:	path += "SCE-PS3-RD-I-LATIN.TTF"; break;
	case SCE_PS3_RD_L_LATIN:	path += "SCE-PS3-RD-L-LATIN.TTF"; break;
	case SCE_PS3_RD_L_LATIN2:	path += "SCE-PS3-RD-L-LATIN2.TTF"; break;
	case SCE_PS3_RD_LI_LATIN:	path += "SCE-PS3-RD-LI-LATIN.TTF"; break;
	case SCE_PS3_RD_R_LATIN:	path += "SCE-PS3-RD-R-LATIN.TTF"; break;
	case SCE_PS3_RD_R_LATIN2:	path += "SCE-PS3-RD-R-LATIN2.TTF"; break;
	case SCE_PS3_SR_R_LATIN:	path += "SCE-PS3-SR-R-LATIN.TTF"; break;
	case SCE_PS3_SR_R_LATIN2:	path += "SCE-PS3-SR-R-LATIN2.TTF"; break;
	case SCE_PS3_VR_R_LATIN:	path += "SCE-PS3-VR-R-LATIN.TTF"; break;
	case SCE_PS3_VR_R_LATIN2:	path += "SCE-PS3-VR-R-LATIN2.TTF"; break;
	default:
		printf("GuiDualFont() unrecognized case.  no font loaded\n" );
		return;
		break;
	}

	//read font from flash
	u32 len;
	flashFontMem2 = FileOps::ReadFile( path, &len );
	if( !flashFontMem )
		return;

	//load font with freetype
	FT_Init_FreeType( &freetype2 );
	if( FT_New_Memory_Face( freetype2, (const FT_Byte*)flashFontMem2, len, 0, &face2 ) )
	{
		printf( "GuiFont::GuiFont(): FT_New_Memory_Face() failed.  be prepared to crash\n");
		free( flashFontMem2 );
		flashFontMem2 = NULL;
		return;
	}
	FT_Set_Pixel_Sizes( face2, size, size );
	slot2 = face2->glyph;
	storedSize = size;

}

GuiDualFont::~GuiDualFont()
{
	FT_Done_FreeType( freetype2 );
	if( flashFontMem2 )
		free( flashFontMem2 );
}

Letter *GuiDualFont::CacheLetter( wchar_t ch )
{
	if( ch >= upperRange )//load this puppy from the second font
		return CacheLetterFromFont2( ch );

	return GuiFont::CacheLetter( ch );
}

Letter *GuiDualFont::CacheLetterFromFont2( wchar_t ch )
{
	static u8 letter_bitmap[ 257 * 256 ];

	u8 i;
	Letter newLetter;
	newLetter.fw = 0;
	newLetter.fy = 0;
	newLetter.rsxOffset = 0;
	newLetter.rsxMem = NULL;
	int ww = 0;
	u8* texture;
	u8* font;

	//load the glyph
	FT_UInt gIndex;
	gIndex = FT_Get_Char_Index( face2, ch );
	if( FT_Load_Glyph( face2, gIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER ) )
	{
		printf("FT_Load_Glyph() failed  %04x\n", ch );
		goto done;
	}

	//convert to bitmap
	newLetter.rsxMem = RsxMem::Alloc( storedSize * storedSize << 1, &newLetter.rsxOffset );
	if( !newLetter.rsxMem )
		goto done;
	newLetter.fw = storedSize;
	newLetter.fy = storedSize - 1 - slot2->bitmap_top;

	texture = newLetter.rsxMem;
	font = (u8*)&letter_bitmap;

	memset( font, 0, storedSize * storedSize );
	for( int n = 0; n < slot2->bitmap.rows; n++ )
	{
		for ( int m = 0; m < slot2->bitmap.width; m++ )
		{
			if( m >= storedSize || n >= storedSize )
				continue;

			font[ m ] = (u8)slot2->bitmap.buffer[ ww + m ];
		}
		font += storedSize;
		ww += slot2->bitmap.width;
	}

	newLetter.fw = ( ( slot2->advance.x + 31 ) >> 6 ) + ( ( slot2->bitmap_left < 0 ) ? -slot2->bitmap_left : 0 );

	font = (u8*)&letter_bitmap;
	for( u32 a = 0; a < storedSize; a++ )
	{
		for( u32 b = 0; b < storedSize; b++ )
		{
			i = font[ b ];

			i >>= ( b & ( 7 / 8 ) ) * 8;

			i = ( i & ( ( 1 << 8 ) - 1 ) ) * 255 / ( ( 1 << 8 ) - 1 );

			if( i )
			{	//TINY3D_TEX_FORMAT_A4R4G4B4
				i>>=4;
				*((u16 *)texture) = ( i<<12 ) | 0xfff;
			}
			else
			{
				texture[ 0 ] = texture[ 1 ] = 0x0; //texture[2] = 0x0;
				//texture[3] = 0x0; // alpha
			}
			texture += 2;
		}
		font += storedSize;
	}
	if( cached++ >= FONT_MAX_CHARS )
		Clear();

done:
	letters[ ch ] = newLetter; //insert the letter in the list even if it wasnt loaded so we wont try to load it again
	return &letters[ ch ];
}
