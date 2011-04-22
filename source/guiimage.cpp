/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 * giantpune 2011
 *
 * gui_image.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <pngdec/pngdec.h>
#include <jpgdec/jpgdec.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "rsxmem.h"
#include "utils.h"

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData( const u8 * img, const u32 len, u32 format )
{
	data = NULL;
	width = 0;
	height = 0;
	rsxOffset = 0xffffffff;
	wpitch = 0;

	if( !img || len < 4 )
		return;

	//load png
	if( *(u32*)img == 0x89504e47 )//png magic word
	{
		LoadPng( img, len, format );
	}
	//load jpeg
	else if( img[ 0 ] == 0xff && img[ 1 ] == 0xd8 && img[ len - 2 ] == 0xff && img[ len - 1 ] == 0xd9 )
	{
		LoadJpeg( img, len, format );
	}
	else
	{
		printf("GuiImageData::GuiImageData(): unrecognized format\n");
	}
}

GuiImageData::GuiImageData( const Resource &resource, u32 format )
{
	data = NULL;
	width = 0;
	height = 0;
	rsxOffset = 0xffffffff;
	wpitch = 0;

	const u8* img = resource.Data();
	u32 len = resource.Size();

	if( !img || len < 4 )
		return;

	//load png
	if( *(u32*)img == 0x89504e47 )//png magic word
	{
		LoadPng( img, len, format );
	}
	//load jpeg
	else if( img[ 0 ] == 0xff && img[ 1 ] == 0xd8 && img[ len - 2 ] == 0xff && img[ len - 1 ] == 0xd9 )
	{
		LoadJpeg( img, len, format );
	}
	else
	{
		printf("GuiImageData::GuiImageData(): unrecognized format\n");
	}
}

GuiImageData::~GuiImageData()
{
	if( data )
	{
		RsxMem::Free( data );
		data = NULL;
	}
}

void GuiImageData::LoadJpeg( const u8* mem, u32 len, u32 fmt )
{
	jpgData jd;

	//jd.jpg_size = len;
	//jd.jpg_in   = (void *)mem;

	// load jpg from memory
	int pp = jpgLoadFromBuffer( (const void*)mem, len, &jd);
	if( pp )
	{
		printf("jpgLoadFromBuffer(): %i\n", pp );
		return;
	}

	if( !jd.bmp_out )
		return;

	//convert to 16bit color
	if( fmt == TINY3D_TEX_FORMAT_R5G6B5 )
	{
		u8* bit16 = (u8*)malloc( jd.height * jd.width * 2 );
		if( !bit16 )
		{
			printf("GuiImageData(): out of memory\n");
			free( jd.bmp_out );
			return;
		}

		u32 *px = (u32*)jd.bmp_out;
		u16* npx = (u16*)bit16;
		for( u32 h = 0; h < jd.height; h++ )
		{
			for( u32 w = 0; w < jd.width; w++ )
			{
				u8 r = ((*px) & 0xff0000 ) >> 16;
				u8 g = ((*px) & 0xff00 ) >> 8;
				u8 b = ((*px) & 0xff ) >> 0;
				*npx = ( ( r/( 255/0x1f ) ) << 11) | (( g/( 255/0x3f ) ) << 5 ) | ( ( b/( 255/0x1f ) ) << 0 );
				npx++;
				px++;
			}
		}

		//adjust png data
		jd.pitch /= 2;
		free( jd.bmp_out );
		jd.bmp_out = bit16;

	}

	//allocate memory in rsx buffer
	u32 texSize = jd.pitch * jd.height;
	data = RsxMem::Alloc( texSize, &rsxOffset );
	if( !data )
	{
		rsxOffset = 0xffffffff;
		free( jd.bmp_out );
		return;
	}
	//printf("GuiImageData(): data: %p, offset: %08x  size: %08x\n", data, rsxOffset, texSize );

	//copy bmp to rsx memory
	memcpy( data, jd.bmp_out, texSize );

	//hexdump( pd.bmp_out, 0x20 );
	free( jd.bmp_out );

	width = jd.width;
	height = jd.height;
	wpitch = jd.pitch;
	this->format = fmt;
}

