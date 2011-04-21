/****************************************************************************
 * Copyright (C) 2010
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
#ifndef BOX_COVER_HPP_
#define BOX_COVER_HPP_

#include "gui.h"
#include "guiimageasync.h"

#define Mtx MATRIX

//class to deal with a 3d box using hardcoded vertexes and texture coords
//! this is a gui element, but does not respect any of the properties such as scale and position
//! it expects that the boxbrowser class already has a guiimagedata created that holds the nocover data
//! and it expects that a modelview matrix has already been set when drawing it
class BoxCover : public GuiElement
{
    public:
		BoxCover();
		~BoxCover();

		//set the color of the box (rgba)
		void SetBoxColor(u32 c) { boxColor = c; };

		//draws the box using whatever modelview matrix is already set
		void DrawNoMtx();

		//sets an iage for this box
		void SetImage( GuiImageAsync* stuff );

    private:
		GuiImageAsync *imgData;
		bool alreadyCheckedSize;

		//determine if this is a flat cover or full cover
		void CheckImageSize();

		bool flatCover;
		u32 boxColor;

		// used to keep track of how many boxes there are total
		// the first box created makes sure that there is loaded the image data
		// for box borders.  and the last box deleted destroys it
		static int cnt;

};

#endif
