/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.cpp
 *
 * Generic file routines - reading, writing, browsing
 ***************************************************************************/

#include <sys/file.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <malloc.h>

#include "filebrowser.h"
#include "menu.h"

namespace FileBrowser
{
BROWSERINFO browser;
BROWSERENTRY * browserList = NULL; // list of files/folders in browser

char rootdir[ 0x10 ];

void Init()
{
	browser.mode = DIR_FILES | DIR_DIRS;
	Reset();
}

/****************************************************************************
 * ResetBrowser()
 * Clears the file browser memory, and allocates one initial entry
 ***************************************************************************/
void Reset()
{
	browser.numEntries = 0;
	browser.selIndex = 0;
	browser.pageIndex = 0;

	// Clear any existing values
	if(browserList != NULL)
	{
		free(browserList);
		browserList = NULL;
	}
	// set aside space for 1 entry
	browserList = (BROWSERENTRY *)malloc(sizeof(BROWSERENTRY));
	memset(browserList, 0, sizeof(BROWSERENTRY));
}

bool GoToParentDir()
{
	if( !browser.dir[ 0 ] )
		return false;

	int size;

	char* test = strtok( browser.dir, "/" );
	test = strtok(NULL,"/");
	if( !test )
	{
		size = 0;
	}
	else
	{
		while (test != NULL)
		{
			size = strlen(test);
			test = strtok(NULL,"/");
		}
		/* remove last subdirectory name */
		size = strlen(browser.dir) - size - 1;
	}
	browser.dir[size] = 0;
	return true;
}

/****************************************************************************
 * UpdateDirName()
 * Update curent directory name for file browser
 ***************************************************************************/
int UpdateDirName()
{
	int size=0;
	char * test;
	char temp[1024];

	/* current directory doesn't change */
	if (strcmp(browserList[browser.selIndex].filename,".") == 0)
	{
		return 0;
	}
	/* go up to parent directory */
	else if (strcmp(browserList[browser.selIndex].filename,"..") == 0)
	{
		/* determine last subdirectory namelength */
		sprintf(temp,"%s",browser.dir);
		test = strtok(temp,"/");
		test = strtok(NULL,"/");
		if( !test )
		{
			size = 0;
		}
		else
		{
			while (test != NULL)
			{
				size = strlen(test);
				test = strtok(NULL,"/");
			}

			/* remove last subdirectory name */
			size = strlen(browser.dir) - size - 1;
		}
		browser.dir[size] = 0;

		return 1;
	}
	/* Open a directory */
	int len = strlen(browser.dir);
	if( !len )
	{
		strcpy( browser.dir, browserList[browser.selIndex].filename );
	}
	/* test new directory namelength */
	else if( ( len + 1 + strlen( browserList[ browser.selIndex ].filename ) ) < MAXPATHLEN )
	{
		/* update current directory name */
		sprintf(browser.dir, "%s/%s",browser.dir, browserList[browser.selIndex].filename);
		return 1;
	}
	return -1;
}

/****************************************************************************
 * FileSortCallback
 *
 * Quick sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/
int FileSortCallback(const void *f1, const void *f2)
{
	/* Special case for implicit directories */
	if(((BROWSERENTRY *)f1)->filename[0] == '.' || ((BROWSERENTRY *)f2)->filename[0] == '.')
	{
		if(strcmp(((BROWSERENTRY *)f1)->filename, ".") == 0) { return -1; }
		if(strcmp(((BROWSERENTRY *)f2)->filename, ".") == 0) { return 1; }
		if(strcmp(((BROWSERENTRY *)f1)->filename, "..") == 0) { return -1; }
		if(strcmp(((BROWSERENTRY *)f2)->filename, "..") == 0) { return 1; }
	}

	/* If one is a file and one is a directory the directory is first. */
	if(((BROWSERENTRY *)f1)->isdir && !(((BROWSERENTRY *)f2)->isdir)) return -1;
	if(!(((BROWSERENTRY *)f1)->isdir) && ((BROWSERENTRY *)f2)->isdir) return 1;

	return stricmp(((BROWSERENTRY *)f1)->filename, ((BROWSERENTRY *)f2)->filename);
}

