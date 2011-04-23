

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sysutil/msg.h>

#include "dns.h"
#include "../fileops.h"
#include "network.h"
#include "../stringstuff.h"


namespace Network
{
bool inited = false;
bool Init()
{
	if( inited )
		return true;
	if( netInitialize() )
	{
		printf("netInitialize() failed\n");
		return false;
	}
	inited = true;
	return true;
}

void DeInit()
{
	if( !inited )
		return;
	netDeinitialize();
	inited = false;
}

static void DialogHandler( msgButton button,void *usrData )
{
	vs32 *dialog_action = (vs32 *)usrData;
	switch(button) {
		case MSG_DIALOG_BTN_OK:
			*dialog_action = 1;
			break;
		case MSG_DIALOG_BTN_NO:
		case MSG_DIALOG_BTN_ESCAPE:
			*dialog_action = 2;
			break;
		case MSG_DIALOG_BTN_NONE:
			*dialog_action = -1;
			break;
		default:
			break;
	}
}

u32 DownloadUrls( const vector<string> &urls, const vector<string> &destinations )
{
	u32 ret = 0;
	if( !urls.size() || urls.size() != destinations.size() )
	{
		printf("DownloadUrls() invalid sizes\n" );
		return ret;
	}

	//this will be set by the dialog window when something is clicked
	vs32 dialog_action = 0;
	f32 percentPerFile = 100.0f/(float)urls.size();
	char message[ 1024 ];
	vector<string>::const_iterator url = urls.begin();
	vector<string>::const_iterator dest = destinations.begin();


	msgDialogOpen2( MSG_DIALOG_SINGLE_PROGRESSBAR, "Downloading files...",\
					DialogHandler, (void*)&dialog_action, NULL );

	while( !dialog_action )
	{
		if( url == urls.end() )//nothing left to do
		{
			sleep( 1 );//this sleep is just to let the progress bar show 100%
			break;
		}

		snprintf( message, sizeof( message ), (*url).c_str() );
		msgDialogProgressBarSetMsg( 0, message );

		if( !(*url).empty() && !(*dest).empty() )
		{
			NetworkRequest request( *url );
			Buffer buf = request.Download();
			if( !buf.IsEmpty() && FileOps::WriteFile( *dest, buf ) )
			{
				ret++;
			}
		}
		msgDialogProgressBarInc( 0, percentPerFile );//increment %
		++url;
		++dest;
	}
	msgDialogAbort();
	return ret;

}
}


NetworkRequest::NetworkRequest( const string &address ): ip( 0 ), url( address )
{

}

Buffer &NetworkRequest::DownloadInternal( const string &str, Buffer &ret )
{
	if( str.empty() )
		return ret;

	char buf[512];
	char* ptr;
	int sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	struct sockaddr_in servaddr;
	memset( &servaddr, 0, sizeof( servaddr ) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = ip;
	servaddr.sin_port = htons( 80 );

	if( connect( sock, (struct sockaddr *)&servaddr, sizeof( servaddr ) ) == -1 )
	{
		printf("NetworkRequest::DownloadInternal(): error creating socket\n");
		return ret;
	}
	snprintf( buf, sizeof( buf ), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", str.c_str(), domain.c_str() );
	write( sock, buf, strlen( buf ) );

	ptr = buf;
	for( ptr = buf; ptr - buf <= 4 || *((u32*)(ptr - 4)) != 0x0d0a0d0a; read( sock, ptr++, 1 ) );

	*ptr = 0;
	ptr = strstr( buf, "Location:" );
	if( ptr )
	{
		ptr += 9;
		while( *ptr == ' ' )
			ptr++;

		if( strchr( ptr, '\r' ) )
			*strchr( ptr, '\r' ) = 0;
		if( strchr( ptr, '\n' ) )
			*strchr( ptr, '\n' ) = 0;

		close( sock );
		return DownloadInternal( ptr, ret );
	}
	ptr = strstr( buf, "Content-Length:" );
	if( !ptr )
	{
		printf("response: \"%s\"\n", buf );
		close( sock );
		printf("NetworkRequest::DownloadInternal(): unknown content length\n");
		return ret;
	}
	ptr += strlen( "Content-Length: " );
	s32 sz = atoi( ptr );
	if( !sz )
	{
		printf("response: \"%s\"\n", buf );
		close( sock );
		printf("NetworkRequest::DownloadInternal(): zero length file\n");
		return ret;
	}
	ret.Resize( sz );
	if( !ret.Size() )//out of memory
	{
		close( sock );
		printf("\"%s\"\n", buf );
		printf("NetworkRequest::DownloadInternal(): ENOMEM\n");
		return ret;
	}

	ssize_t rsz = 0;
	for( ptr = (char*)ret.Data(); sz > 0; ptr += rsz )
	{
		rsz = read( sock, ptr, sz );
		sz -= rsz;
		if( rsz <= 0 )
			break;
	}
	close( sock );

	if( sz != 0 )
	{
		printf("NetworkRequest::DownloadInternal(): downloaded size is not what was expected\n");
		ret.Free();
	}

	return ret;
}

Buffer NetworkRequest::Download()
{
	Buffer ret;
	if( !ParseUrl() )
	{
		printf("NetworkRequest::Download() parse url failed\n");
		return ret;
	}
	//ret = DownloadInternal( path );		// there is some tom-foolery where the destructor for Buffer()
	//printf("returned to Download()\n");	// is crashing before this function returns.  but it is not
	//return ret;							// any part of my code in the destructor that is crashing, but after it :(
											// i think the socket stuff is dostroying the stack or something?
	return DownloadInternal( path, ret );
}

bool NetworkRequest::ParseUrl()
{
	if( !Network::inited || !StartsWith( url, "http://" ) )
		return false;

	url.erase( 0, 7 );

	size_t slash = url.find( "/" );
	if( slash == string::npos )
	{
		domain = url;
		path = "/";
	}
	else
	{
		domain = url;
		domain.resize( slash );
		path = url;
		path.erase( 0, slash );
	}

	if( path.empty() || domain.empty() || domain.find( "." ) == string::npos || path.find( "/" ) != 0 )
		return false;
	if( path.size() == 1 )//trying to make it not crash :(
		path = "/index.html";

	ip = Dns::GetIpByNameCached( domain );
	if( !ip )
	{
		cout << "error resolving " << domain << " to IP for \"" << url << "\"" << endl;
		return false;
	}
	cout << "url: " << url << endl;
	cout << "domain: " << domain << endl;
	cout << "path: " << path << endl;

	return true;
}

