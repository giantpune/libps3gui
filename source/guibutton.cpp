/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 * giantpune 2011
 *
 * gui_button.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <unistd.h>

#include "gui.h"

GuiButton::GuiButton(int w, int h)
{
	width = w;
	height = h;
	image = NULL;
	imageOver = NULL;
	imageHold = NULL;
	imageClick = NULL;
	icon = NULL;
	iconOver = NULL;
	iconHold = NULL;
	iconClick = NULL;

	for(int i=0; i < 3; i++)
	{
		label[i] = NULL;
		labelOver[i] = NULL;
		labelHold[i] = NULL;
		labelClick[i] = NULL;
	}


	soundOver = NULL;
	soundHold = NULL;
	soundClick = NULL;
	selectable = true;
	holdable = false;
	clickable = true;
}

/**
 * Destructor for the GuiButton class.
 */
GuiButton::~GuiButton()
{
}

void GuiButton::SetImage(GuiImage* img)
{
	image = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageOver(GuiImage* img)
{
	imageOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageHold(GuiImage* img)
{
	imageHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetImageClick(GuiImage* img)
{
	imageClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIcon(GuiImage* img)
{
	icon = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconOver(GuiImage* img)
{
	iconOver = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconHold(GuiImage* img)
{
	iconHold = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetIconClick(GuiImage* img)
{
	iconClick = img;
	if(img) img->SetParent(this);
}
void GuiButton::SetLabel(GuiText* txt, int n)
{
	label[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelOver(GuiText* txt, int n)
{
	labelOver[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelHold(GuiText* txt, int n)
{
	labelHold[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetLabelClick(GuiText* txt, int n)
{
	labelClick[n] = txt;
	if(txt) txt->SetParent(this);
}
void GuiButton::SetSoundOver(GuiSound * snd)
{
	soundOver = snd;
}
void GuiButton::SetSoundHold(GuiSound * snd)
{
	soundHold = snd;
}
void GuiButton::SetSoundClick(GuiSound * snd)
{
	soundClick = snd;
}

/**
 * Draw the button on screen
 */
void GuiButton::Draw()
{
	if(!this->IsVisible())
		return;

	// draw image
	if((state == STATE_SELECTED || state == STATE_HELD) && imageOver)
		imageOver->Draw();
	else if(image)
		image->Draw();
	// draw icon
	if((state == STATE_SELECTED || state == STATE_HELD) && iconOver)
		iconOver->Draw();
	else if(icon)
		icon->Draw();
	// draw text
	for(int i=0; i<3; i++)
	{
		if((state == STATE_SELECTED || state == STATE_HELD) && labelOver[i])
			labelOver[i]->Draw();
		else if(label[i])
        {
            //printf( "gui button calling draw text for label[ %i ]\n", i );
			label[i]->Draw();
        }
	}

	this->UpdateEffects();
}

//printf("\n");
void GuiButton::Update(GuiTrigger * t)
{
	if(state == STATE_CLICKED || state == STATE_DISABLED || !t || !IsVisible() )
		return;
	else if(parentElement && parentElement->GetState() == STATE_DISABLED)
		return;

	if( t->pad.showCursor && t->chan >= 0 )
	{
		if( this->IsInside( t->pad.cursorX, t->pad.cursorY ) )
		{
			if( state == STATE_DEFAULT ) // we weren't on the button before!
			{
				this->SetState( STATE_SELECTED, t->chan );

//				if(this->Rumble())
//					rumbleRequest[t->chan] = 1;

				if(soundOver)
					soundOver->Play();

				if( effectsOver && !effects )
				{
					// initiate effects
					effects = effectsOver;
					effectAmount = effectAmountOver;
					effectTarget = effectTargetOver;
				}
			}
		}
		else
		{
			if( state == STATE_SELECTED && ( stateChan == t->chan || stateChan == -1 ) )
				this->ResetState();

			if( effectTarget == effectTargetOver && effectAmount == effectAmountOver )
			{
				// initiate effects (in reverse)
				effects = effectsOver;
				effectAmount = -effectAmountOver;
				effectTarget = 100;
			}
		}
	}

	// button triggers
	if( this->IsClickable() )
	{
		for( int i = 0; i < 2; i++ )
		{
			if(trigger[ i ] && ( trigger[ i ]->chan == -1 || trigger[ i ]->chan == t->chan ) )
			{


				if( t->pad.pressed > 0 &&
					( t->pad.pressed & trigger[ i ]->btns ) )
				{
					if( t->chan == stateChan || stateChan == -1 )
					{
						if( state == STATE_SELECTED )
						{
							if( !t->pad.showCursor || this->IsInside( t->pad.cursorX, t->pad.cursorY ) )
							{
								this->SetState( STATE_CLICKED, t->chan );

								if( soundClick )
									soundClick->Play();
							}
						}
						else if( trigger[ i ]->type == TRIGGER_BUTTON_ONLY )
						{
							this->SetState( STATE_CLICKED, t->chan );
						}
						else if( trigger[ i ]->type == TRIGGER_BUTTON_ONLY_IN_FOCUS &&
								parentElement->IsFocused())
						{
							this->SetState( STATE_CLICKED, t->chan );
						}
					}
				}
			}
		}
	}

	if( this->IsHoldable() )
	{
		bool held = false;
		for( int i = 0; i < 2; i++ )
		{
			if( trigger[ i ] && ( trigger[ i ]->chan == -1 || trigger[ i ]->chan == t->chan ) )
			{

				if( t->pad.pressed > 0 &&
					( t->pad.pressed & trigger[ i ]->btns ) )
				{
					if( trigger[ i ]->type == TRIGGER_HELD && state == STATE_SELECTED &&
						( t->chan == stateChan || stateChan == -1 ) )
					{
						this->SetState( STATE_CLICKED, t->chan );

					}
				}

				if( ( t->pad.held > 0 && ( t->pad.held & trigger[ i ]->btns ) )
					|| ( t->pad.pressed > 0 && ( t->pad.pressed & trigger[ i ]->btns ) ) )
				{
					if(trigger[ i ]->type == TRIGGER_HELD )
					{
						held = true;
					}
				}

				if( !held && state == STATE_HELD && stateChan == t->chan )
				{
					this->ResetState();
				}
				else if( held && state == STATE_CLICKED && stateChan == t->chan )
				{
					this->SetState( STATE_HELD, t->chan );
				}
			}
		}
	}

	if( updateCB )
		updateCB( this );
}

