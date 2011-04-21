
#include <sys/systime.h>
#include <cstdlib>
#include <iostream>
#include <strings.h>

#include "devicethread.h"
#include "gameinstalldialog.h"

#include "settings.h"
#include "stringstuff.h"

using namespace std;
extern GuiFont *font;
GameInstallDialog::GameInstallDialog( int w, int h )
{
	height = h;
	width = w;
	xoffset = 0;
	yoffset = 0;
	alignment = ALIGN_CENTER | ALIGN_MIDDLE;

	currentFileSize = 0;
	currentFileDone = 0;
	totalSize = 0;
	totalDone = 0;
	progress = 0;

	copyBuffer = NULL;

	mode = ModeCheckGame;

	statusTxt = new GuiText( font, (char*)NULL, 20, GUI_TEXT_COLOR );
	statusTxt->SetMaxWidth( WINDOW_WIDTH - 50 );
	statusTxt->SetPosition( 20, -80 );
	statusTxt->SetAlignment( ALIGN_BOTTOM | ALIGN_LEFT );
	statusTxt->SetParent( this );

	statusTxt2 = new GuiText( font, (char*)NULL, 14, GUI_TEXT_COLOR );
	statusTxt2->SetMaxWidth( WINDOW_WIDTH - 50 );
	statusTxt2->SetPosition( 10, -50 );
	statusTxt2->SetAlignment( ALIGN_BOTTOM | ALIGN_LEFT );
	statusTxt2->SetParent( this );
}

GameInstallDialog::~GameInstallDialog()
{
	if( gameImgData1 )
		delete gameImgData1;
	if( gameImgData2 )
		delete gameImgData2;
	if( gameImg1 )
		delete gameImg1;
	if( gameImg2 )
		delete gameImg2;

	if( copyBuffer )
		free( copyBuffer );

	delete statusTxt;
	delete statusTxt2;


	sysFsClose( fdIn );
	sysFsClose( fdOut );

}

void GameInstallDialog::LoadGameBanner()
{
	u32 size;
	u8 * mem;
	string path = "/dev_bdvd/PS3_GAME/PIC0.PNG";

	gameImgData1 = NULL;
	gameImgData2 = NULL;
	gameImg1 = NULL;
	gameImg2 = NULL;


	Status( 0, "Checking /dev_bdvd..." );
	Game game( "/dev_bdvd" );
	if( game.IsOk() )
	{
		string path = Settings::installDir;
		if( path.at( path.size() - 1 ) != '/' )
			path += "/";

		//convert game title to ansi (get rid of (r), (c), ...)
		//the convert to FAT name (get rid of ';', ':', ... )
		destPath = path + ToFatName( Utf8ToAnsi( game.Name() ) ) + "/";
		mode = ModeConfirmStart;
		title = game.Name();
	}
	else
	{
		Status( 1, "Error reading game info!" );
		mode = ModeError;
		return;
	}

	mem = FileOps::ReadFile( path, &size );
	if( mem )
	{
		gameImgData1 = new GuiImageData( mem, size );
		gameImg1 = new GuiImage( gameImgData1 );
		gameImg1->SetScale( 0.4416666f );//window width/1920

		//this image should either be 1000px wide or 1920
		if( gameImgData1->GetWidth() <= 1000 )
			gameImg1->SetPosition( 124, 69 );//these values seem to work for all my guitar hero & rockband games
		else
			gameImg1->SetPosition( 0, 0 );//i dont have any games that fit this category, but i assume this is right

		gameImg1->SetAlignment( ALIGN_CENTER | ALIGN_MIDDLE );
		gameImg1->SetParent( this );
	}

	path = "/dev_bdvd/PS3_GAME/PIC1.PNG";
	mem = FileOps::ReadFile( path, &size );
	if( mem )
	{
		gameImgData2 = new GuiImageData( mem, size );
		gameImg2 = new GuiImage( gameImgData2 );
		gameImg2->SetScale( 0.4416666f );//window width/1920
		gameImg2->SetPosition( 0, 0 );
		gameImg2->SetAlignment( ALIGN_CENTER | ALIGN_MIDDLE );
		gameImg2->SetParent( this );
	}

}

void GameInstallDialog::Draw()
{
	if( gameImg2 )
		gameImg2->Draw();
	if( gameImg1 )
		gameImg1->Draw();

	statusTxt->Draw();
	statusTxt2->Draw();

	GuiWindow::Draw();
}

