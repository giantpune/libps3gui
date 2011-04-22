/****************************************************************************
 * Copyright (C) 2011
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/


#include "boxbrowser.h"
#include "BoxCover.hpp"
#include "BoxMesh.hpp"
#include "rsxmem.h"
#include "utils.h"

#define BOX_ALPHA ((u8)0xe8)
static GuiImageData *noCoverData = NULL;//this is never allocated, it is just a copy of one created in the boxbrowser
static GuiImageData *boxBorder = NULL;
static GuiImageData *boxLogos = NULL;
static u32 defaultBoxColor = 0xd2d4e700 | BOX_ALPHA ;

int BoxCover::cnt = 0;

BoxCover::BoxCover()
{
	flatCover = false;
	imgData = NULL;
	alreadyCheckedSize = false;
	boxColor = defaultBoxColor;

	//make sure there is nocover loaded
	if( !cnt++ )
	{
		noCoverData = BoxBrowser::noCoverdata;
		boxBorder = new (std::nothrow) GuiImageData( Resource( "images/boxBorder.png" ), TINY3D_TEX_FORMAT_R5G6B5 );
		boxLogos = new (std::nothrow) GuiImageData( Resource( "images/boxLogos.png" ), TINY3D_TEX_FORMAT_R5G6B5 );
		if( !noCoverData || !boxLogos )
		{
			printf( "BoxCover::BoxCover(): noCover is NULL.  prepare for crashy-crashy\n");
			return;
		}
	}
}

BoxCover::~BoxCover()
{
	if( !--cnt )
	{
		//printf("deleting the last BoxCover\n");
		noCoverData = NULL;
		if( boxBorder )
		{
			delete boxBorder;
			boxBorder = NULL;
		}
		if( boxLogos )
		{
			delete boxLogos;
			boxLogos = NULL;
		}
	}
}

void BoxCover::CheckImageSize()
{
	if( alreadyCheckedSize || !imgData )
		return;

	if( !imgData->IsLoaded() )
		return;

	flatCover = imgData->GetWidth() < imgData->GetHeight();
	alreadyCheckedSize = true;
}

void BoxCover::SetImage( GuiImageAsync* stuff )
{
	imgData = stuff;
	alreadyCheckedSize = false;
	flatCover = false;
}

void BoxCover::DrawNoMtx()
{
	if( !this->IsVisible() || !imgData )
	{
		//printf("BoxCover::DrawNoMtx() skipping\n");
		return;
	}
	CheckImageSize();

	u8 BoxAlpha = (int) (alpha+alphaDyn) & 0xFF;
	u32 color = ( boxColor & 0xffffff00 ) | (u8)( MIN ( BoxAlpha, BOX_ALPHA ) );
	u32 whiteAlpha = 0xffffff00 | BoxAlpha;

	//load border texture
	tiny3d_SetTexture( 0, boxBorder->GetRsxTexOffset(), boxBorder->GetWidth(), boxBorder->GetHeight(), \
					   boxBorder->GetWPitch(), (text_format)boxBorder->Format(), TEXTURE_LINEAR );

	//border quads
	tiny3d_SetPolygon( TINY3D_QUADS );
	for( u32 j = 0; j < g_boxMeshQSize; ++j )
	{
		tiny3d_VertexPos( g_boxMeshQ[ j ].pos.x, \
						  g_boxMeshQ[ j ].pos.y, \
						  g_boxMeshQ[ j ].pos.z );
		tiny3d_VertexColor( color );
		tiny3d_VertexTexture( g_boxMeshQ[ j ].texCoord.x, \
							  g_boxMeshQ[ j ].texCoord.y );
	}
	tiny3d_End();

	//border triangles
	tiny3d_SetPolygon( TINY3D_TRIANGLES );
	for( u32 j = 0; j < g_boxMeshTSize; ++j )
	{
		tiny3d_VertexPos( g_boxMeshT[ j ].pos.x, \
						  g_boxMeshT[ j ].pos.y, \
						  g_boxMeshT[ j ].pos.z );
		tiny3d_VertexColor( color );
		tiny3d_VertexTexture( g_boxMeshT[ j ].texCoord.x, \
							  g_boxMeshT[ j ].texCoord.y );
	}
	tiny3d_End();

	//filler quads above the cover back & spine (no texture)
	tiny3d_SetPolygon( TINY3D_QUADS );
	for( u32 j = 0; j < g_boxMeshFillerQSize - 4; ++j )
	{
		tiny3d_VertexPos( g_boxMeshFillerQ[ j ].pos.x, \
						  g_boxMeshFillerQ[ j ].pos.y, \
						  g_boxMeshFillerQ[ j ].pos.z );
		tiny3d_VertexColor( color );
	}
	tiny3d_End();

	//the filler panel above the front cover, it has a texture with some logos on it
	tiny3d_SetPolygon( TINY3D_QUADS );
	tiny3d_SetTexture( 0, boxLogos->GetRsxTexOffset(), boxLogos->GetWidth(), boxLogos->GetHeight(), \
				   boxLogos->GetWPitch(), (text_format)boxLogos->Format(), TEXTURE_LINEAR );
	for( u32 j = g_boxMeshFillerQSize - 4; j < g_boxMeshFillerQSize; ++j )
	{
		tiny3d_VertexPos( g_boxMeshFillerQ[ j ].pos.x, \
						  g_boxMeshFillerQ[ j ].pos.y, \
						  g_boxMeshFillerQ[ j ].pos.z );
		//tiny3d_VertexColor( defaultBoxColor );
		tiny3d_VertexColor( color );

		tiny3d_VertexTexture( g_boxMeshFillerQ[ j ].texCoord.x, \
							  g_boxMeshFillerQ[ j ].texCoord.y );
	}
	tiny3d_End();

	//Back Cover (Might be flat)
	if( flatCover )
		tiny3d_SetTexture( 0, noCoverData->GetRsxTexOffset(), noCoverData->GetWidth(), noCoverData->GetHeight(), \
					   noCoverData->GetWPitch(), (text_format)noCoverData->Format(), TEXTURE_LINEAR );
	else
		tiny3d_SetTexture( 0, imgData->GetRsxTexOffset(), imgData->GetWidth(), imgData->GetHeight(), \
						   imgData->GetWPitch(), (text_format)imgData->Format(), TEXTURE_LINEAR );

	tiny3d_SetPolygon( TINY3D_QUADS );
	for( u32 j = 0; j < g_boxBackCoverMeshSize; ++j )
	{
		tiny3d_VertexPos( g_boxBackCoverMesh[ j ].pos.x, \
						  g_boxBackCoverMesh[ j ].pos.y, \
						  g_boxBackCoverMesh[ j ].pos.z );
		//if( flatCover )
		//	tiny3d_VertexColor( color );			//nocover wraps around the full box for now
		//else
			tiny3d_VertexColor( whiteAlpha );
		tiny3d_VertexTexture( g_boxBackCoverMesh[ j ].texCoord.x, \
							  g_boxBackCoverMesh[ j ].texCoord.y );
	}
	tiny3d_End();

	//front cover
	if(flatCover)
	{
		tiny3d_SetTexture( 0, imgData->GetRsxTexOffset(), imgData->GetWidth(), imgData->GetHeight(), \
						   imgData->GetWPitch(), (text_format)imgData->Format(), TEXTURE_LINEAR );
		tiny3d_SetPolygon( TINY3D_QUADS );
		for( u32 j = 0; j < g_flatCoverMeshSize; ++j )
		{


			tiny3d_VertexPos( g_flatCoverMesh[ j ].pos.x, \
							  g_flatCoverMesh[ j ].pos.y, \
							  g_flatCoverMesh[ j ].pos.z );
			tiny3d_VertexColor( whiteAlpha );
			tiny3d_VertexTexture( g_flatCoverMesh[ j ].texCoord.x, \
								  g_flatCoverMesh[ j ].texCoord.y );
		}
		tiny3d_End();
	}
	else
	{
		tiny3d_SetPolygon( TINY3D_QUADS );
		for( u32 j = 0; j < g_boxCoverMeshSize; ++j )
		{


			tiny3d_VertexPos( g_boxCoverMesh[ j ].pos.x, \
							  g_boxCoverMesh[ j ].pos.y, \
							  g_boxCoverMesh[ j ].pos.z );
			tiny3d_VertexColor( whiteAlpha );
			tiny3d_VertexTexture( g_boxCoverMesh[ j ].texCoord.x, \
								  g_boxCoverMesh[ j ].texCoord.y );
		}
		tiny3d_End();
	}
}
