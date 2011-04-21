
#include <cstdlib>

#include "boxbrowser.h"
#include "fileops.h"
#include "pad.h"
#include "rsxmem.h"
#include "warez.h"
#include "wstring.h"


INC_FILE( nocoverFull_png );

#define BUTTON_DELAY	6
#define MAX_SPEED		6


#define BB_COVER_PATH "user/cover"		//todo, make this a setting somewhere

enum mode
{
	M_IDLE,
	M_LEFT,
	M_RIGHT
};

extern GuiFont * font;

GuiImageData *BoxBrowser::noCoverdata = NULL;

BoxBrowser::BoxBrowser()
{
	currentSelection = 0;
	busy = false;
	ignoreInput = false;
	needToUpdateBoxes = true;
	showDetails = false;
	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
	mode = M_IDLE;
	speed = 0;
	buttonDelay = 0;
	animFrame = 0;
	inputIdx = 0;
	listIdx = 0;
	viewMode = viewFlow;
	bgImg1 = NULL;
	bgImg2 = NULL;
	//ratingImg = NULL;
	//ratingImgData = NULL;
	camY = 0;
	camdif = 1;
	camDelay = 3;
	DefaultZoom();

	gameText = new GuiText( font, (char*)NULL, 20, 0x00ff00ff );
	gameText->SetAlignment( ALIGN_TOP | ALIGN_CENTER );
	gameText->SetPosition( 0, 40 );
	gameText->SetParent( this );

//	zIdTxt = new GuiText( font, (char*)NULL, 20, 0x00ff00ff );
//	zIdTxt->SetAlignment( ALIGN_TOP | ALIGN_RIGHT );
//	zIdTxt->SetPosition( 0, -40 );
//	zIdTxt->SetParent( this );

	CreateMaxtrices();

	//create nocover data
	noCoverdata = new (std::nothrow) GuiImageData( nocoverFull_png, nocoverFull_png_size, TINY3D_TEX_FORMAT_R5G6B5 );
	if( !noCoverdata )
	{
		printf("oh snap! failed to create noCoverdata.  this wont end well\n");
		return;
	}
	//create boxes
	for( int i = 0; i < NUM_BOXES; i++ )
	{
		box[ i ] = new BoxCover();
	}
}

BoxBrowser::~BoxBrowser()
{
	for( int i = 0; i < NUM_BOXES; i++ )
	{
		delete box[ i ];
	}

	delete gameText;
//	delete zIdTxt;
	delete noCoverdata;

	ClearCache();

	if( bgImg1 )
		delete bgImg1;

	if( bgImg2 )
		delete bgImg2;

//	if( ratingImg )
//		delete ratingImg;

//	if( ratingImgData )
//		delete ratingImgData;
}

void BoxBrowser::ClearCache( bool full )
{
	//RsxMem::PrintInfo( true );
	u32 r1Low = 0xffffffff;
	u32 r1High = 0xffffffff;
	u32 r2Low = 0xffffffff;
	u32 r2High = 0xffffffff;
	std::map<u32, GuiImageAsync *> keepers;
	if( !full )
	{
		r1Low = listIdx;
		r1High = listIdx + NUM_BOXES;
		if( r1High >= (u32)GameList::Count() )
		{
			r1High = GameList::Count() - 1;
			r2Low = 0;
			r2High = NUM_BOXES - ( r1High - r1Low );
		}
	}
	else
	{
		needToUpdateBoxes = true;
	}
	if( coverImg.size() )
	{
		for( std::map<u32, GuiImageAsync *>::iterator i = coverImg.begin(), iEnd = coverImg.end(); i != iEnd; ++i )
		{
			if( full
				|| ( !IsInRange( (*i).first, r1Low, r1High) && !IsInRange( (*i).first, r2Low, r2High) ) )
			{
				if( (*i).second )
					delete (*i).second;
			}
			else
			{
				keepers[ (*i).first ] = (*i).second;
			}
		}
	}
	coverImg = keepers;
}

