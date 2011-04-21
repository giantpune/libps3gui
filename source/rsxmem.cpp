#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>
#include <vector>
#include <map>

#include "rsxmem.h"
#include "utils.h"

using namespace std;
namespace RsxMem
{

struct MemItr
{
	u32 start;
	u32 end;
};

static vector<MemItr> ptrs;

static sys_mutex_t rsxMutex;

static u32 memSize = 0;
static u32 *memStart = NULL;

static bool Lock()
{
	u16 dontFReeze = 0;
retry:
	sysThreadYield();

	// ghetto avoid thread deadlock :)
	if( !++dontFReeze )
	{
		printf("RsxMem::Lock() failed\n" );
		return false;
	}

	if( sysMutexTryLock( rsxMutex ) )
		goto retry;

	return true;
}

static void UnLock()
{
	sysMutexUnlock( rsxMutex );
}

bool Init( u32 size )
{
	//printf("RsxMem::Init( %08x )\n", size );

	//make sure we're not already inited
	if( memSize )
	{
		printf("RsxMem::Init( %08x ) failed:  init is already called\n", size );
		return false;
	}

    //create mutex
    mutex_init( &rsxMutex );

	//try to grab the big chunk of memory
	memStart = (u32*)tiny3d_AllocTexture( size );
	if( !memStart )
	{
		printf("RsxMem::Init( %08x ) failed to allocate memory\n", size );
		return false;
	}

	//set variables
	memSize = size;
	//memset( (void*)&ptr, 0, sizeof( MemItr ) * RSX_MEM_MAX_TEXTURES );

	//printf("memSize: %08x\nmemStart: %p\n", memSize, memStart );
	return true;
}

u8 *Alloc( u32 bytes, u32* off )
{
	if( !Lock() )
		return NULL;
	sysMutexLock( rsxMutex, 0 );
	//align
	bytes = RU( bytes, 4 );

	//printf("ptrs.size(): %i\n", (int)ptrs.size() );
	//find the first available chunk of memory big enough
	u32 offset = 0;
	u32 i = 0;
	vector<MemItr>::iterator ptr = ptrs.begin();
	while( ptr < ptrs.end() )
	{
		//if there is another chunk of allocated memory after this one, check the space between them
		if( i + 1 < ptrs.size() )
		{
			if( ( ptrs.at( i + 1 ).start - (*ptr).end ) >= bytes )
			{
				//printf("%08x - %08x >= %08x    2\n", ptrs.at( i + 1 ).start, (*ptr).end, bytes );
				offset = ptrs.at( i ).end;
				i++;
				break;
			}
		}
		//if this is the last texture
		else if( i == ptrs.size() - 1 )
		{
			if( ( memSize - (*ptr).end ) >= bytes )
			{
				//printf("%08x - %08x >= %08x     1\n", memSize, (*ptr).end, bytes );
				offset = (*ptr).end;
				i++;
			}
			else
			{
				printf("RsxMem::Alloc() :didnt find enough free space for %08x\n", bytes );
				UnLock();
				return NULL;
			}
			break;
		}
		++ptr;
		++i;
	}

	MemItr newPtr;
	newPtr.start = offset;
	newPtr.end = offset + bytes;

	ptrs.insert( ptrs.begin() + i, newPtr );
	//printf("inserting new chunk in list @ %u\n", i );

	if( off )
	{
		*off = tiny3d_TextureOffset( (u8*)memStart + offset );
	}

    u8 *ret = ((u8*)memStart) + offset;
	UnLock();
    return ret;
}

void Free( u8 *p )
{
	if( !Lock() )//better to leak memory than to have the app freeze in a thread deadlock
	{
		printf("RsxMem failed to aquire mutex.  memory leaking\n");
		return;
	}
	//printf("RsxMem::Free( %p )\n", p );
	u32 offset = p - ((u8*)memStart);
	//printf("looking for offset: %08x\n", offset );
	vector<MemItr>::iterator ptr = ptrs.begin();
	while( ptr < ptrs.end() )
	{
		if( (*ptr).start == offset )
		{
			ptrs.erase( ptr );
			sysMutexUnlock( rsxMutex );
			return;
		}
		++ptr;
	}
	printf("RsxMem::Free( %p ) :no entry matched that pointer\n", p );
	UnLock();
}

u32 Offset( u8 *p )
{
	return tiny3d_TextureOffset( (u32*)p );
}

void PrintInfo( bool verbose )
{
    sysMutexLock( rsxMutex, 0 );
	printf("-------------------------------------------------------------------------\n");
	printf("RsxMem::PrintInfo\n");
	printf("memSize: %08x (%.2fMiB)  memStart: %p  numPtrs: %u\n", memSize, (float)((float)memSize/(float)1048576.0f ), memStart, (u32)ptrs.size() );
	u32 used = 0;
	u32 free = 0;
	u32 i = 0;
	if( verbose )
		printf("[idx]   start  ->    end   (   size   )    addr\n" );
	vector<MemItr>::iterator ptr = ptrs.begin();
	while( ptr < ptrs.end() )
	{
		u32 bytes = ( (*ptr).end - (*ptr).start );
		if( verbose )
		{
			printf("[ %u ] %08x -> %08x ( %08x ) %p\n",\
				   i, (*ptr).start, (*ptr).end, bytes, ( ((u8*)memStart) + (*ptr).start ) );
		}
		used += bytes;
		++ptr;
		++i;
	}
	free = memSize - used;
	printf("used: %08x (%.2fMiB)  free: %08x (%.2fMiB)\n", used, (float)((float)used/(float)1048576.0f ), free, (float)((float)free/(float)1048576.0f ) );
	printf("-------------------------------------------------------------------------\n");
    sysMutexUnlock( rsxMutex );
}

}
