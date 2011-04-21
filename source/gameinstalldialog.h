#ifndef GAMEINSTALLDIALOG_H
#define GAMEINSTALLDIALOG_H

#include <sys/file.h>

#include "gui.h"
#include "gamelist.h"
#include "utils.h"

#define GAME_COPY_BLOCK_SIZE ( MiB * 20 )	//copy in 20MiB chunks

//class to handle installing a game
//! the basic idea here is to create an instance of this class and append it to the mainwindow
//! then in the main thread, use a loop and call Run().  check the return of that function to see what
//! has happened.
class GameInstallDialog : public GuiWindow
{
public:

	enum
	{
		CopyOk,			//still working, no error
		CopyStarting,	//now starting to copy stuff
		CopyDone,		//success
		CopyFailed,		//some error happened
		CopyCanceled	//user canceled the operation
	};

	GameInstallDialog( int w, int h );
	~GameInstallDialog();

	//call this function in a loop to actually do the copying
	//! it returns one of the above "Copy..." enums
	int Run();


	void Draw();

	//cancel installing
	//! if delete files is true, it will delete the directory that it has created and been writing file inside
	void Cancel( bool deleteFiles = true );

private:

	enum
	{
		ModeCheckGame,
		ModeReadFiles,
		ModeConfirmStart,
		ModeCopying,
		ModeDone,
		ModeError,
		ModeCancel
	};

	GuiImageData *progEmptyData;
	GuiImageData *progFullData;
	GuiImageData *progOutlineData;

	GuiImageData *btnImgData;
	GuiImageData *btnImgOverData;

	GuiImageData *gameImgData1;
	GuiImageData *gameImgData2;
	GuiImage * gameImg1;
	GuiImage * gameImg2;

	GuiImage *progEmptyImg;
	GuiImage *progFullImg;
	GuiImage *progOutlineImg;

	GuiImage *btnImg;
	GuiImage *btnImgOver;

	GuiText *okBtnTxt;
	GuiText *cancelBtnTxt;

	GuiText *statusTxt;
	GuiText *statusTxt2;

	GuiButton *okBtn;
	GuiButton *cancelBtn;

	std::string title;

	//variables for tracking the file copy status
	std::vector<std::string> entryList;
	std::string destPath;
	int mode;
	u8* copyBuffer;

	u32 currentFileSize;
	u32 currentFileDone;
	u64 totalSize;
	u64 totalDone;
	u64 startTime;
	u64 lastTime;
	float progress;
	u64 null;
	s32 fdIn;
	s32 fdOut;




	//create all the necessary directories and gather up all the necessary filenames at the same time
	bool GeneratePathList( const std::string &parent = "/dev_bdvd" );

	bool StartNextFile();
	bool CopyBlock();

	//load the bigass game banner images
	void LoadGameBanner();

	//show some message
	void Status( int t, const char *format, ...) __attribute__( ( format( printf, 3, 4 ) ) );
};

#endif // GAMEINSTALLDIALOG_H