void GuiImageData::LoadPng( const u8* mem, u32 len, u32 fmt )
{
	//printf("GuiImageData() loading as PNG\n");
	pngData pd;

	//pd.png_size = len;
	//pd.png_in   = (void *)mem;

	// load PNG from memory
	int pp = pngLoadFromBuffer( (const void*)mem, len, &pd);
	if( pp )
	{
		printf("pngLoadFromBuffer(): %i\n", pp );
		return;
	}
	if( !pd.bmp_out )
		return;

	//convert to 16bit color
	if( fmt == TINY3D_TEX_FORMAT_R5G6B5 )
	{
		u8* bit16 = (u8*)malloc( pd.height * pd.width * 2 );
		if( !bit16 )
		{
			printf("GuiImageData(): out of memory\n");
			free( pd.bmp_out );
			return;
		}

		u32 *px = (u32*)pd.bmp_out;
		u16* npx = (u16*)bit16;
		for( u32 h = 0; h < pd.height; h++ )
		{
			for( u32 w = 0; w < pd.width; w++ )
			{
				u8 r = ((*px) & 0xff0000 ) >> 16;
				u8 g = ((*px) & 0xff00 ) >> 8;
				u8 b = ((*px) & 0xff ) >> 0;
				*npx = ( ( r/( 255/0x1f ) ) << 11) | (( g/( 255/0x3f ) ) << 5 ) | ( ( b/( 255/0x1f ) ) << 0 );
				npx++;
				px++;
			}
		}

		//adjust png data
		pd.pitch /= 2;
		free( pd.bmp_out );
		pd.bmp_out = bit16;

	}

	//allocate memory in rsx buffer
	u32 texSize = pd.pitch * pd.height;
	data = RsxMem::Alloc( texSize, &rsxOffset );
	if( !data )
	{
		rsxOffset = 0xffffffff;
		free( pd.bmp_out );
		return;
	}
	//printf("GuiImageData(): data: %p, offset: %08x  size: %08x\n", data, rsxOffset, texSize );

	//copy bmp to rsx memory
	memcpy( data, pd.bmp_out, texSize );

	//hexdump( pd.bmp_out, 0x20 );
	free( pd.bmp_out );

	width = pd.width;
	height = pd.height;
	wpitch = pd.pitch;
	this->format = fmt;
}

u8 * GuiImageData::GetImage()
{
	return data;
}

int GuiImageData::GetWidth()
{
	return width;
}

int GuiImageData::GetHeight()
{
	return height;
}

u32 GuiImageData::GetRsxTexOffset()
{
	return rsxOffset;
}

u32 GuiImageData::GetWPitch()
{
	return wpitch;
}
/*
void GuiImageData::Resize( int w, int h )
{
	if( !w || !h || !width || !height || !data
		|| ( w < 1 && h < 1 ) )
		return;

	if( w == -1 )
		w *= height/h;
	else if( h == -1 )
		h *= width/w;

	switch( format )
	{
	case TINY3D_TEX_FORMAT_A8R8G8B8://32bit
		{



		}
		break;
	case TINY3D_TEX_FORMAT_R5G6B5://16bit
		{
			u32 newSize = w * h * 4;
			u8* newData = (u8*)malloc( newSize );
			if( !newData )
			{
				printf( "GuiImageData::Resize() out of memory\n");
				return;
			}
			u16 *newPx = (u16*)newData;
			u16* oldPx = (u16*)data;
			for( int hh = 0; hh < h; hh++ )
			{
				int hhh = (height/h) * hh;
				for( int ww = 0; ww < w; ww++ )
				{
					int www = (width/w) * ww;





				}

			}


		}
		break;
	default:
		printf( "GuiImageData::Resize() unsupported format  %08x\n", format );
		break;
	}


}
*/

/**
 * Constructor for the GuiImage class.
 */
