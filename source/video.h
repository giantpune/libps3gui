#ifndef VIDEO_H
#define VIDEO_H

#include <tiny3d.h>

//window size
#define WINDOW_HEIGHT	511.0f
#define WINDOW_WIDTH	847.0f


#define GUI_TEXT_COLOR          0xf0f0f0ff //almost black


//clear teh screen and prepare to draw some stuff
void MenuPrepareToDraw();

//adjust over/underscan
void AdjustViewport( float x, float y );

//draw an image
//! rsxOffset is the texture location inside the allocated rsx memory
//! format is the texture format: TINY3D_TEX_FORMAT_?????
//! width and height are the dimensions to draw the image
//! wpitch is the wpitch of the image
//! x, y, and z are the coords to draw it
//! angleZ is the rotation (on the Z axis)
//! alpha is the alpha transparency to use
//! scale is used to stretch the image on the x & Y axises
void MenuDrawImage( u32 rsxOffset, u32 format, float width, float height, u32 wpitch, float x, float y, float z, float angleZ, u8 alpha, float scale );

//draw a rectangle
//! basically the same params as MenuDrawImage()
//! rgba is the color to use
//! and filled is a 1 or 0 to decide if the rectangle should be filled with color or just an outline
void MenuDrawRectangle( u32 rgba, float width, float height, float x, float y, float z, u8 filled, float angleZ, float scale );

//fill the screen up with a certain color
//! rgba is the color to use
void MenuFillBackground( u32 rgba );

#endif // VIDEO_H
