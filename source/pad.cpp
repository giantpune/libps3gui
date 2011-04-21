
#include <io/pad.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "pad.h"
#include "gui.h"

#define PAD_SLEEP 1000

GuiTrigger userInput[ GUI_MAX_PADS ];

//delay to reduce ghost presses of l3 on my shitty controllers
static u8 l3fix[ GUI_MAX_PADS ];

static padInfo padinfo;
static padData paddata[ GUI_MAX_PADS ];

void PadInit()
{
	//clear input
	for( int i = 0; i < GUI_MAX_PADS; i++ )
	{
		memset( &paddata[ i ], 0, sizeof( padData ) );
		userInput[ i ].ClearPadData();
		userInput[ i ].pad.cursorX = WINDOW_WIDTH/2;
		userInput[ i ].pad.cursorY = WINDOW_HEIGHT/2;
		userInput[ i ].pad.showCursor = false;
		userInput[ i ].chan = i;
		l3fix[ i ] = 0;

		// wtf is this?
		// for some reason, the pad read in psl1ght doesnt alway set these values
		// so sometimes the program starts out thinking that the sticks are pushed all the way up and left
		// 0x80 is right in the middle
		paddata[ i ].ANA_R_H = 0x80;
		paddata[ i ].ANA_L_H = 0x80;
		paddata[ i ].ANA_R_V = 0x80;
		paddata[ i ].ANA_L_V = 0x80;
	}

	//init pslight pad library
	ioPadInit( GUI_MAX_PADS );

}

