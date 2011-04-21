

#include <malloc.h>
//#include <psl1ght/lv2.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "fileops.h"
//#include "payloadGrooveHermes.bin.h"
#include "sfo.h"
#include "utils.h"
#include "warez.h"

#ifdef WAREZZZ
#error "this code doesnt work since being converted to psl1ght v2.  the syscalls are all borked still"

INC_FILE( payloadGrooveHermes_bin );

namespace Warez
{

int SendPayload()
{
	u64 v = lv2peek( 0x8000000000017CD0ULL );

	if( v != 0x4E8000203C608001ULL )
		return 1; // if syscall 8 is modified payload is resident...

	if( payloadGrooveHermes_bin_size > 0xfff)
		return 2; // payload too big...

	/* i repeat 25 times the patch because it is possible fail patching (cache instruction problem? maybe only in syscall 9 patch
	because the call of syscall 7) */

	for( u8 l = 0; l < 25; l++ )
	{
		u8 * p = (u8 *)&payloadGrooveHermes_bin[0];
		for( u16 n = 0; n < payloadGrooveHermes_bin_size ; n += 8 )
		{
			static u64 value;
			memcpy( &value, &p[ n ], 8 );
			lv2poke( 0x80000000007e0000ULL + (u64) n, value );

			__asm__("sync");
			value = lv2peek( 0x8000000000000000ULL );
		}

		// syscall 9 enable
		lv2poke( 0x8000000000017CE0ULL , 0x7C6903A64E800420ULL );
		__asm__("sync");
	}
	return 0;
}

bool Init()
{
	int flag = SendPayload();
	if( !flag )
	{
		lv2launch( 0x80000000007e0000ULL );
		__asm__( "sync" );
		sleep( 1 );
	}

	if( sys8_enable( 0 ) < 0 && sys8_enable( SYSCALL_8_LOCK ) < 0 )
	{
		printf("syscall 8 failed\n");
		return false;
	}
	usleep( 250000 );

	sys8_perm_mode( 2 );
	sys8_path_table( 0LL );

	if( flag == 1 && sys8_enable( 0 ) < 0x102 )
	{
		printf("payload version is too low\n");
		return false;
	}

	syscall36( "/dev_bdvd" );
	sys8_perm_mode( (u64)2 );


	return true;

}

void unpatch_bdvdemu()
{
	char* mem = (char*)memalign( 8, 0xff0 );
	if( !mem )
	{
		printf("unpatch_bdvdemu(): failed to allocate memory\n");
		return;
	}

	sys8_memcpy( (u64) mem, LV2MOUNTADDR_341, 0xff0ULL );

	for( int n = 0; n < 0xff0; n+= 0x100 )
	{
		if( !memcmp( mem + n, "CELL_FS_IOS:PATA0_BDVD_DRIVE", 29 ) )
		{
			if( !memcmp( mem + n + 0x69, "temp_bdvd", 10 ) )
				sys8_memcpy( LV2MOUNTADDR_341 + n + 0x69, (u64)"dev_bdvd\0", 10ULL );
		}

		if( !memcmp( mem + n, "CELL_FS_IOS:USB_MASS_STORAGE0", 29 ) )
		{
			if( !memcmp( mem + n + 0x69, "dev_bdvd", 9 ) )
			{
				sys8_memcpy( LV2MOUNTADDR_341 + n + 0x69, (u64)(mem + n + 0x79), 11ULL );
				sys8_memset( LV2MOUNTADDR_341 + n + 0x79, 0ULL, 12ULL );
			}
		}
	}
	free( mem );
}

void TestSfoShit( const string &path )
{
	//testing 1, 2
	Sfo sfo( path + "PS3_GAME/PARAM.SFO" );
	if( sfo.Count() )
	{
		sfo.SetValue( "TITLE", path );
		Buffer buf = sfo.Data();
		if( !buf.IsEmpty() )
		{
			string home = FileOps::HomeDir();
			if( !home.empty() )
			{
				home += "/tmp/";
				if( FileOps::Exists( home ) || !sysFsMkdir( home.c_str(), 0755 ) )
				{
					if( FileOps::WriteFile( home + "PARAM.SFO", buf.Data(), buf.Size() ) )
					{
						RedirectPaths( home + "PARAM.SFO", "/dev_bdvd/PS3_GAME/PARAM.SFO" );
					}
					else
						printf( "12\n" );
				}
				else
					printf( "13\n" );
			}
			else
				printf( "14\n" );
		}
		else
			printf( "15\n" );
	}
}

bool LoadGame( const std::string &path, const std::string &ebootPath )
{
	printf("LoadGame( \"%s\", \"%s\" )\n", path.c_str(), ebootPath.c_str() );
	int i = 0;
	if( ebootPath.empty() )
	{
		i = sys8_path_table( 0LL );
		if( i )
		{
			printf( "LoadGame( %s ): sys8_path_table( 0LL ):%i\n", path.c_str(), i );
			goto error;
		}
	}
	else
	{
		printf( "LoadGame(): external eboot not implemented\n" );
		return false;
	}
	i = sys8_sys_configure( CFG_XMB_DEBUG );
	if( i )
	{
		printf( "LoadGame( %s ): sys8_sys_configure( CFG_XMB_DEBUG ):%i\n", path.c_str(), i );
		goto error;
	}
	i = sys8_sys_configure( CFG_UNPATCH_APPVER );
	if( i )
	{
		printf( "LoadGame( %s ): sys8_sys_configure( CFG_UNPATCH_APPVER ):%i\n", path.c_str(), i );
		goto error;
	}
	i = syscall36( path.c_str() );
	if( i )
	{
		printf( "LoadGame( %s ): syscall36():%i\n", path.c_str(), i );
		goto error;
	}
	//sys8_perm_mode((u64) (game_cfg.perm & 3));
	i = sys8_perm_mode((u64)0);
	if( i )
	{
		printf( "LoadGame( %s ): sys8_perm_mode((u64)0)%i\n", path.c_str(), i );
		goto error;
	}

	TestSfoShit( path );



	return true;

error:
	syscall36( "/dev_bdvd" );
	sys8_perm_mode( (u64)1 );
	return false;
}

typedef struct
{
	path_open_entry entries[ 4 ];

	char arena[ 0x2000 ];
} path_open_table;


void RedirectPaths( const string &src1, const string &dst1,
					const string &src2, const string &dst2,
					const string &src3, const string &dst3 )
{

	if( !src1.size() || src1.size() >= 0x100 || !dst1.size() || dst1.size() >= 0x800 )
	{
		printf( "RedirectPaths() invalid path: \"%s\" \"%s\"\n", src1.c_str(), dst1.c_str() );
		return;
	}

	path_open_table open_table;
	u64 dest_table_addr;
	u8 numFiles = 1;

	//count number of files to replace
	if( src2.size() && src2.size() < 0x100 && dst2.size() && dst2.size() < 0x800 )
	{
		numFiles = 2;
		if( src3.size() && src3.size() < 0x100 && dst3.size() && dst3.size() < 0x800 )
		{
			numFiles = 3;
		}
	}
	memset( &open_table.arena, 0, sizeof( open_table.arena ) );


	printf("last table %llx\n", (long long unsigned int)sys8_path_table(0ULL) );

	// calculate dest_table addr from payload start to back
	dest_table_addr = 0x80000000007FF000ULL - ( ( sizeof( path_open_table ) + 15 ) & ~15 );

	// fix the start addresses
	open_table.entries[ 0 ].compare_addr = ((u64) &open_table.arena[ 0 ]) - ((u64) &open_table) + dest_table_addr;
	open_table.entries[ 0 ].replace_addr = ((u64) &open_table.arena[ 0x800 ])- ((u64) &open_table) + dest_table_addr;

	if( numFiles > 1 )
	{
		open_table.entries[ 1 ].compare_addr = ((u64) &open_table.arena[ 0x100 ]) - ((u64) &open_table) + dest_table_addr;
		open_table.entries[ 1 ].replace_addr = ((u64) &open_table.arena[ 0x1000 ])- ((u64) &open_table) + dest_table_addr;

		if( numFiles > 2 )
		{
			open_table.entries[ 2 ].compare_addr = ((u64) &open_table.arena[ 0x200 ]) - ((u64) &open_table) + dest_table_addr;
			open_table.entries[ 2 ].replace_addr = ((u64) &open_table.arena[ 0x1800 ])- ((u64) &open_table) + dest_table_addr;
		}
		else
		{
			open_table.entries[ 2 ].compare_addr = 0ULL; // the last entry always 0
		}
	}
	else
	{
		open_table.entries[ 1 ].compare_addr = 0ULL; // the last entry always 0
	}

	open_table.entries[ 3 ].compare_addr = 0ULL; // the last entry always 0

	// copy the paths
	strncpy( &open_table.arena[0], dst1.c_str(), 0x100 );           // compare 1
	strncpy( &open_table.arena[0x800], src1.c_str(), 0x800 );     // replace 1

	if( numFiles > 1 )
	{
		strncpy(&open_table.arena[0x100], dst2.c_str(), 0x100 );	 // compare 2
		strncpy(&open_table.arena[0x1000], src2.c_str(), 0x800 ); // replace 2

		if( numFiles > 2 )
		{
			strncpy(&open_table.arena[0x200], dst3.c_str(), 0x100 );          // compare 3
			strncpy(&open_table.arena[0x1800], src3.c_str(), 0x800 ); // replace 3
		}
	}

	open_table.entries[ 0 ].compare_len = dst1.size();		// 1
	open_table.entries[ 0 ].replace_len = src1.size();

	open_table.entries[ 1 ].compare_len = dst2.size();	// 2
	open_table.entries[ 1 ].replace_len = src2.size();

	open_table.entries[ 2 ].compare_len = dst3.size();    // 3
	open_table.entries[ 2 ].replace_len = src3.size();

	// copy the datas to  the destination address
	sys8_memcpy( dest_table_addr, (u64) &open_table, sizeof( path_open_table ) );

	// set the path table
	sys8_path_table( dest_table_addr );
}

}