/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
int
ParseDirectory()
{
	int i;
	s32 fd;
	sysFSDirent entry;
	u64 read;

	//DIR_ITER *dir = NULL;
	char fulldir[ MAXPATHLEN ];
	char filename[ MAXPATHLEN ];

	// reset browser
	Reset();

	// open the directory
	snprintf( fulldir, sizeof( fulldir ), "%s%s", rootdir, browser.dir); // add currentDevice to path
	printf("browsing: \"%s\" root: \"%s\"  dir: \"%s\"\n", fulldir, rootdir, browser.dir );

	//open dir
	i = sysFsOpendir( fulldir, &fd );
	if( i )
	{
		printf("sysFsOpendir( %s ): %i\n", fulldir, i );
		browser.dir[ 0 ] = 0;
		i = sysFsOpendir( rootdir, &fd );
		if( i )
		{
			printf("sysFsOpendir( %s ): %i\n", rootdir, i );
			return -1;
		}
	}

	// index files/folders
	int entryNum = 0;

	while( !sysFsReaddir( fd, &entry , &read ) && read > 0 )
	{
		if( !strcmp( entry.d_name,".") )
			continue;

		snprintf( filename, sizeof( filename ), "%s%s/%s", rootdir, browser.dir, entry.d_name );

		sysFSStat filestat;
		if( sysFsStat( filename, &filestat ) )
			continue;

		if( ( !strcmp( entry.d_name, ".." ) && !strlen( browser.dir ) )//dont add dotdot if this is the lower allowed directory
			|| ( ( filestat.st_mode & S_IFDIR ) && !( browser.mode & DIR_DIRS ) )//we dont want to see folders, so skip them
			|| ( !( filestat.st_mode & S_IFDIR ) && !( browser.mode & DIR_FILES ) ))
		{
			continue;
		}

		BROWSERENTRY * newBrowserList = (BROWSERENTRY *)realloc( browserList, ( entryNum + 1 ) * sizeof(BROWSERENTRY));

		if( !newBrowserList ) // failed to allocate required memory
		{
			Reset();
			entryNum = -1;
			break;
		}
		else
		{
			browserList = newBrowserList;
		}
		memset( &( browserList[ entryNum ] ), 0, sizeof(BROWSERENTRY) ); // clear the new entry

		strncpy( browserList[ entryNum ].filename, entry.d_name, MAXJOLIET );

		if( strcmp( entry.d_name,"..") == 0 )
		{
			sprintf( browserList[ entryNum ].displayname, "Up One Level" );
		}
		else
		{
			strncpy(browserList[entryNum].displayname, entry.d_name, MAXDISPLAY);	// crop name for display
		}

		browserList[ entryNum ].length = filestat.st_size;
		browserList[ entryNum ].isdir = (filestat.st_mode & S_IFDIR ) == 0 ? 0 : 1; // flag this as a dir

		entryNum++;
	}

	// close directory
	sysFsClosedir( fd );

	// Sort the file list
	qsort(browserList, entryNum, sizeof(BROWSERENTRY), FileSortCallback);

	browser.numEntries = entryNum;
	return entryNum;
}

//set the display mode ( files, folders, or both )
void SetDisplayMode( u8 mode )
{
	if( browser.mode == mode )
		return;
	browser.mode = mode;
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
int ChangeFolder()
{
	if(!UpdateDirName())
		return -1;

	ParseDirectory();

	return browser.numEntries;
}

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected device
 ***************************************************************************/
int BrowseDevice( const std::string &lowestLevel, std::string dir )
{
	memset( browser.dir, 0, sizeof( browser.dir ) );
	memset( rootdir, 0, sizeof( rootdir ) );

	//dir doesnt start with a "/", that is the root directory
	while( dir.size() && dir[ 0 ] == '/' )
		dir.erase( 0, 1 );

	strncpy( browser.dir, dir.c_str(), sizeof( browser.dir ) );
	strncpy( rootdir, lowestLevel.c_str(), sizeof( rootdir ) );

	//make sure rootdir ends with a '/' since this character is used not only for a delimiter, but also the FS root name
	//it just makes things easier
	int len = strlen( rootdir );
	if( rootdir[ len - 1 ] != '/' )
	{
		rootdir[ len ] = '/';
		rootdir[ len + 1 ] = 0;
	}

	//dir doesnt end with a "/"
	len = strlen( browser.dir );
	while( browser.dir[ len - 1 ] == '/' )
	{
		browser.dir[ len - 1 ] = 0;
		len--;
	}

	ParseDirectory(); // Parse root directory
	return browser.numEntries;
}

}