bool GameInstallDialog::GeneratePathList( const std::string &parent )
{
	int i;
	s32 fd;
	sysFSDirent entry;
	u64 read;

	//make folder
	std::string str = parent + "/";
	std::string newLoc = str;
	newLoc.replace( 0, 10, destPath );
	if( sysFsMkdir( newLoc.c_str(), 0755 ) )
	{
		printf("failed to make directory \"%s\"\n", newLoc.c_str() );
		return false;
	}

	//open dir
	i = sysFsOpendir( parent.c_str(), &fd );
	if( i )
	{
		printf("sysFsOpendir( %s ): %i\n", parent.c_str(), i );
		return false;
	}

	while( !sysFsReaddir( fd, &entry , &read ) && read > 0 )
	{
		//printf("namelen: %i  type: %u  name: %s\n", entry.d_namlen, entry.d_type, entry.d_name );
		if( !entry.d_namlen || !strcmp( entry.d_name, "." ) || !strcmp( entry.d_name, ".." ) )
			continue;

		std::string str = parent + "/";
		str += entry.d_name;
		std::string newLoc = str;
		newLoc.replace( 0, 10, destPath );
		//cout << "str: \"" << str << "\"" << endl;
		//cout << "new: \"" << newLoc << "\"" << endl;

		//sub dirs
		if( entry.d_type == 1 )
		{
			if( !GeneratePathList( str ) )
			{
				sysFsClosedir( fd );
				return false;
			}
			continue;
		}
		//files
		if( entry.d_type == 2 )
		{
			entryList.push_back( str );
			continue;
		}
		printf("unhandled stat type: %i in \"%s\"\n", entry.d_type, str.c_str() );
		sysFsClosedir( fd );
		return false;
	}

	//close dir
	sysFsClosedir( fd );

	return true;
}

int GameInstallDialog::Run()
{
	int ret = CopyOk;
	switch( mode )
	{
	case ModeCheckGame:
		LoadGameBanner();
		break;
	case ModeConfirmStart:
		{
			copyBuffer = (u8*)malloc( GAME_COPY_BLOCK_SIZE );
			if( !copyBuffer )
			{
				Status( 0, "failed to create buffer to copy files" );
				ret = CopyFailed;
				break;
			}
			totalDone = 0;
			Status( 0, "Checking game size..." );
			totalSize = FileOps::FileSize( "/dev_bdvd" );
			if( !totalSize )
			{
				ErrorPrompt( "Theres nothing to copy!" );
				ret = CopyFailed;
				break;
			}
			Status( 0, "Checking free space..." );
			float totalGiB = (float)totalSize/(float)GiB;
			float freeGiB = FileOps::FreeSpace( destPath );
			if( totalGiB >= freeGiB )
			{
				ErrorPrompt( "Not enough free space to copy the game!" );
				ret = CopyFailed;
				break;
			}
			char message[ 0x100 ];
			snprintf( message, sizeof( message ), "Do you want to install \"%s\" (%.2f GiB) to \"%s\" (%.2f GiB free)?", \
					  title.c_str(), totalGiB, destPath.c_str(), freeGiB );
			if( !WindowPrompt( "Install?", message, "Yes", "No" ) )
			{
				ret = CopyCanceled;
				break;
			}
			if( !FileOps::Exists( Settings::installDir ) && sysFsMkdir( Settings::installDir.c_str(), 0755 ) )
			{
				snprintf( message, sizeof( message ), "Failed to make \"%s\"", \
						  Settings::installDir.c_str() );
				ErrorPrompt( message );
				ret = CopyFailed;
				break;
			}
			if( FileOps::Exists( destPath ) )
			{
				int r = WindowPrompt( "The directory already exists", "Delete that shit?", "Yes", "No" );
				if( !r )
				{
					ret = CopyCanceled;
					break;
				}
				GameList::RemoveGame( destPath );
				Status( 0, "Deleting old files..." );
				if( !FileOps::RecurseDeletePath( destPath ) )
				{
					ErrorPrompt( "Failed to delete that shit!" );
					ret = CopyFailed;
					break;
				}
				printf("deleted \"%s\"\n", destPath.c_str() );
			}
			mode = ModeReadFiles;
			ret = CopyStarting;
		}
		break;
	case ModeReadFiles:
		{
			Status( 0, "Creating folders" );
			Status( 1, "and listing files to copy..." );
			if( !GeneratePathList() )
			{
				ret = CopyFailed;
				break;
			}
			sysGetCurrentTime( &startTime, &null );
			mode = ModeCopying;
		}
		break;
	case ModeCopying:
		{
			if( !currentFileSize )
			{
				//testing 1, 2
				/*if( progress >= 1 )
				{
					mode = ModeDone;
					break;
				}*/
				if( !entryList.size() )
				{
					Status( 0, "Done!" );
					//ret = CopyDone;
					mode = ModeDone;
					free( copyBuffer );
					copyBuffer = NULL;
					break;
				}
				if( !StartNextFile() || !CopyBlock() )
				{
					ret =  CopyFailed;
					break;
				}
			}
			else if( !CopyBlock() )
			{
				ret =  CopyFailed;
				break;
			}

			u64 now;
			sysGetCurrentTime( &now, &null );
			if( lastTime != now )
			{
				float seconds = now - startTime;
				float speed = ( (float)totalDone/seconds );
				speed /= 1024;
				if( speed < 1024 )
				{
					Status( 0, "%.2f %%    -    %.2f KiB", progress, speed );
					//printf("speed: %.2f KiB/s\n", speed );
				}
				else
				{
					speed /= 1024;
					if( speed < 1024 )
					{
						//printf("speed: %.2f MiB/s\n", speed );
						Status( 0, "%.2f %%    -    %.2f MiB/s", progress, speed );
					}
					else
					{
						speed /= 1024;
						//printf("speed: %.2f GiB/s\n", speed );
						Status( 0, "%.2f %%    -    %.2f GiB/s", progress, speed );
					}
				}

				lastTime = now;
			}
		}
		break;
	case ModeDone:
		{
			u64 now;
			sysGetCurrentTime( &now, &null );
			u64 secs = now - startTime;
			u32 hours = secs/3600;
			if( hours )
				secs -= ( hours * 3600 );
			u32 mins = secs/60;
			if( mins )
				secs -= ( mins * 60 );

			float gib = (float) totalDone/(float)GiB;
			printf("copied %.2f GiB in %02u:%02u:%02u\n", gib, hours, mins, (u32)secs );
			ret = CopyDone;

			//add it to the list
			Game game( destPath );
			if( game.IsOk() )
				GameList::AddGame( game );
		}
		break;
	case ModeError:
		Status( 0, "Error!" );
		break;
	case ModeCancel:
		Status( 0, "Canceled!" );
		break;
	}

	return ret;
}