GuiImage::GuiImage()
{
	image = NULL;
	width = 0;
	height = 0;
	imageangle = 0;
	tile = -1;
	stripe = 0;
	imgType = IMAGE_DATA;
	rsxOffset = 0xffffffff;
	wpitch = 0;
	format = 0;
}

GuiImage::GuiImage( GuiImageData * img )
{
	image = NULL;
	width = 0;
	height = 0;
	rsxOffset = 0xffffffff;
	wpitch = 0;
	format = 0;
	if(img)
	{
		image = img->GetImage();
		width = img->GetWidth();
		height = img->GetHeight();
		rsxOffset = img->GetRsxTexOffset();
		wpitch = img->GetWPitch();
		format = img->Format();
	}
	imageangle = 0;
	tile = -1;
	stripe = 0;
	imgType = IMAGE_DATA;

}

GuiImage::GuiImage( u8 * img, int w, int h )
{
	image = img;
	width = w;
	height = h;
	imageangle = 0;
	tile = -1;
	stripe = 0;
	wpitch = 0;
	format = TINY3D_TEX_FORMAT_A8R8G8B8;
	rsxOffset = RsxMem::Offset( img );
	imgType = IMAGE_TEXTURE;
}

GuiImage::GuiImage(int w, int h, u32 c)
{
    width = w;
    height = h;
    imageangle = 0;
    tile = -1;
    stripe = 0;
    imgType = IMAGE_COLOR;
    image = NULL;
    color = c;
	format = TINY3D_TEX_FORMAT_A8R8G8B8;
}

/**
 * Destructor for the GuiImage class.
 */
GuiImage::~GuiImage()
{
    //if( imgType == IMAGE_COLOR && image )
    //	RsxMem::Free( image );
}

u8 * GuiImage::GetImage()
{
	return image;
}

void GuiImage::SetImage( GuiImageData * img )
{
	image = NULL;
	width = 0;
	height = 0;
	rsxOffset = 0xffffffff;
	wpitch = 0;
	format = 0;
	if(img)
	{
		image = img->GetImage();
		width = img->GetWidth();
		height = img->GetHeight();
		rsxOffset = img->GetRsxTexOffset();
		wpitch = img->GetWPitch();
		format = img->Format();
	}
	imgType = IMAGE_DATA;
}

void GuiImage::SetImage(u8 * img, int w, int h)
{
	image = img;
	width = w;
	height = h;
	imgType = IMAGE_TEXTURE;
	rsxOffset = RsxMem::Offset( img );
	format = TINY3D_TEX_FORMAT_A8R8G8B8;
}

void GuiImage::SetAngle(float a)
{
	imageangle = a;
}