void BoxBrowser::UpdateCamera()
{
	//adjust the camera a little bit
	MATRIX cam = MatrixRotationX( FROM_ANGLE( -3.5 ) );
	cam = MatrixMultiply( cam, MatrixTranslation( 0, camY, 0 ) );
	tiny3d_SetProjectionMatrix( &cam );
	if( !--camDelay )
	{
		camDelay = 3;
		camY += camdif;
		if( camY == 10 )
		{
			camdif = -1;
		}
		else if( !camY )
		{
			camdif = 1;
		}
	}
}

void BoxBrowser::Draw()
{
	if( !visible )
		return;
	if( needToUpdateBoxes )
	{
		needToUpdateBoxes = false;
		viewMode = viewFlow;
		Shift( 0 );
	}
	if( viewMode == viewFlow )
	{
		//draw text and any other 2D elements ( and leave it in 2D for the rest of the gui )
		gameText->Draw();
		//change to 3D context to draw boxes
		tiny3d_Project3D();

		//draw the surface
		DrawSurface();

		//update camera motion
		UpdateCamera();

		for( int i = 0; i < NUM_BOXES; i++ )
		{
			int mtxIdx = ( ( i * ANIMATION_FRAMES ) + animFrame );
			if( mtxIdx >= (int)mtxBox.size() || mtxIdx < 0 )
			{
				continue;
			}
			tiny3d_SetMatrixModelView( &mtxBox[ mtxIdx ] );
			box[ i ]->DrawNoMtx();
		}
		UpdateAnimation();
	}
	else if( viewMode == viewZoom )
	{
		//draw text and any other 2D elements ( and leave it in 2D for the rest of the gui )
		if( bgImg2 )
		{
			bgImg2->Draw();
		}
		if( bgImg1 )
		{
			//make sure this image is positioned correctly
			if( !bgLoaded && bgImg1->IsLoaded() )
			{
				bgLoaded = true;
				//this image should either be 1000px wide or 1920
				if( bgImg1->GetWidth() <= 1000 )
					bgImg1->SetPosition( 124, 69, 0xfff0 );//these values seem to work for all my guitar hero & rockband games
				else
					bgImg1->SetPosition( 0, 0, 0xfff0 );//i dont have any games that fit this category, but i assume this is right
			}
			bgImg1->Draw();
		}

		//change to 3D
		tiny3d_Project3D();

		//update camera motion
		UpdateCamera();

		//set matrix for zoomed box
		SetZoomMatrix();

		//the center box hopefully is the selected one
		box[ NUM_BOXES / 2 ]->DrawNoMtx();
	}


	//done in 3D mode, switch to 2D
	tiny3d_SetMatrixModelView( NULL ); // set matrix identity
	tiny3d_Project2D();

	busy = false;
	inputIdx = 0;
}

void BoxBrowser::DrawSurface()
{
	//vars
	VECTOR fr;
	VECTOR fl;
	VECTOR bl;
	VECTOR br;

	fl.x = -1400;
	fl.y = 500;
	fl.z = -500;

	fr.x = -fl.x;
	fr.y = fl.y;
	fr.z = fl.z;

	br.x = fr.x;
	br.y = 200;
	br.z = 0xffff;

	bl.x = fl.x;
	bl.y = br.y;
	bl.z = br.z;

	u32 rgba = 0x141414ff;

	//start drawing
	tiny3d_SetPolygon( TINY3D_QUADS );

	tiny3d_VertexPosVector( fl );
	tiny3d_VertexColor( rgba );
	tiny3d_VertexPosVector( fr );
	tiny3d_VertexPosVector( br );
	tiny3d_VertexPosVector( bl );

	tiny3d_End();
}

int BoxBrowser::GetSelectedGame()
{
	if( speed || animFrame || !GameList::Count() )
		return -1;
	return currentSelection;
}

//shifts the image datas to a different box and shifts the index
void BoxBrowser::Shift( int num )
{
	//shift list index
	listIdx -= num;
	if( listIdx >= GameList::Count() )
	{
		listIdx = 0;
	}
	else if( listIdx < 0 )
	{
		listIdx = GameList::Count() - 1;
	}

	// check cache size
	// probably could be more elegant than this, but this should work for now
	if( coverImg.size() + abs(num) > BB_MAX_CACHE )
		ClearCache( false );

	//assign new image to boxes
	int lOff = ( NUM_BOXES / 2 );
	for( int i = 0; i < NUM_BOXES; i++ )
	{
		u32 cvrIdx = ( ( ( listIdx ) + i ) % GameList::Count() );
		box[ i ]->SetImage( Get( cvrIdx ) );

		if( i == lOff )
		{
			//set title text
			currentSelection = cvrIdx;
			gameText->SetText( GameList::Name( currentSelection ).c_str() );
		}
	}
	//RsxMem::PrintInfo( true );
}