bool GameInstallDialog::StartNextFile()
{
	currentFileSize = 0;
	currentFileDone = 0;
	string path = entryList.at( 0 );
	string newLoc = path;
	newLoc.replace( 0, 10, destPath );

	Status( 1, "%s", path.c_str() );
	sysFSStat stat;
	int i = sysFsStat( path.c_str(), &stat );
	if( i < 0 || ( stat.st_mode & S_IFDIR ) )
	{
		printf("sysFsStat( %s ): %i\n", path.c_str(), i );
		return false;
	}
	currentFileSize = stat.st_size;

	//open source file
	i = sysFsOpen( path.c_str(), SYS_O_RDONLY, &fdIn, NULL, 0 );
	if( i )
	{
		printf("sysFsOpen( %s ) %i\n", path.c_str(), i );
		return false;
	}

	//open destination file
	i = sysFsOpen( newLoc.c_str(), SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fdOut, NULL, 0 );
	if( i )
	{
		printf("sysFsOpen( %s ): %i\n", newLoc.c_str(), i );
		sysFsClose( fdIn );
		return false;
	}
	return true;

}

bool GameInstallDialog::CopyBlock()
{
	int i;
	u64 read;
	u32 toCopy = MIN( GAME_COPY_BLOCK_SIZE, ( currentFileSize - currentFileDone ) );
	if( !toCopy )
		goto happyEnding;

	//read chunk
	i = sysFsRead( fdIn, (void*)copyBuffer, toCopy, &read );
	if( i || read != toCopy )
	{
		printf("sysFsRead( %u ): %i ( %u )\n", toCopy, i, (unsigned int)read );
		goto sadEnding;
	}

	//write chunk
	i = sysFsWrite( fdOut, (void*)copyBuffer, toCopy, &read );
	if( i )
	{
		printf("sysFsWrite( %s ): %i\n", entryList.at( 0 ).c_str(), i );
		goto sadEnding;
	}

	currentFileDone += toCopy;
	totalDone += toCopy;
	progress = ( (float)totalDone / (float)totalSize ) * 100.0f;
	//printf("progress: %.2f\n", progress );
	if( currentFileSize == currentFileDone )
		goto happyEnding;

	return true;

	//current file is done
happyEnding:
	sysFsClose( fdIn );
	sysFsClose( fdOut );
	if( entryList.size() )
		entryList.erase( entryList.begin() );
	currentFileSize = 0;
	currentFileDone = 0;
	return true;

sadEnding:
	sysFsClose( fdIn );
	sysFsClose( fdOut );
	return false;
}

void GameInstallDialog::Status( int t, const char *format, ... )
{
	GuiText *txt = NULL;
	if( !t )
	{
		txt = statusTxt;
	}
	else if( t == 1 )
	{
		txt = statusTxt2;
	}
	else
	{
		return;
	}
	if( !format )
	{
		HaltGui();
		txt->SetText((char *) NULL);
		ResumeGui();
		return;
	}

	char *tmp = 0;
	va_list va;
	va_start( va, format );
	if( ( vasprintf( &tmp, format, va ) >= 0 ) && tmp )
	{
		HaltGui();
		txt->SetText(tmp);
		ResumeGui();
	}
	va_end( va );
	if( tmp )
		free( tmp );
}

void GameInstallDialog::Cancel( bool deleteFiles )
{
	sysFsClose( fdIn );
	sysFsClose( fdOut );
	if( copyBuffer )
	{
		free( copyBuffer );
		copyBuffer = NULL;
	}
	mode = ModeCancel;
	if( deleteFiles && !destPath.empty() && FileOps::Exists( destPath ) )
	{
		Status( 0, "Deleting written files..." );
		Status( 1, NULL );
		FileOps::RecurseDeletePath( destPath );
	}
}

