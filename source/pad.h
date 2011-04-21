#ifndef PAD_H
#define PAD_H

#include <io/pad.h>
#include <string.h>

#include "menu.h"

//these values are just arbatrarily assigned
#define BTN_LEFT_		1
#define BTN_DOWN_		2
#define BTN_RIGHT_		4
#define BTN_UP_			0x10
#define BTN_START_		0x20
#define BTN_R3_			0x40
#define BTN_L3_			0x100
#define BTN_SELECT_		0x200
#define BTN_SQUARE_		0x400
#define BTN_CROSS_		0x1000
#define BTN_CIRCLE_		0x2000
#define BTN_TRIANGLE_	0x4000
#define BTN_R1_			0x10000
#define BTN_L1_			0x20000
#define BTN_R2_			0x40000
#define BTN_L2_			0x100000

//ignore small movements in the analog sticks
#define PAD_STICK_DEADZONE	0x40
#define PAD_STICK_DEADZONE_MIN ( 0x80 - ( PAD_STICK_DEADZONE / 2 ) )
#define PAD_STICK_DEADZONE_MAX ( 0x80 + ( PAD_STICK_DEADZONE / 2 ) )

//smaller number = faster cursor  ( 18 seems too slow )
//#define PAD_CURSOR_FACTOR 12	//works well in 60fps mode
#define PAD_CURSOR_FACTOR 8		//works well when the framerate drops to 30
								//TODO, make automatic switching between the 2 of these

//ignore false l3 press due to stick moving while pressing l3 ( at least in my shitty controller )
#define PAD_CURSOR_TOGGLE_WAIT 10;

//support 4 input sources
#define GUI_MAX_PADS	4

//for convenience
#define BTN_DPAD_ ( BTN_LEFT_ | BTN_DOWN_ | BTN_UP_ | BTN_RIGHT_ )

//struct to provide pressed & held buttons
typedef struct MyPadData
{
	u32 pressed;		//contains buttons that were just now pressed
	u32 held;			//contains buttons that were either pressed or held last time the pads were scanned and are still held down
	u32 released;		//contains buttons that were held or pressed last scan, but are not down now
	s16 stickL_x;		//stick positions ranging from -0x80ish to +0x80ish
	s16 stickL_y;
	s16 stickR_x;
	s16 stickR_y;
	s16 cursorX;		//keep track of a cursor.  press L3 to toggle
	s16 cursorY;
	bool showCursor;
} MyPadData;

//extern MyPadData myPadData[ MAX_PADS ];

void PadInit();
void PadRead();


#endif // PAD_H
