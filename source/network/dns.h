#ifndef DNS_H
#define DNS_H


#include <fcntl.h>
#include <string>

#include <ppu-types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/socket.h>
#include <net/net.h>

//gets an IP address and caches the address and  domain so it doesnt need to be looked up over and over
// copy/pasted from the wii libs
using namespace std;
namespace Dns
{
u32 GetIpByNameCached( const string &domain );
}

#endif // DNS_H