GuiImageAsync *BoxBrowser::Get( u32 idx )
{
	std::map<u32, GuiImageAsync *>::iterator it = coverImg.find( idx );
	if( it != coverImg.end() )
	{
		return (*it).second;
	}
	return Cache( idx );
}

GuiImageAsync *BoxBrowser::Cache( u32 idx )
{
	if( idx >= (u32)GameList::Count() )
		return NULL;

	Game game = GameList::At( idx );
	GuiImageAsync *img = new (std::nothrow) GuiImageAsync( noCoverdata, game.Path() + BB_COVER_PATH, \
														   TINY3D_TEX_FORMAT_R5G6B5, GuiImageAsync::Any );
	if( !img )
		return NULL;
	coverImg[ idx ] = img;

	return img;
}

void BoxBrowser::CreateMaxtrices()
{
	mtxBox.clear();

	float zoom1 = 405.0f;
	float zoom2 = 405.0f;
	float zoom3 = 405.0f;
	float zoom4 = 405.0f;
	float zoom5 = 405.0f;

	float posY1 = 362.0f;
	float posY2 = 362.0f;
	float posY3 = 362.0f;
	float posY4 = 362.0f;
	float posY5 = 362.0f;

	float posZ1 = 5600.0f;
	float posZ2 = 5600.0f;
	float posZ3 = 5600.0f;
	float posZ4 = 5600.0f;
	float posZ5 = 5600.0f;

	float rotX1 = 7.5f;
	float rotX2 = 7.5f;
	float rotX3 = 7.5f;
	float rotX4 = 7.5f;
	float rotX5 = 7.5f;

	float rotZ1 = 180;
	float rotZ2 = 180;
	float rotZ3 = 180;
	float rotZ4 = 180;
	float rotZ5 = 180;

	float angleLeft = 105.0f;
	float angleCenter = 180.0f;
	float angleRight = 75.0f;

	float angleDeltaLeft = 2.0f;
	float angleDeltaRight = 2.0f;

	float spacingSides = 110.0f;
	float spacingCenter = 400.0f;

	float selectedX = 0.0f;

	float rightAdjXpos = 100.0f;//move all the boxes on the right over a bit


	int left = BOXES_PER_SIDE - 1;
	int right = 0;

	int sideFrames = ( ( BOXES_PER_SIDE - 1 ) * ( ANIMATION_FRAMES ) ) + 1;
	int sideCenterFrames = ( ANIMATION_FRAMES - 1 );
	float angleDeltaLeftMtx = ( angleDeltaLeft / ANIMATION_FRAMES );
	float angleDeltaLeftCenterMtx = ( angleCenter - angleLeft ) / ANIMATION_FRAMES;
	float angleDeltaRightMtx = ( angleDeltaRight / ANIMATION_FRAMES );
	float angleDeltaRightCenterMtx = ( angleRight - angleCenter ) / ANIMATION_FRAMES;
	float spacingSidesMtx = ( spacingSides / ANIMATION_FRAMES );
	left = sideFrames;
	right = 0;
	//create left matrices
	for( int i = 0; i < sideFrames; i++ )
	{
		//CreateMatrix( int idx, float xpos, float ypos, float zpos, float xrot, float yrot, float zrot, float scale )
		int rotx, roty, rotz, posx, posy, posz;
		float scale;

		rotx = ( rotX2 - ( left * ( rotX2 - rotX1 ) / sideFrames ) );
		roty = ( angleLeft - ( left * angleDeltaLeftMtx ) );
		rotz = ( rotZ2 - ( left * ( rotZ2 - rotZ1 ) / BOXES_PER_SIDE ) );

		posx = ( selectedX - ( spacingCenter + ( left * spacingSidesMtx ) ) );
		posy = ( posY2 - ( left * ( posY2 - posY1 ) / sideFrames ) );
		posz = ( posZ2 - ( left * ( posZ2 - posZ1 ) / sideFrames ) );

		scale = ( zoom2 - ( left * ( zoom2 - zoom1 ) / sideFrames ) );
		left--;

		CreateMatrix( posx, posy, posz, rotx, roty, rotz, scale );
	}

	//create center matrices
	int leftCenterSpacingX = ( selectedX - spacingCenter ) / ANIMATION_FRAMES;
	left = sideCenterFrames;
	for( int i = 0; i < sideCenterFrames; i++ )
	{
		int rotx, roty, rotz, posx, posy, posz;
		float scale;

		rotx = ( rotX3 - ( left * ( rotX3 - rotX2 ) / sideCenterFrames ) );
		roty = ( angleLeft + ( i * angleDeltaLeftCenterMtx ) );
		rotz = ( rotZ3 - ( left * ( rotZ3 - rotZ2 ) / sideCenterFrames ) );

		posx = ( selectedX + ( left * leftCenterSpacingX ) );
		posy = ( posY3 - ( left * ( posY3 - posY2 ) / sideCenterFrames ) );
		posz = ( posZ3 - ( left * ( posZ3 - posZ2 ) / sideCenterFrames ) );

		scale = ( zoom3 - ( left * ( zoom3 - zoom2 ) / sideCenterFrames ) );
		left--;

		CreateMatrix( posx, posy, posz, rotx, roty, rotz, scale );
	}

	//center matrix
	CreateMatrix( selectedX, posY3, posZ3, rotX3, angleCenter, rotZ3, zoom3 );

	//create right-center matrices
	float rightCenterSpacingX = ( selectedX - ( spacingCenter + rightAdjXpos ) ) / ANIMATION_FRAMES;
	left = sideCenterFrames;
	for( int i = 0; i < sideCenterFrames; i++ )
	{
		int rotx, roty, rotz, posx, posy, posz;
		float scale;

		rotx = ( rotX4 + ( i * ( rotX5 - rotX4 ) / sideCenterFrames ) );
		roty = ( angleCenter + ( ( i + 1 ) * angleDeltaRightCenterMtx ) );
		rotz = ( rotZ4 + ( i * ( rotZ5 - rotZ4 ) / sideCenterFrames ) );

		posx = ( selectedX - ( i * rightCenterSpacingX ) );
		posy = ( posY4 + ( i * ( posY5 - posY4 ) / sideCenterFrames ) );
		posz = ( posZ4 + ( i * ( posZ5 - posZ4 ) / sideCenterFrames ) );

		scale = ( zoom4 + ( i * ( zoom5 - zoom4 ) / sideCenterFrames ) );

		CreateMatrix( posx, posy, posz, rotx, roty, rotz, scale );
	}

	//create right matrices
	for( int i = 0; i < sideFrames; i++ )
	{
		//CreateMatrix( int idx, float xpos, float ypos, float zpos, float xrot, float yrot, float zrot, float scale )
		int rotx, roty, rotz, posx, posy, posz;
		float scale;

		rotx = ( rotX4 + ( right * ( rotX5 - rotX4 ) / sideFrames ) );
		roty = ( angleRight + ( right * angleDeltaRightMtx ) );
		rotz = ( rotZ4 + ( right * ( rotZ5 - rotZ4 ) / BOXES_PER_SIDE ) );

		posx = ( selectedX + rightAdjXpos + ( spacingCenter + ( right * spacingSidesMtx ) ) );
		posy = ( posY4 + ( right * ( posY5 - posY4 ) / sideFrames ) );
		posz = ( posZ4 + ( right * ( posZ5 - posZ4 ) / sideFrames ) );
		//posz = 600.0f;

		scale = ( zoom4 + ( right * ( zoom5 - zoom4 ) / sideFrames ) );
		//scale = 42.0f;
		++right;

		CreateMatrix( posx, posy, posz, rotx, roty, rotz, scale );
	}
}