u64 lv2peek(u64 addr)
{
	lv2syscall1(6, (u64) addr);
	return_to_user_prog ( u64 );
}

u64 lv2poke(u64 addr, u64 value)
{
	lv2syscall2(7, (u64) addr, (u64) value);
	return_to_user_prog ( u64 );
}

int lv2launch(u64 addr)
{
	lv2syscall8(9, (u64) addr, 0,0,0,0,0,0,0);
	return_to_user_prog ( int );
}

int syscall36( const char * path )
{
	lv2syscall1(36, (u64) path);
	return_to_user_prog ( int );
}

/* syscall8.c

Copyright (c) 2010 Hermes <www.elotrolado.net>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of
  conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this list
  of conditions and the following disclaimer in the documentation and/or other
  materials provided with the distribution.
- The names of the contributors may not be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

//#include "syscall8.h"

static u64 syscall8(register u64 cmd, register u64 param1, register  u64 param2, register  u64 param3)
{
	lv2syscall4(8, cmd, param1, param2, param3);
	return_to_user_prog ( u64 );
}

int sys8_disable(u64 key)
{

	return (int) syscall8(0ULL, key, 0ULL, 0ULL);
}

int sys8_enable(u64 key)
{

	return (int) syscall8(1ULL, key, 0ULL, 0ULL);
}

u64 sys8_memcpy(u64 dst, u64 src, u64 size)
{

	return syscall8(2ULL, dst, src, size);

}

u64 sys8_memset(u64 dst, u64 val, u64 size)
{

	return syscall8(3ULL, dst, val, size);

}

u64 sys8_call(u64 addr, u64 param1, u64 param2)
{

	return syscall8(4ULL, addr, param1, param2);

}

u64 sys8_alloc(u64 size, u64 pool)
{

	return syscall8(5ULL, size, pool, 0ULL);

}

u64 sys8_free(u64 addr, u64 pool)
{

	return syscall8(6ULL, addr, pool, 0ULL);

}

void sys8_panic(void)
{

	syscall8(7ULL, 0ULL, 0ULL, 0ULL);

}

int sys8_perm_mode(u64 mode)
{

	return (int) syscall8(8ULL, mode, 0ULL, 0ULL);
}

int sys8_sys_configure(u64 mode)
{

	return (int) syscall8(10ULL, mode, 0ULL, 0ULL);
}

int sys8_lv1_syscall(lv1_reg *in, lv1_reg *out)
{

	return (int) syscall8(11ULL, (u64) in, (u64) out, 0ULL);
}

u64 sys8_path_table(u64 addr_table)
{

	return syscall8(9ULL, addr_table, 0ULL, 0ULL);
}

#endif //#ifdef WAREZZZ
