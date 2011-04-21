

#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "devicethread.h"
#include "fileops.h"
#include "menu.h"
#include "stringstuff.h"


namespace DeviceThread
{
static bool haltdev = true;
static bool devhalted = true;
static bool devThreadRunning = false;

static sys_mutex_t hDevMutex;
static sys_mutex_t listMutex;
static sys_mutex_t h2DevMutex;
static sys_mutex_t wDevMutex;
static sys_cond_t devWaitCondition;
static sys_cond_t haltWaitCondition;

static sys_ppu_thread_t deviceThread;

using namespace std;

//list of currently connected HDDs
static vector<string>devList;

static bool listChanged = true;

static void UpdateList()
{
	vector<string> newList = FileOps::DevList( DEV_USER_STORAGE );
	vector<string>::iterator it = newList.begin();

	sysMutexLock( listMutex, 0 );

	//check which devices have been added
	while( it < newList.end() )
	{
		if( find( devList.begin(), devList.end(), *it ) == devList.end() )
		{
			listChanged = true;
		}
		++it;
	}

	//check which devices have been removed
	it = devList.begin();
	while( it < devList.end() )
	{
		if( find( newList.begin(), newList.end(), *it ) == newList.end() )
		{
			listChanged = true;
		}
		++it;
	}

	//assign new lest
	devList = newList;
	sysMutexUnlock( listMutex );
}

static void DeviceThreadMain( void * arg )
{
	devThreadRunning = true;
	bool halt;

	while( 1 )
	{
		//check if GuiHalt() has been called
		sysMutexLock( hDevMutex, 0 );
		halt = haltdev;
		sysMutexUnlock( hDevMutex );
		if( halt )
		{
			while( 1 )
			{
				sysThreadYield();
				//signal that this thread has halted
				if( sysMutexTryLock( h2DevMutex ) )
					continue;

				sysCondSignal( haltWaitCondition );
				sysMutexUnlock( h2DevMutex );

				//wait for Resume() to be called
				if( sysMutexTryLock( wDevMutex ) )
					continue;
				devhalted = true;
				sysCondWait( devWaitCondition, 0 );
				devhalted = false;
				sysMutexUnlock( wDevMutex );

				break;
			}
		}
		else
		{
			//check if exit() has been called
			sysMutexLock( exitRequestMutex, 0 );
			bool exit = exitRequested;
			sysMutexUnlock( exitRequestMutex );
			if( exit )
			{
				break;
			}

			//do stuff
			UpdateList();
			usleep( 1000 * 1000 * 3 );
		}
		sysThreadYield();
	}
	devThreadRunning = false;

	sysThreadExit( 0 );
}

void Halt()
{
	while( 1 )
	{
		sysThreadYield();
		//signal thread to wait
		if( sysMutexTryLock( hDevMutex ) )
			continue;

		haltdev = true;
		sysMutexUnlock( hDevMutex );

		//wait for gui thread to halt
		if( sysMutexTryLock( h2DevMutex ) )
			continue;

		sysCondWait( haltWaitCondition, 0 );
		sysMutexUnlock( h2DevMutex );

		break;
	}
}

void Resume()
{
	u16 wtf = 1;
	while( 1 )
	{
		if( !wtf++ )
		{
			printf("threads deadlocked (device & calling thread)\n");
			exit( 0 );
		}
		sysThreadYield();
		//signal thread to wait
		if( sysMutexTryLock( hDevMutex ) )
			continue;
		haltdev = false;
		sysMutexUnlock( hDevMutex );

		if( devThreadRunning && !devhalted && !exitRequested )
			continue;

		if( sysMutexTryLock( wDevMutex ) )
			continue;
		sysCondSignal( devWaitCondition );
		sysMutexUnlock( wDevMutex );

		break;
	}
}

void Init()
{
	//thread parameters
	u64 priority = 200;
	size_t stack_size = 0x2000;
	const char *thread_name = "Device Thread";

	//initialize mutex & wait condition
	mutex_init( &listMutex );
	mutex_init( &hDevMutex );
	mutex_init( &h2DevMutex );
	mutex_init( &wDevMutex );
	cond_init( &devWaitCondition, &wDevMutex );
	cond_init( &haltWaitCondition, &h2DevMutex );

	int s = sysThreadCreate( &deviceThread, DeviceThreadMain, NULL, priority, stack_size, THREAD_JOINABLE, (char *)thread_name );
	if( s )
	{
		printf("create device thread returned %i.  exiting...\n", s );
		exit( 0 );
	}

}

void Shutdown()
{
	//make sure this thread has exited
	if( devThreadRunning )
	{
		sysMutexLock( hDevMutex, 0 );
		bool halt = haltdev;
		sysMutexUnlock( hDevMutex );

		if( halt )
			Resume();
	}
	u64 retval;
	int t = sysThreadJoin( deviceThread, &retval );
	if( t )
		printf("device thread tried to join with return: %llX, sysThreadJoin returned %d\n", (unsigned long long int)retval, t );

	//destroy mutexes
	sysMutexDestroy( hDevMutex );
	sysMutexDestroy( listMutex );
	sysMutexDestroy( h2DevMutex );
	sysMutexDestroy( wDevMutex );
	sysCondDestroy( devWaitCondition );
	sysCondDestroy( haltWaitCondition );
	//printf("device thread is killed\n");
}

vector<string> CurrentDevList()
{
	sysMutexLock( listMutex, 0 );
	vector<string> ret = devList;
	listChanged = false;
	sysMutexUnlock( listMutex );
	return ret;
}

vector<string> DevicesRemoved( vector<string> &oldList )
{
	//cout << "DevicesRemoved()" << endl;
	vector<string> ret;
	vector<string>::iterator it = oldList.begin();
	sysMutexLock( listMutex, 0 );
	while( it < oldList.end() )
	{
		if( find( devList.begin(), devList.end(), *it ) == devList.end() )
		{
			ret.push_back( *it );
		}
		++it;
	}
	sysMutexUnlock( listMutex );
	sort( ret.begin(), ret.end() );
	return ret;
}

vector<string> DevicesAdded( vector<string> &oldList )
{
	//cout << "DevicesAdded()" << endl;
	vector<string> ret;
	sysMutexLock( listMutex, 0 );
	vector<string>::iterator it = devList.begin();
	while( it < devList.end() )
	{
		if( find( oldList.begin(), oldList.end(), *it ) == oldList.end() )
		{
			ret.push_back( *it );
		}
		++it;
	}
	sysMutexUnlock( listMutex );
	sort( ret.begin(), ret.end() );
	return ret;
}

bool DevListChanged()
{
	sysMutexLock( listMutex, 0 );
	bool ret = listChanged;
	sysMutexUnlock( listMutex );
	return ret;
}

}