void BoxBrowser::CreateMatrix( float xpos, float ypos, float zpos, float xrot, float yrot, float zrot, float scale )
{
	MATRIX mtxScale = MatrixScale( scale, scale, scale );
	MATRIX matrixX = MatrixRotationX( FROM_ANGLE( xrot ) );
	MATRIX matrixZ = MatrixRotationZ( FROM_ANGLE( zrot ) );

	// rotate and translate
	MATRIX mx = MatrixRotationY( FROM_ANGLE( yrot ) );					//rotate
	mx = MatrixMultiply( mx, matrixX );
	mx = MatrixMultiply( mx, matrixZ );
	mx = MatrixMultiply( mx, mtxScale );								//scale this thing up

	mx = MatrixMultiply( mx, MatrixTranslation( xpos, ypos, zpos ) ); //translate into place

	//put this matrix in the list
	mtxBox.push_back( mx );
}

void BoxBrowser::DefaultZoom()
{
	zPosX = -565;
	zPosY = 367.0f;
	zPosZ = 5600.0f;

	zScale = 436.0f;

	zRotX = 9.0f;
	zRotY = 156.0f;
	zRotZ = 180.0f;
}

void BoxBrowser::SetZoomMatrix()
{
	MATRIX mtxScale = MatrixScale( zScale, zScale, zScale );
	MATRIX matrixX = MatrixRotationX( FROM_ANGLE( zRotX ) );
	MATRIX matrixZ = MatrixRotationZ( FROM_ANGLE( zRotZ ) );

	// rotate and translate
	MATRIX mx = MatrixRotationY( FROM_ANGLE( zRotY ) );					//rotate
	mx = MatrixMultiply( mx, matrixX );
	mx = MatrixMultiply( mx, matrixZ );
	mx = MatrixMultiply( mx, mtxScale );								//scale this thing up

	mx = MatrixMultiply( mx, MatrixTranslation( zPosX, zPosY, zPosZ ) ); //translate into place

	//apply matrix
	tiny3d_SetMatrixModelView( &mx );
}

