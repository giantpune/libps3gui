/* 
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/

#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char ascii( char s ) {
	if ( s < 0x20 ) return '.';
	if ( s > 0x7E ) return '.';
	return s;
}

void hexdump( const void *d, int len ) {
	unsigned char *data;
	int i, off;
	data = (unsigned char*)d;
	printf(  "\n");
	for ( off = 0; off < len; off += 16 ) {
		printf(  "%08x  ", off );
		for ( i=0; i<16; i++ )
		{
			if( ( i + 1 ) % 4 )
			{
				if ( ( i + off ) >= len ) printf( "  ");
				else printf( "%02x",data[ off + i ]);
			}
			else
			{
				if ( ( i + off ) >= len ) printf( "   ");
				else printf( "%02x ",data[ off + i ]);
			}
		}

		printf(  " " );
		for ( i = 0; i < 16; i++ )
			if ( ( i + off) >= len ) printf( " ");
		else printf( "%c", ascii( data[ off + i ]));
		printf( "\n");
	}
	fflush( stdout );
}

void mutex_init( sys_mutex_t *m )
{
	sys_mutex_attr_t attr;
	s32 r;
	memset(&attr, 0, sizeof(attr));
	attr.attr_protocol = SYS_MUTEX_PROTOCOL_PRIO;
	attr.attr_recursive = SYS_MUTEX_ATTR_NOT_RECURSIVE;
	attr.attr_pshared  = 0x00200;
	attr.attr_adaptive = 0x02000;

	strcpy(attr.name, "mutex");

	if((r = sysMutexCreate(m, &attr)) != 0) {
		printf("Failed to create mutex: error: 0x%x", r);
		exit(0);
	}
}

void cond_init( sys_cond_t *c, sys_mutex_t *m )
{
	sys_cond_attr_t attr;
	s32 r;
	memset(&attr, 0, sizeof(attr));
	attr.attr_pshared = 0x00200;
	strcpy(attr.name, "cond");
	if((r = sysCondCreate(c, *m, &attr) != 0)) {
		printf("Failed to create cond: error: 0x%x", r);
		exit(0);
	}
}

bool IsInRange( int num, int low, int high )
{
	return ( ( num >= low ) && ( num <= high ) );
}
