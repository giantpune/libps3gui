#ifndef BOXBROWSER_H
#define BOXBROWSER_H

#include <map>
#include <vector>

#include "BoxCover.hpp"
#include "BoxMesh.hpp"
#include "gamelist.h"
#include "guiimageasync.h"
#include "gui.h"
#include "rsxmem.h"
#include "utils.h"

//dirty hardcoded constants.  see CreateMaxtrices(); for the rest of the positional parameters
#define ANIMATION_FRAMES 17
#define BOXES_PER_SIDE 6
#define NUM_BOXES ( ( BOXES_PER_SIDE * 2 ) + 1 )
#define MATRIX_CNT ( NUM_BOXES * ANIMATION_FRAMES )

// max guiimagedata to store in memory before dumping the cache and starting it again
#define BB_MAX_CACHE 70


//class to display an array of boxes
//! positions are currenty hardcoded, and it uses the gamelist provided by GameList.cpp
class BoxBrowser : public GuiWindow
{
public:
	enum ViewMode
	{
		viewFlow,	//coverflow mode
		viewZoom	//zoom in on 1 cover
	};

	//should be called by main thread
	BoxBrowser();
	~BoxBrowser();
	int GetSelectedGame();

	//dont respond to button presses
	//! used whan another window has focus
	void IgnoreInput( bool ignore = true );

	//should be called by gui thread
	void Update( GuiTrigger * t );
	void Draw();

	//clear image cache
	//! this needs to be done whenever the gamelist changes
	//! only call this while the gui thread is halted, or from inside that thread
	//! if full is false, it will not clear covers that are currently being displayed
	void ClearCache( bool full = true );

	//used to check which view this browser is displaying
	ViewMode GetViewMode() { return viewMode; }

protected:
	friend class BoxCover;//let the boxcover class access this one's nocover data

	//these should only be called by the gui thread
	void UpdateAnimation();
	void ChangeSpeed( u8 up );
	void Shift( int num );

	BoxCover *box[ NUM_BOXES ];
	GuiText *gameText;

	// this is static, but there is only meant to be one instance of this class at as time.
	// any more, and this behavior will need to be changed
	//this value also needs to point to a valid imagedata before constructing any BoxCover, as they share the same data
	static GuiImageData *noCoverdata;

	// list index in the gamelist, cover
	std::map<u32, GuiImageAsync *> coverImg;

	//loads an image into the cache
	//returns NULL on error (out of range)
	GuiImageAsync *Cache( u32 idx );

	//get an image from the cache, and if it isnt present, cache it
	//! idx is the index of the game in the global gamelist
	GuiImageAsync *Get( u32 idx );

	int currentSelection;
	bool busy;
	bool needToUpdateBoxes;
	int mode;
	int speed;
	int buttonDelay;
	int animFrame;
	bool ignoreInput;
	ViewMode viewMode;
	u8 inputIdx;//used to gather input from all 4 controllers and respond to only the first one

	int listIdx;//used to map which list item is really selected even though theres like 11 boxes

	//generate matrices ahead of time rather than doing it 1000s of times a second
	void CreateMatrix( float xpos, float ypos, float zpos,\
					   float xrot, float yrot, float zrot, float scale );
	//matrices to use while flipping trough covers
	std::vector<MATRIX>mtxBox;

	//matrices to use while zooming in on a box
	//std::vector<MATRIX>mtxZoom;

	//this is just a dirty function to create a list of matrices.
	// to make it user configurable, replace this function with something that accept some sort of variables
	void CreateMaxtrices();

	//draw the surface under the boxes
	void DrawSurface();

	//stuff dealing with the camera
	int camY;
	int camdif;
	int camDelay;
	void UpdateCamera();


	//stuff to deal with "zoomed in on a game" mode
	float zPosX;
	float zPosY;
	float zPosZ;
	float zScale;
	float zRotX;
	float zRotY;
	float zRotZ;
	bool bgLoaded;
	bool showDetails;
	GuiImageAsync *bgImg1;
	GuiImageAsync *bgImg2;
	//GuiText *zIdTxt;
	//GuiImage *ratingImg;
	//GuiImageData *ratingImgData;
	void LoadBgImages();
	void DefaultZoom();
	void SetZoomMatrix();
};

#endif // BOXBROWSER_H
