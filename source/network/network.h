#ifndef NETWORK_H
#define NETWORK_H


#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <ppu-types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/socket.h>
#include <net/net.h>

#include "../buffer.h"

using namespace std;


// class to handle downloading a file from the internet
//! this is extremely unstable :(
//! the parseurl function is very basic and only attempts to split an url like "http://stuff.com/porn.html"
//! into "stuff.com" and "/porn.html".  the download alse fails without a path in urls like "http:// www.google.com/"
//! and it makes the program crash

//! if it works right, yo can create an objuct with an URL.  and then use Download() to try and downloda the file.
//! it will either work, give an empty buffer on error, or flat out crash the program
class NetworkRequest
{
public:
	NetworkRequest( const string &address );

	//downloads the file
	Buffer Download();

private:
	u32 ip;
	string domain;
	string path;
	string url;

	bool ParseUrl();
	Buffer &DownloadInternal( const string &str, Buffer &buf );
};



namespace Network
{
bool Init();
void DeInit();


//function to display a progress bar and download files
//! urls is a list of the URLs to download
//! destinations is a list of destination paths to write the downloaded files to
//! returns the number of file successfully downloaded & written
//! it overwrites any of the destination files without asking
//! expects that the gui thread is already running to draw it
u32 DownloadUrls( const vector<string> &urls, const vector<string> &destinations );

}

#endif // NETWORK_H