void GuiImage::SetTile(int t)
{
	tile = t;
}
/*
GXColor GuiImage::GetPixel(int x, int y)
{
	if(!image || this->GetWidth() <= 0 || x < 0 || y < 0)
		return (GXColor){0, 0, 0, 0};

	u32 offset = (((y >> 2)<<4)*this->GetWidth()) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
	GXColor color;
	color.a = *(image+offset);
	color.r = *(image+offset+1);
	color.g = *(image+offset+32);
	color.b = *(image+offset+33);
	return color;
}

void GuiImage::SetPixel(int x, int y, GXColor color)
{
	if(!image || this->GetWidth() <= 0 || x < 0 || y < 0)
		return;

	u32 offset = (((y >> 2)<<4)*this->GetWidth()) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
	*(image+offset) = color.a;
	*(image+offset+1) = color.r;
	*(image+offset+32) = color.g;
	*(image+offset+33) = color.b;
}

void GuiImage::SetStripe(int s)
{
	stripe = s;
}

void GuiImage::ColorStripe(int shift)
{
	int x, y;
	GXColor color;
	int alt = 0;

	for(y=0; y < this->GetHeight(); y++)
	{
		if(y % 3 == 0)
			alt ^= 1;

		for(x=0; x < this->GetWidth(); x++)
		{
			color = GetPixel(x, y);

			if(alt)
			{
				if(color.r < 255-shift)
					color.r += shift;
				else
					color.r = 255;
				if(color.g < 255-shift)
					color.g += shift;
				else
					color.g = 255;
				if(color.b < 255-shift)
					color.b += shift;
				else
					color.b = 255;

				color.a = 255;
			}
			else
			{
				if(color.r > shift)
					color.r -= shift;
				else
					color.r = 0;
				if(color.g > shift)
					color.g -= shift;
				else
					color.g = 0;
				if(color.b > shift)
					color.b -= shift;
				else
					color.b = 0;

				color.a = 255;
			}
			SetPixel(x, y, color);
		}
	}
	int len = width*height*4;
	if(len%32) len += (32-len%32);
	DCFlushRange(image, len);
}

void GuiImage::Grayscale()
{
	GXColor color;
	u32 offset, gray;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			offset = (((y >> 2) << 4) * this->GetWidth()) + ((x >> 2) << 6)
					+ (((y % 4 << 2) + x % 4) << 1);
			color.r = *(image + offset + 1);
			color.g = *(image + offset + 32);
			color.b = *(image + offset + 33);

			gray = (77 * color.r + 150 * color.g + 28 * color.b) / 255;

			*(image + offset + 1) = gray;
			*(image + offset + 32) = gray;
			*(image + offset + 33) = gray;
		}
	}
	int len = width*height*4;
	if(len%32) len += (32-len%32);
	DCFlushRange(image, len);
}
*/
u32 GuiImage::GetRsxTexOffset()
{
	return rsxOffset;
}

u32 GuiImage::GetWPitch()
{
	return wpitch;
}
/**
 * Draw the button on screen
 */
void GuiImage::Draw()
{
    if( ( !image && imgType != IMAGE_COLOR )
        || !this->IsVisible()
		|| tile == 0
		|| rsxOffset == 0xffffffff
		|| !height
		|| !width )
	{
		//printf( "GuiImage::Draw() abort: %p, %u, %i, %08x\n", image, this->IsVisible(), tile, rsxOffset );
		return;
	}

	int currLeft = this->GetLeft();
    float currScale = this->GetScale();


    if( scale != 1 )
	{
		//TODO:  ALIGN_RIGHT is not handled here  (ALIGN_CENTER is just do nothing)
		if( alignment & ALIGN_LEFT )
			currLeft = currLeft - width/2 + (width*scale)/2;
	}

	if(tile > 0)
	{
		for(int i=0; i<tile; i++)
			MenuDrawImage( rsxOffset, format, width, height, wpitch, currLeft + width * i, this->GetTop(), zoffset, imageangle, this->GetAlpha(), currScale );
            //Menu_DrawImg(currLeft+width*i, this->GetTop(), width, height, image, imageangle, currScale, currScale, this->GetAlpha());
	}
	else
    {
		// temporary (maybe), used to correct offset for scaled images
        //if( scale != 1 )
            //currLeft = currLeft - ( ( width + ( width * scale ) ) / 2 );
            //currLeft = currLeft - width/2 + (width*scale)/2;

		//Menu_DrawImg(currLeft, this->GetTop(), width, height, image, imageangle, currScale, currScale, this->GetAlpha());

        if( imgType == IMAGE_COLOR )
        {
			MenuDrawRectangle( ( ( color & 0xffffff00 ) | ( this->GetAlpha() & 0xff ) ), width, height, currLeft, this->GetTop(), zoffset, true, imageangle, currScale );
        }
        else
        {
			MenuDrawImage( rsxOffset, format, width, height, wpitch, currLeft, this->GetTop(), zoffset, imageangle, this->GetAlpha(), currScale );
        }
    }

    /*if(stripe > 0)
		for(int y=0; y < this->GetHeight(); y+=6)
			Menu_DrawRectangle(currLeft,this->GetTop()+y,this->GetWidth(),3,(GXColor){0, 0, 0, stripe},1);*/

	this->UpdateEffects();
}
