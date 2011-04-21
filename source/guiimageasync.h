#ifndef GUIIMAGEASYNC_H
#define GUIIMAGEASYNC_H
#include <string>
#include <utility>
#include <vector>

#include "gui.h"
#include "utils.h"

// class to handle asynchronous images.
//!  you pass an already existing imagedata to the constructor, along with an image path, format, and target
//! the constructor then adds itself to a list.  there is a thread running that pulls the first object from this
//! list and attempt to load the image specified.  the image will display the preloaded image whenever it is drawn
//! if the thread is able to load teh image, it will set the new data, size, width, format, etc to the object
//! and from then on, the object will act as a basic GuiImage.

//! the "Target" parameter probably is a bad name for it, but heres how it works...
//! GuiImageAsync( someImgData, "/dev_hdd0/path/file", TINY3D_TEX_FORMAT_R5G6B5, GuiImageAsync::Any );
//! will attempt to load "/dev_hdd0/path/file.png" and failing that, "/dev_hdd0/path/file.jpg".
//! failing both of those, it will give up

//! GuiImageAsync( someImgData, "/dev_hdd0/path/file.png", TINY3D_TEX_FORMAT_R5G6B5, GuiImageAsync::Exact );
//! will attempt to load "/dev_hdd0/path/file.png".  if this exact name is not found, it will give up

class GuiImageAsync : public GuiImage
{
public:
	enum Target
	{
		Any = 0x10000,	//try to load any supported file matching the filename
						//currently is .png and .jpg
		Exact = 0x20000	//load only the exact path specified
	};

	GuiImageAsync( GuiImageData *preload, const std::string &path, u32 format = TINY3D_TEX_FORMAT_R5G6B5, Target = Exact );
	~GuiImageAsync();

	// basically the same as the parent one, except that it locks itself against mutex if the thread
	// may still be messing with it
	void Draw();

	//check if this image has been loaded
	bool IsLoaded();

	//lock and unlock the mutex for this item's data
	bool Lock();
	void UnLock();
private:
	//stuff for handeling the thread
	static void InitThread();
	static void WakeThread();

	//the thread just runs forever once started.  no reason to kill it on app exit
	static void ThreadMain( void* arg );

	static bool threadInited;
	static sys_mutex_t listMutex;
	static sys_mutex_t lwMutex;
	static sys_cond_t waitCondition;
	static sys_ppu_thread_t workThread;

	static std::vector< std::pair < GuiImageAsync *, u32 > >list;

	static void RemoveFirstEntry();


	//stuff for each individual instance of this class
	sys_mutex_t dataMutex;					// avoid multiple threads trying to access the image data at the same time
	std::string imagePath;					// path of the image to load for this instance

	//when the thread assigns this image and imagedata, store it here so it can be deleted later
	GuiImageData *imgData;
	u32 destFormat;
	bool loaded;
	Target target;

	void AddToList();
	void RemoveFromList();
};

#endif // GUIIMAGEASYNC_H
