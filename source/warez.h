#ifndef WAREZ_H
#define WAREZ_H

#include <string>

//#define WAREZZZ
#ifdef WAREZZZ

#define SYSCALL_8_LOCK 0x1122334455667788ull
#define LV2MOUNTADDR_341 0x80000000003EE504ULL
using namespace std;
namespace Warez
{

//make sure there is a payload in memory, and that it has the correct version
bool Init();

//use syscall 8 to prepare this game for warezing
bool LoadGame( const std::string &path, const std::string &ebootPath = "" );

//redirect paths
//! clears any previously redirected ones and sets up to 3
//! im not entirely clear on this, but it doesnt a blind memcpy and IDK how much room is there to play with
//! so im leaving it at max 3 for now
//! its an obvious copy/paste from the example below
void RedirectPaths( const string &src1, const string &dst1,
					const string &src2 = "", const string &dst2 = "",
					const string &src3 = "", const string &dst3 = "" );



void unpatch_bdvdemu();



//returns 0 on succes, 1 if payload is already present, 2 if payload is too big
int SendPayload();

};

u64 lv2peek(u64 addr);
u64 lv2poke(u64 addr, u64 value);
int lv2launch(u64 addr);
int syscall36( const char * path);



/*

FUNCTIONS TO USE IN KERNEL AREA 0x8000000000000000 - 0x80000000007fffff

payload V3 is loaded at 0x80000000007ff000

*/

// lv1 registers struct
typedef struct {

	uint64_t reg3;
	uint64_t reg4;
	uint64_t reg5;
	uint64_t reg6;
	uint64_t reg7;
	uint64_t reg8;
	uint64_t reg9;
	uint64_t reg10;
	uint64_t reg11;

} lv1_reg;


// hook_open
typedef struct
{
	uint64_t compare_addr; // kernel address to compare string
	uint64_t replace_addr; // kernel address to replace string
	int compare_len;       // len of compare string
	int replace_len;       // len of replace string

} path_open_entry;


/* NOTES about path_open_entry:

compare_addr can be a string full path for files or not. The size of the string can be len+1
replacea_ddr can replace a file, files or directories. The size of the string must be replace_string + hook_open string remnant+1
(recommended 0x800 to work with directories)

// example 1: replacing a file

compare_addr: "/app_home/PS3_GAME/USRDIR/EBOOT.BIN"
replace_addr: "/dev_usb000/PS3_GAME/USRDIR/EBOOT.BIN"

compare_len= strlen(compare_addr);
replace_len= strlen(replace_addr);

// example 2: replacing files by a unique file

compare_addr: "/app_home/PS3_GAME/ICON"
replace_addr: "/dev_usb000/PS3_GAME/ICON0.PNG"

compare_len= strlen(compare_addr);
replace_len= strlen(replace_addr)+1;

// example 3: replacing directories

compare_addr: "/app_home/PS3_GAME"
replace_addr: "/dev_usb000/PS3_GAME"

compare_len= strlen(compare_addr);
replace_len= strlen(replace_addr);

*/

/*
	sys8_disable: disable syscalls 6,7, 36 and stealth syscall 8

	key: 64 bits key to enable syscalls
*/

int sys8_disable(uint64_t key);

/*
	sys8_enable: enable syscalls 6,7, 36 and 8 when

	key: 64 bits key to enable syscalls or 0 to test

	return: 0x80010003 (<0) or current version (0x100)

*/

int sys8_enable(uint64_t key);

/*
	sys8_memcpy: 64 bits address memory copy

	dst: destination addr

	src: source addr

	size: number of bytes to copy

	return: destination addr

*/

uint64_t sys8_memcpy(uint64_t dst, uint64_t src, uint64_t size);

/*
	sys8_memset: 64 bits address memory set

	dst: destination addr

	val: value to set (only the 8 bits lower)

	size: number of bytes to set

	return: destination addr

*/

uint64_t sys8_memset(uint64_t dst, uint64_t val, uint64_t size);

/*
	sys8_call: kernel 64 bits address to call

	param1: first 64 bits param (maybe IN datas addr)

	param2: second 64 bits param (maybe OUT datas addr)

	return: value returned by kernel call

*/


uint64_t  sys8_call(uint64_t addr, uint64_t param1, uint64_t param2);

/*
	sys8_alloc: kernel function to alloc memory

	size: bytes to alloc

	pool: Usually 0x27 from psjailbreak

	return: 64 bits addr allocated

*/

uint64_t  sys8_alloc(uint64_t size, uint64_t pool);

/*
	sys8_free: kernel function to free memory

	addr: 64 bits addr to free

	pool: Usually 0x27 from psjailbreak

	return: unknown 64 bits addr

*/

uint64_t  sys8_free(uint64_t addr, uint64_t pool);

/*
	sys8_panic: kernel panic function

*/

void sys8_panic(void);

/*
	sys8_perm_mode: function to changes as work 0x80000000000505d0 calls (connected with access permissions)

	mode: 0 -> PS3 perms normally, 1-> Psjailbreak by default, 2-> Special for games as F1 2010 (option by default)

	return: 0 - Ok

*/

int sys8_perm_mode(uint64_t mode);