//read user input and populate "userInput"
void PadRead()
{
	//static PadInfo padinfo;
	//static PadData paddata[ GUI_MAX_PADS ];

	ioPadGetInfo( &padinfo );

	for( int i = 0; i < GUI_MAX_PADS; i++ )
	{
		if( !padinfo.status[ i ] )
		{
			continue;
		}
		//read input
		ioPadGetData( i, &paddata[ i ] );

		//process LEFT button
		if( paddata[ i ].BTN_LEFT )
		{
			if( userInput[ i ].pad.held & BTN_LEFT_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_LEFT_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFFFE;
				userInput[ i ].pad.held |= BTN_LEFT_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_LEFT_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_LEFT_ )
				|| ( userInput[ i ].pad.pressed & BTN_LEFT_) )
				{
				userInput[ i ].pad.released |= BTN_LEFT_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFFFE;
			}
			userInput[ i ].pad.held &= 0xFFFFFFFE;
			userInput[ i ].pad.pressed &= 0xFFFFFFFE;
		}

		//process down button
		if( paddata[ i ].BTN_DOWN )
		{
			if( userInput[ i ].pad.held & BTN_DOWN_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_DOWN_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFFFD;
				userInput[ i ].pad.held |= BTN_DOWN_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_DOWN_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_DOWN_ )
				|| ( userInput[ i ].pad.pressed & BTN_DOWN_) )
				{
				userInput[ i ].pad.released |= BTN_DOWN_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFFFD;
			}
			userInput[ i ].pad.held &= 0xFFFFFFFD;
			userInput[ i ].pad.pressed &= 0xFFFFFFFD;
		}

		//process RIGHT button
		if( paddata[ i ].BTN_RIGHT )
		{
			if( userInput[ i ].pad.held & BTN_RIGHT_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_RIGHT_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFFFB;
				userInput[ i ].pad.held |= BTN_RIGHT_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_RIGHT_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_RIGHT_ )
				|| ( userInput[ i ].pad.pressed & BTN_RIGHT_) )
				{
				userInput[ i ].pad.released |= BTN_RIGHT_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFFFB;
			}
			userInput[ i ].pad.held &= 0xFFFFFFFB;
			userInput[ i ].pad.pressed &= 0xFFFFFFFB;
		}

		//process UP button
		if( paddata[ i ].BTN_UP )
		{
			if( userInput[ i ].pad.held & BTN_UP_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_UP_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFFEF;
				userInput[ i ].pad.held |= BTN_UP_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_UP_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_UP_ )
				|| ( userInput[ i ].pad.pressed & BTN_UP_) )
				{
				userInput[ i ].pad.released |= BTN_UP_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFFEF;
			}
			userInput[ i ].pad.held &= 0xFFFFFFEF;
			userInput[ i ].pad.pressed &= 0xFFFFFFEF;
		}

		//process START button
		if( paddata[ i ].BTN_START )
		{
			if( userInput[ i ].pad.held & BTN_START_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_START_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFFDF;
				userInput[ i ].pad.held |= BTN_START_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_START_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_START_ )
				|| ( userInput[ i ].pad.pressed & BTN_START_) )
				{
				userInput[ i ].pad.released |= BTN_START_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFFDF;
			}
			userInput[ i ].pad.held &= 0xFFFFFFDF;
			userInput[ i ].pad.pressed &= 0xFFFFFFDF;
		}

		//process R3 button
		if( paddata[ i ].BTN_R3 )
		{
			if( userInput[ i ].pad.held & BTN_R3_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_R3_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFFBF;
				userInput[ i ].pad.held |= BTN_R3_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_R3_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_R3_ )
				|| ( userInput[ i ].pad.pressed & BTN_R3_) )
				{
				userInput[ i ].pad.released |= BTN_R3_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFFBF;
			}
			userInput[ i ].pad.held &= 0xFFFFFFBF;
			userInput[ i ].pad.pressed &= 0xFFFFFFBF;
		}

		//process L3 button
		if( paddata[ i ].BTN_L3 )
		{
			paddata[ i ].BTN_L3 = 0;
			if( !l3fix[ i ] )
			{
				l3fix[ i ] = PAD_CURSOR_TOGGLE_WAIT;
				if( userInput[ i ].pad.held & BTN_L3_ )		//already held down
				{//do nothing
				}
				else if( userInput[ i ].pad.pressed & BTN_L3_ )	//was pressed last time, change state to held
				{
					userInput[ i ].pad.pressed &= 0xFFFFFEFF;
					userInput[ i ].pad.held |= BTN_L3_;
				}
				else									//button was just pressed down now
				{
					userInput[ i ].pad.pressed |= BTN_L3_;
//					userInput[ i ].pad.showCursor ^= 1;//toggle show/hide cursor
				}
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_L3_ )
				|| ( userInput[ i ].pad.pressed & BTN_L3_) )
				{
				userInput[ i ].pad.released |= BTN_L3_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFEFF;
			}
			userInput[ i ].pad.held &= 0xFFFFFEFF;
			userInput[ i ].pad.pressed &= 0xFFFFFEFF;
		}

		//process SELECT button
		if( paddata[ i ].BTN_SELECT )
		{
			if( userInput[ i ].pad.held & BTN_SELECT_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_SELECT_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFDFF;
				userInput[ i ].pad.held |= BTN_SELECT_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_SELECT_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_SELECT_ )
				|| ( userInput[ i ].pad.pressed & BTN_SELECT_) )
				{
				userInput[ i ].pad.released |= BTN_SELECT_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFDFF;
			}
			userInput[ i ].pad.held &= 0xFFFFFDFF;
			userInput[ i ].pad.pressed &= 0xFFFFFDFF;
		}

		//process SQUARE button
		if( paddata[ i ].BTN_SQUARE )
		{
			if( userInput[ i ].pad.held & BTN_SQUARE_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_SQUARE_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFFBFF;
				userInput[ i ].pad.held |= BTN_SQUARE_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_SQUARE_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_SQUARE_ )
				|| ( userInput[ i ].pad.pressed & BTN_SQUARE_) )
				{
				userInput[ i ].pad.released |= BTN_SQUARE_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFFBFF;
			}
			userInput[ i ].pad.held &= 0xFFFFFBFF;
			userInput[ i ].pad.pressed &= 0xFFFFFBFF;
		}

		//process CROSS button
		if( paddata[ i ].BTN_CROSS )
		{
			if( userInput[ i ].pad.held & BTN_CROSS_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_CROSS_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFEFFF;
				userInput[ i ].pad.held |= BTN_CROSS_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_CROSS_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_CROSS_ )
				|| ( userInput[ i ].pad.pressed & BTN_CROSS_) )
				{
				userInput[ i ].pad.released |= BTN_CROSS_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFEFFF;
			}
			userInput[ i ].pad.held &= 0xFFFFEFFF;
			userInput[ i ].pad.pressed &= 0xFFFFEFFF;
		}

		//process CIRCLE button
		if( paddata[ i ].BTN_CIRCLE )
		{
			if( userInput[ i ].pad.held & BTN_CIRCLE_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_CIRCLE_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFDFFF;
				userInput[ i ].pad.held |= BTN_CIRCLE_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_CIRCLE_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_CIRCLE_ )
				|| ( userInput[ i ].pad.pressed & BTN_CIRCLE_) )
				{
				userInput[ i ].pad.released |= BTN_CIRCLE_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFDFFF;
			}
			userInput[ i ].pad.held &= 0xFFFFDFFF;
			userInput[ i ].pad.pressed &= 0xFFFFDFFF;
		}

		//process TRIANGLE button
		if( paddata[ i ].BTN_TRIANGLE )
		{
			if( userInput[ i ].pad.held & BTN_TRIANGLE_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_TRIANGLE_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFFBFFF;
				userInput[ i ].pad.held |= BTN_TRIANGLE_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_TRIANGLE_;
			}

			//printf( "goodbye (failsafe in PadRead())\n" );
			//exit( 0 );
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_TRIANGLE_ )
				|| ( userInput[ i ].pad.pressed & BTN_TRIANGLE_) )
				{
				userInput[ i ].pad.released |= BTN_TRIANGLE_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFFBFFF;
			}
			userInput[ i ].pad.held &= 0xFFFFBFFF;
			userInput[ i ].pad.pressed &= 0xFFFFBFFF;
		}

		//process R1 button
		if( paddata[ i ].BTN_R1 )
		{
			if( userInput[ i ].pad.held & BTN_R1_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_R1_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFEFFFF;
				userInput[ i ].pad.held |= BTN_R1_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_R1_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_R1_ )
				|| ( userInput[ i ].pad.pressed & BTN_R1_) )
				{
				userInput[ i ].pad.released |= BTN_R1_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFEFFFF;
			}
			userInput[ i ].pad.held &= 0xFFFEFFFF;
			userInput[ i ].pad.pressed &= 0xFFFEFFFF;
		}

		//process L1 button
		if( paddata[ i ].BTN_L1 )
		{
			if( userInput[ i ].pad.held & BTN_L1_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_L1_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFDFFFF;
				userInput[ i ].pad.held |= BTN_L1_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_L1_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_L1_ )
				|| ( userInput[ i ].pad.pressed & BTN_L1_) )
				{
				userInput[ i ].pad.released |= BTN_L1_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFDFFFF;
			}
			userInput[ i ].pad.held &= 0xFFFDFFFF;
			userInput[ i ].pad.pressed &= 0xFFFDFFFF;
		}

		//process R2 button
		if( paddata[ i ].BTN_R2 )
		{
			if( userInput[ i ].pad.held & BTN_R2_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_R2_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFFBFFFF;
				userInput[ i ].pad.held |= BTN_R2_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_R2_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_R2_ )
				|| ( userInput[ i ].pad.pressed & BTN_R2_) )
				{
				userInput[ i ].pad.released |= BTN_R2_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFFBFFFF;
			}
			userInput[ i ].pad.held &= 0xFFFBFFFF;
			userInput[ i ].pad.pressed &= 0xFFFBFFFF;
		}

		//process L2 button
		if( paddata[ i ].BTN_L2 )
		{
			if( userInput[ i ].pad.held & BTN_L2_ )		//already held down
			{//do nothing
			}
			else if( userInput[ i ].pad.pressed & BTN_L2_ )	//was pressed last time, change state to held
			{
				userInput[ i ].pad.pressed &= 0xFFEFFFFF;
				userInput[ i ].pad.held |= BTN_L2_;
			}
			else									//button was just pressed down now
			{
				userInput[ i ].pad.pressed |= BTN_L2_;
			}
		}
		else										//button not pressed
		{
			if( ( userInput[ i ].pad.held & BTN_L2_ )
				|| ( userInput[ i ].pad.pressed & BTN_L2_) )
				{
				userInput[ i ].pad.released |= BTN_L2_;
			}
			else
			{
				userInput[ i ].pad.released &= 0xFFEFFFFF;
			}
			userInput[ i ].pad.held &= 0xFFEFFFFF;
			userInput[ i ].pad.pressed &= 0xFFEFFFFF;
		}


		//analog sticks
		if( paddata[ i ].ANA_L_H < PAD_STICK_DEADZONE_MIN || paddata[ i ].ANA_L_H > PAD_STICK_DEADZONE_MAX )
		{
			userInput[ i ].pad.stickL_x = ( paddata[ i ].ANA_L_H - 0x80 );
		}
		else
		{
			userInput[ i ].pad.stickL_x = 0;
		}

		if( paddata[ i ].ANA_L_V < PAD_STICK_DEADZONE_MIN || paddata[ i ].ANA_L_V > PAD_STICK_DEADZONE_MAX )
		{
			userInput[ i ].pad.stickL_y = ( paddata[ i ].ANA_L_V - 0x80 );
		}
		else
		{
			userInput[ i ].pad.stickL_y = 0;
		}

		if( paddata[ i ].ANA_R_H < PAD_STICK_DEADZONE_MIN || paddata[ i ].ANA_R_H > PAD_STICK_DEADZONE_MAX )
		{
			userInput[ i ].pad.stickR_x = ( paddata[ i ].ANA_R_H - 0x80 );
		}
		else
		{
			userInput[ i ].pad.stickR_x = 0;
		}

		if( paddata[ i ].ANA_R_V < PAD_STICK_DEADZONE_MIN || paddata[ i ].ANA_R_V > PAD_STICK_DEADZONE_MAX )
		{
			userInput[ i ].pad.stickR_y = ( paddata[ i ].ANA_R_V - 0x80 );
		}
		else
		{
			userInput[ i ].pad.stickR_y = 0;
		}

		//move cursor
		if( userInput[ i ].pad.showCursor )
		{
			userInput[ i ].pad.cursorX += userInput[ i ].pad.stickL_x/PAD_CURSOR_FACTOR;
			userInput[ i ].pad.cursorY += userInput[ i ].pad.stickL_y/PAD_CURSOR_FACTOR;

			if( userInput[ i ].pad.cursorX < 0 )
				userInput[ i ].pad.cursorX = 0;

			else if( userInput[ i ].pad.cursorX > WINDOW_WIDTH )
				userInput[ i ].pad.cursorX = WINDOW_WIDTH;

			if( userInput[ i ].pad.cursorY < 0 )
				userInput[ i ].pad.cursorY = 0;

			else if( userInput[ i ].pad.cursorY > WINDOW_HEIGHT )
				userInput[ i ].pad.cursorY = WINDOW_HEIGHT;
		}
		//printf("pad[%i] pressed: %08x  held: %08x   released: %08x rsx: %i  rsy: %i  lsx: %i  lsy: %i\n",\
		//	   i, userInput[ i ].pad.pressed, userInput[ i ].pad.held, userInput[ i ].pad.released, \
		//	   userInput[ i ].pad.stickR_x, userInput[ i ].pad.stickR_y, userInput[ i ].pad.stickL_x, userInput[ i ].pad.stickL_y );

		if( l3fix[ i ] )
			l3fix[ i ]--;
	}

	return;

}

