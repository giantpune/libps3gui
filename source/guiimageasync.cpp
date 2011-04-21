
#include <iostream>
#include <unistd.h>

#include "buffer.h"
#include "fileops.h"
#include "guiimageasync.h"
#include "utils.h"

bool GuiImageAsync::threadInited = false;
sys_mutex_t GuiImageAsync::listMutex;
sys_mutex_t GuiImageAsync::lwMutex;
sys_cond_t GuiImageAsync::waitCondition;
sys_ppu_thread_t GuiImageAsync::workThread;

std::vector<std::pair < GuiImageAsync *, u32 > >GuiImageAsync::list;

using namespace std;
GuiImageAsync::GuiImageAsync( GuiImageData *preload, const string &path, u32 format, Target target )
	: GuiImage( preload ), imagePath( path )
{
	imgData = NULL;
	loaded = false;
	//this->format = format;
	if( preload )
		this->format = preload->Format();
	destFormat = format;
	this->target = target;
	mutex_init( &dataMutex );
	InitThread();
	AddToList();
	WakeThread();
}

GuiImageAsync::~GuiImageAsync()
{
	RemoveFromList();
	Lock();
	if( imgData )
	{
		delete imgData;
		imgData = NULL;
	}

	UnLock();
	sysMutexDestroy( dataMutex );
}

void GuiImageAsync::Draw()
{
	if( loaded )//just draw normally
	{
		//printf("GuiImageAsync::Draw() 1\n");
		GuiImage::Draw();
		return;
	}
	if( Lock() )
	{
		//printf("GuiImageAsync::Draw() 2\n");
		GuiImage::Draw();
		UnLock();
	}
}

bool GuiImageAsync::IsLoaded()
{
	bool ret;
	Lock();
	ret = loaded;
	UnLock();
	return ret;
}

bool GuiImageAsync::Lock()
{
	if( sysMutexLock( dataMutex, 1000 * 1000 ) )
	{
		printf("GuiImageAsync::Lock() \"%s\" failed\n", imagePath.c_str() );
		return false;
	}

	return true;
}

void GuiImageAsync::UnLock()
{
	sysMutexUnlock( dataMutex );
}

void GuiImageAsync::AddToList()
{
	if( sysMutexLock( listMutex, 1000 * 1000 ) )
		return;

	list.push_back( pair < GuiImageAsync *, u32 >( this, destFormat | target ) );
	sysMutexUnlock( listMutex );
}

void GuiImageAsync::RemoveFromList()
{
	if( loaded )//this is already loaded, it shouldnt be in the list anyways
		return;

	if( sysMutexLock( listMutex, 1000 * 1000 ) )
		return;

	std::vector<pair < GuiImageAsync *, u32 > >::iterator it = list.begin();
	while( it < list.end() )
	{
		if( (*it).first == this )
		{
			list.erase( it );
			break;
		}
		++it;
	}
	sysMutexUnlock( listMutex );
}

void GuiImageAsync::InitThread()
{
	if( threadInited )
		return;
	u64 priority = 100;
	size_t stack_size = 0x2000;
	const char *thread_name = "async image thread";

	//initialize mutex & wait condition
	mutex_init( &listMutex );
	mutex_init( &lwMutex );
	cond_init( &waitCondition, &lwMutex );

	int s = sysThreadCreate( &workThread, ThreadMain, NULL, priority, stack_size, THREAD_JOINABLE, (char *)thread_name );
	if( s )
	{
		printf("create async thread returned %i.  exiting...\n", s );
		exit( 0 );
	}
	threadInited = true;
}

void GuiImageAsync::WakeThread()
{
	if( sysMutexLock( lwMutex, 1000 * 1000 ) )
		return;

	sysCondSignal( waitCondition );
	sysMutexUnlock( lwMutex );
}

void GuiImageAsync::RemoveFirstEntry()
{
	if( sysMutexLock( listMutex, 1000 * 1000 ) )
		return;

	if( list.size() )
		list.erase( list.begin() );

	sysMutexUnlock( listMutex );
}

void GuiImageAsync::ThreadMain( void* arg )
{
	while( 1 )
	{
		//get the first image in the list and try to load it
		if( sysMutexTryLock( listMutex ) )
		{
			sysThreadYield();
			continue;
		}
		if( list.size() )
		{
			//printf("list.size(): %u\n", (u32)list.size() );
			GuiImageAsync *cur = (*list.begin()).first;
			u32 fmt = (*list.begin()).second & 0xffff;			//lower 16 bits are format
			Target target = (Target)((*list.begin()).second & 0xffff0000);	//upper 16 bits are the target mode
			std::string path;
			if( !cur )//dunno why this would happen, but just in case
			{
				list.erase( list.begin() );
				sysMutexUnlock( listMutex );
				continue;
			}
			// get the path to load
			path = cur->imagePath;

			// unlock the mutex so other stuff can add items to the list
			sysMutexUnlock( listMutex );

			if( path.empty() )
			{
				RemoveFirstEntry();
				continue;
			}

			// try to load the file
			Buffer buf;
			switch( target )
			{
			case Any:
				{
					//printf("read file \"%s\"\n", path.c_str());
					buf = FileOps::ReadFileToBuffer( path + ".png" );
					if( buf.IsEmpty() )
						buf = FileOps::ReadFileToBuffer( path + ".jpg" );
				}
				break;
			default:
			case Exact:
				{
					buf = FileOps::ReadFileToBuffer( path );
				}
				break;
			}
			if( buf.IsEmpty() )
			{
				RemoveFirstEntry();
				continue;
			}
			//printf("loading with format: %08x target: %08x\n", fmt, target );



			// try to create imagedata
			GuiImageData * img = new (std::nothrow) GuiImageData( buf.Data(), buf.Size(), fmt );
			if( !img
				|| img->GetRsxTexOffset() == 0xffffffff ) // <- image failed to load, just use the preloaded data
			{
				RemoveFirstEntry();
				continue;
			}

			//see if the first list item is still the one we think it is
			if( sysMutexLock( listMutex, 1000 * 1000 ) )
			{
				delete img;
				continue;
			}

			// the first list entry has changed (been deleted) since we got the path
			if( !list.size() )
			{
				sysMutexUnlock( listMutex );
				delete img;
				continue;
			}

			if( cur != (*list.begin()).first || path != (*list.begin()).first->imagePath )
			{
				sysMutexUnlock( listMutex );
				//cout << "expected: \"" << path << "\"  got \"" <<  (*list.begin()).first->imagePath << endl;
				delete img;
				continue;
			}

			// set the new data to this instance
			if( cur->Lock() )
			{
				cur->SetImage( img );
				cur->imgData = img;
				cur->format = fmt;
				cur->loaded = true;
				cur->UnLock();
			}

			// remove this item from the list and continue on to the next;
			list.erase( list.begin() );
			sysMutexUnlock( listMutex );
			continue;
		}
		sysMutexUnlock( listMutex );

		if( !sysMutexLock( lwMutex, 1000 * 1000 ) )
		{
			sysCondWait( waitCondition, 0 );
			sysMutexUnlock( lwMutex );
		}

	}
}