void BoxBrowser::UpdateAnimation()
{
	switch( mode )
	{
	case M_LEFT:
		animFrame -= speed;
		if( animFrame <= -ANIMATION_FRAMES )
		{
			animFrame = ( animFrame % ANIMATION_FRAMES );
			Shift( -1 );
		}
		break;
	case M_RIGHT:
		animFrame += speed;
		if( animFrame >= ANIMATION_FRAMES )
		{
			animFrame = ( animFrame % ANIMATION_FRAMES );
			Shift( 1 );
		}
		break;
	default:
	case M_IDLE:
		return;
		break;
	}
}

void BoxBrowser::ChangeSpeed( u8 up )
{
	if( up )
	{
		if( speed > MAX_SPEED )
			return;
		speed++;
	}
	else
	{
		if( !speed )
			return;
		speed--;
	}
}

void BoxBrowser::IgnoreInput( bool ignore )
{
	ignoreInput = ignore;
}

void BoxBrowser::LoadBgImages()
{
	//delete existing images
	if( bgImg1 )
	{
		delete bgImg1;
		bgImg1 = NULL;
	}
	if( bgImg2 )
	{
		delete bgImg2;
		bgImg2 = NULL;
	}
	bgLoaded = false;

	//check for out of range
	if( currentSelection < 0 || currentSelection >= GameList::Count() )
		return;

	//try to load new images for this game
	string path = GameList::At( currentSelection ).Path();
	if( path.empty() )
		return;

	//load this image first ( its the background one )
	bgImg2 = new (std::nothrow) GuiImageAsync( NULL, path + "PS3_GAME/PIC1.PNG" );
	if( bgImg2 )
	{
		//printf("creating image 1\n");
		bgImg2->SetScale( 0.4416666f );//window width/1920
		bgImg2->SetPosition( 0, 0, 0xfff0 );//this image should always be 1920x1080
		bgImg2->SetAlignment( ALIGN_CENTER | ALIGN_MIDDLE );
		bgImg2->SetParent( this );
	}

	//this one may or not exist for each game.  it is an optional overlay telling about the game
	bgImg1 = new (std::nothrow) GuiImageAsync( NULL, path + "PS3_GAME/PIC0.PNG", TINY3D_TEX_FORMAT_A8R8G8B8 );
	if( bgImg1 )
	{
		//printf("creating image 2\n");
		bgImg1->SetScale( 0.4416666f );//window width/1920
		bgImg1->SetAlignment( ALIGN_CENTER | ALIGN_MIDDLE );//set position for this image after it is loaded
		bgImg1->SetParent( this );
	}

}

