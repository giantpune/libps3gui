#ifndef MENU_H
#define MENU_H


#include <sys/mutex.h>
#include <sys/thread.h>
#include <tiny3d.h>

//alignment
#define ALIGN_LEFT		1
#define ALIGN_RIGHT		2
#define ALIGN_CENTER	4
#define ALIGN_CENTRE	ALIGN_CENTER

#define ALIGN_TOP		0x10
#define ALIGN_BOTTOM	0x20
#define ALIGN_MIDDLE	0x40

enum
{
	MENU_NONE = 0,
	MENU_EXIT,
	MENU_SETTINGS,
	MENU_ABOUT,
	MENU_COVERFLOW,
	MENU_INSTALL
};



int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label);
void ErrorPrompt( const char* message );

void ProgressWindow( const char *title, const char *msg );

int MainMenu( int menu );
void HaltGui();
bool ResumeGui();
void exiting();
void InitGuiThread();

extern sys_mutex_t exitRequestMutex;
extern bool exitRequested;

#endif // MENU_H