/*
	sys8_sys_configure: function to change the XMB or releases devices redirected

	mode: 0 -> XMB retail, 1-> XMB debug, 2-> free /apps_home and /dev_usb redirections and unpatch APP_VER in PARAM.SFO
		  3 -> unpatch APP_VER in PARAM.SFO 4 -> patch APP_VER in PARAM.SFO

	return: 0 - Ok

*/

#define CFG_XMB_RETAIL          0
#define CFG_XMB_DEBUG           1
#define CFG_UNMOUNT_DEVICES     2
#define CFG_UNPATCH_APPVER      3
#define CFG_PATCH_APPVER        4



int sys8_sys_configure(uint64_t mode);

/*
	sys8_lv1_syscall: call to lv1 syscall from lv2

	in:  input lv1_reg struct registers

	out:  output lv1_reg struct registers

	return:  lv1 syscall return

*/

int sys8_lv1_syscall(lv1_reg *in, lv1_reg *out);

/*
	sys8_path_table: function to add one table for path redirections in hook_open

	addr_table: kernel 64 bits addr from one path_open_entry array, starts. The last path_open_entry.compare_addr must be 0 to stop

	return: last addr_table

*/


uint64_t sys8_path_table(uint64_t addr_table);


/*

Developers notes:

To test if syscall8 is working:

	int ret= sys8_enable(0);

	if(ret<0)
		{
		//try to put the correct key

		ret= sys8_enable(key);
		}

	if(ret<0) printf("Sorry, syscall8 unavailable\n");
	else printf("Current version %x\n", ret);

------------------------------------------------------------

To Test sys8_alloc(), sys8_free(), sys8_memcpy() and sys8_call():

	uint32_t my_fun[0x10/4]={0x38601234, 0x4e800020}; // li %r3, 0x1234, blr

	..................................................

	uint64_t memo= sys8_alloc(0x10ULL, 0x27ULL); // alloc 16 bytes

	printf("ret: %llx\n", sys8_call( memo , 0ULL, 0ULL));

	sys8_free( memo  , 0x27ULL)

------------------------------------------------------------

To test path redirections:

	// struct for 3 paths

	typedef struct
	{
		path_open_entry entries[4];

		char arena[0x2000];
	} path_open_table;

	path_open_table open_table;

	uint64_t dest_table_addr;

	..................................................

	// disable table

	printf(" last table %llx\n", sys8_path_table(0ULL));

	// calculate dest_table addr from payload start to back

	dest_table_addr= 0x80000000007FF000ULL-((sizeof(path_open_table)+15) & ~15);

	// fix the start addresses

	open_table.entries[0].compare_addr= ((uint64_t) &open_table.arena[0]) - ((uint64_t) &open_table) + dest_table_addr;
	open_table.entries[0].replace_addr= ((uint64_t) &open_table.arena[0x800])- ((uint64_t) &open_table) + dest_table_addr;

	open_table.entries[1].compare_addr= ((uint64_t) &open_table.arena[0x100]) - ((uint64_t) &open_table) + dest_table_addr;
	open_table.entries[1].replace_addr= ((uint64_t) &open_table.arena[0x1000])- ((uint64_t) &open_table) + dest_table_addr;

	open_table.entries[2].compare_addr= ((uint64_t) &open_table.arena[0x200]) - ((uint64_t) &open_table) + dest_table_addr;
	open_table.entries[2].replace_addr= ((uint64_t) &open_table.arena[0x1800])- ((uint64_t) &open_table) + dest_table_addr;

	open_table.entries[3].compare_addr= 0ULL; // the last entry always 0

	// copy the paths

	strncpy(&open_table.arena[0], "/app_home/PS3_GAME/USRDIR", 0x100);           // compare 1
	strncpy(&open_table.arena[0x800], "/dev_usb000/PS3_GAME/USRDIR", 0x800);     // replace 1: replaces all content or USRDIR

	strncpy(&open_table.arena[0x100], "/app_home/PS3_GAME/ICON0.PNG", 0x100);	 // compare 2
	strncpy(&open_table.arena[0x1000], "/dev_usb000/PS3_GAME/ICON0.PNG", 0x800); // replace 2: replace only ICON0.PNG

	strncpy(&open_table.arena[0x200], "/app_home/PS3_GAME/ICO", 0x100);          // compare 3
	strncpy(&open_table.arena[0x1800], "/dev_usb000/PS3_GAME/ICON0.PNG", 0x800); // replace 3: // replace all ICONxxxx by ICON0.PNG


	// fix the string len

	open_table.entries[0].compare_len= strlen(&open_table.arena[0]);		// 1
	open_table.entries[0].replace_len= strlen(&open_table.arena[0x800]);

	open_table.entries[1].compare_len= strlen(&open_table.arena[0x100]);	// 2
	open_table.entries[1].replace_len= strlen(&open_table.arena[0x1000]);

	open_table.entries[2].compare_len= strlen(&open_table.arena[0x200]);    // 3
	open_table.entries[2].replace_len= strlen(&open_table.arena[0x1800]) +1; // truncate the name because it skip  '\0'

	// copy the datas to  the destination address

	sys8_memcpy(dest_table_addr, (uint64_t) &open_table, sizeof(path_open_table));

	// set the path table
	sys8_path_table( dest_table_addr);


*/

#endif //#ifdef WAREZZZ
#endif // WAREZ_H