//limits for zooming the box
#define Z_POS_MIN_X		-860.0f
#define Z_POS_MIN_Y		-28.0f

#define Z_ROT_MIN_X		0
#define Z_ROT_MIN_Y		0
#define Z_ROT_MIN_Z		0

#define Z_SCALE_MIN		170.0f

#define Z_POS_MAX_X		850.0f
#define Z_POS_MAX_Y		728.0f

#define Z_ROT_MAX_X		359
#define Z_ROT_MAX_Y		359
#define Z_ROT_MAX_Z		359

#define Z_SCALE_MAX		1372.0f

#define POS_DELTA		5
#define ANGLE_DELTA		3

void BoxBrowser::Update( GuiTrigger * t )
{
	inputIdx++;
	if( !visible || busy )//after responding to 1 controller, ignore all others till redraw
		return;

	switch( viewMode )//respond differently to button depending on what screen is showing
	{
	default:
	case viewFlow:
		{
			if( !ignoreInput )
			{
				//shift coverflow
				if( t->pad.pressed & BTN_LEFT_ || t->pad.held & BTN_LEFT_  )
				{
					if( buttonDelay )
					{
						buttonDelay--;
					}
					else
					{
						mode = M_LEFT;
						buttonDelay = BUTTON_DELAY;
						ChangeSpeed( 1 );
					}
					busy = true;
				}
				else if( t->pad.pressed & BTN_RIGHT_ || t->pad.held & BTN_RIGHT_ )
				{

					if( buttonDelay )
					{
						buttonDelay--;
					}
					else
					{
						mode = M_RIGHT;
						buttonDelay = BUTTON_DELAY;
						ChangeSpeed( 1 );
					}
					busy = true;
				}
				//respond to left stick for movement
				else if( t->pad.stickL_x < 0 )
				{
					mode = M_LEFT;
					if( t->pad.stickL_x < -120 )
					{
						speed = MAX_SPEED;
					}
					else if( t->pad.stickL_x < -80 )
					{
						speed = 4;
					}
					else if( t->pad.stickL_x < -60 )
					{
						speed = 2;
					}
					else
					{
						speed = 1;
					}
					busy = true;
				}
				else if( t->pad.stickL_x > 0 )
				{
					mode = M_RIGHT;
					if( t->pad.stickL_x > 120 )
					{
						speed = MAX_SPEED;
					}
					else if( t->pad.stickL_x > 80 )
					{
						speed = 4;
					}
					else if( t->pad.stickL_x > 60 )
					{
						speed = 2;
					}
					else
					{
						speed = 1;
					}
					busy = true;
				}
				//change to zoom mode
				else if( t->pad.pressed & BTN_CROSS_ )
				{
					viewMode = viewZoom;

					//clear cache to make sure there is room for 2 1920 x 1080 images
					ClearCache( false );

					LoadBgImages();
					speed = 0;//cancel any pending animation
					animFrame = 0;

				}
				//debugging memory availibility
				else if( t->pad.pressed & BTN_CIRCLE_ )
				{
					RsxMem::PrintInfo( true );
				}
			}
			//no button is pressed that we care about, start slowing down animation
			if( !busy && inputIdx == GUI_MAX_PADS )
			{
				buttonDelay = 0;
				ChangeSpeed( 0 );
				if( animFrame )//make sure it doesnt stop in the middle of animation
				{
					if( !speed )
						speed = 1;
				}
				else
				{
					ChangeSpeed( 0 );
					if( !speed )
						mode = M_IDLE;
				}
			}

		}
		break;
	case viewZoom:
		{
			//change back to coverflow mode
			if( !ignoreInput  )
			{
				if( t->pad.pressed & BTN_CROSS_  )
				{
					//if( currentSelection < 0 || currentSelection > GameList::Count() )
					//	return;
					//bool warez = Warez::LoadGame( GameList::At( currentSelection ).Path() );
					//printf("warez: %u\n", warez );

					//delete bigass background images to make room for more covers
					if( bgImg1 )
					{
						delete bgImg1;
						bgImg1 = NULL;
					}
					if( bgImg2 )
					{
						delete bgImg2;
						bgImg2 = NULL;
					}

					viewMode = viewFlow;
				}
				//move box left/right
				else if( ( ( t->pad.pressed & BTN_LEFT_ ) || ( t->pad.held & BTN_LEFT_ ) )
					&& zPosX > Z_POS_MIN_X )
				{
					zPosX -= POS_DELTA;
					//printf("zPosX: %.2f  zPosY: %.2f\n", zPosX, zPosY );
				}
				else if( ( ( t->pad.pressed & BTN_RIGHT_ ) || ( t->pad.held & BTN_RIGHT_ ) )
					&& zPosX < Z_POS_MAX_X )
				{
					zPosX += POS_DELTA;
					//printf("zPosX: %.2f  zPosY: %.2f\n", zPosX, zPosY );
				}
				//move box up/down
				if( ( ( t->pad.pressed & BTN_UP_ ) || ( t->pad.held & BTN_UP_ ) )
					&& zPosY > Z_POS_MIN_Y )
				{
					zPosY -= POS_DELTA;
					//printf("zPosX: %.2f  zPosY: %.2f\n", zPosX, zPosY );
				}
				else if( ( ( t->pad.pressed & BTN_DOWN_ ) || ( t->pad.held & BTN_DOWN_ ) )
					&& zPosX < Z_POS_MAX_Y )
				{
					zPosY += POS_DELTA;
					//printf("zPosX: %.2f  zPosY: %.2f\n", zPosX, zPosY );
				}
				//rotate on Y axis
				if( t->pad.stickR_x > 0 )
				{
					zRotY -= ANGLE_DELTA;
					if( zRotY < Z_ROT_MIN_Y )
						zRotY = 360;
					//printf("zRotX: %.2f  zRotY: %.2f\n", zRotX, zRotY );
				}
				else if( t->pad.stickR_x < 0 )
				{
					zRotY += ANGLE_DELTA;
					if( zRotY >= Z_ROT_MAX_Y )
						zRotY = 0;
					//printf("zRotX: %.2f  zRotY: %.2f\n", zRotX, zRotY );
				}
				//rotate on X axis
				if( t->pad.stickR_y < 0 )
				{
					zRotX -= ANGLE_DELTA;
					if( zRotX < Z_ROT_MIN_X )
						zRotX = 360;
					//printf("zRotX: %.2f  zRotY: %.2f\n", zRotX, zRotY );
				}
				else if( t->pad.stickR_y > 0 )
				{
					zRotX += ANGLE_DELTA;
					if( zRotX >= Z_ROT_MAX_X )
						zRotX = 0;
					//printf("zRotX: %.2f  zRotY: %.2f\n", zRotX, zRotY );
				}
				//scale
				if( ( ( t->pad.pressed & BTN_L2_ ) || ( t->pad.held & BTN_L2_ ) )
					&& zScale > Z_SCALE_MIN )
				{
					zScale *= 0.98039f;
					//printf("zScale: %.2f\n", zScale );
				}
				else if( ( ( t->pad.pressed & BTN_R2_ ) || ( t->pad.held & BTN_R2_ ) )
					&& zScale < Z_SCALE_MAX )
				{
					zScale *= 1.02;
					//printf("zScale: %.2f\n", zScale );
				}
				//shift to a different game
				if( ( t->pad.pressed & BTN_R1_ )  )
				{
					Shift( -1 );
					LoadBgImages();
				}
				else if( ( t->pad.pressed & BTN_L1_ ) )
				{
					Shift( 1 );
					LoadBgImages();
				}
				//reset position
				if( ( t->pad.pressed & BTN_TRIANGLE_ ) )
				{
					DefaultZoom();
				}
			}
		}
		break;
	}
}

#if ( BB_MAX_CACHE < NUM_BOXES )
#error "BB_MAX_CACHE < NUM_BOXES"
#endif
