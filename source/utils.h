/* 
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/

#ifndef UTILS_H
#define UTILS_H

#include <tiny3d.h>
#include <sys/cond.h>
#include <sys/mutex.h>
#include <sys/thread.h>

#define MIN( x, y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )
#define MAX( x, y ) ( ( x ) > ( y ) ? ( x ) : ( y ) )
//#define RU(x,n)	(-(-(x) & -(n)))
#define RU(N, S) ((((N) + (S) - 1) / (S)) * (S))

#define le16(i) ((((u16) ((i) & 0xFF)) << 8) | ((u16) (((i) & 0xFF00) >> 8)))
#define le32(i) ((((u32)le16((i) & 0xFFFF)) << 16) | ((u32)le16(((i) & 0xFFFF0000) >> 16)))
#define le64(i) ((((u64)le32((i) & 0xFFFFFFFFLL)) << 32) | ((u64)le32(((i) & 0xFFFFFFFF00000000LL) >> 32)))


#define PI 				3.14159265f
#define FROM_ANGLE( x ) ( -( PI * x )/180.0f )


#define THREAD_SLEEP 100

#define KiB				1024
#define MiB				1048576
#define GiB				1073741824

#define INC_FILE( x ) \
	extern const u8 x [];\
	extern const u32 x##_size;

template <typename T>
class ForeachContainer {
public:
	inline ForeachContainer(const T& t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
	const T c;
	int brk;
	typename T::const_iterator i, e;
};

//apparently this macro doesnt work well if you try to break from inside it
#define FOREACH(variable, container)                                \
for (ForeachContainer<__typeof__(container)> _container_(container); \
	 !_container_.brk && _container_.i != _container_.e;              \
	 __extension__  ({ ++_container_.brk; ++_container_.i; }))                       \
	for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))

void hexdump( const void *d, int len );
void mutex_init( sys_mutex_t *m );
void cond_init( sys_cond_t *c, sys_mutex_t *m );

bool IsInRange( int num, int low, int high );



//for lack of a better place, these are the constants i see for settings and crap
/*enum LangCode
{
	Japanese = 0,
	English = 1,
	French = 2,
	Spanish = 3,
	German = 4,
	Italian = 5,
	Dutch = 6,
	Portugese = 7,
	Russian = 8,
	Korean = 9,
	ChineseTrad = 10,
	ChineseSimpl = 11,
	Finnish = 12,
	Swedish = 13,
	Danish = 14,
	Norwegian = 15
};*/

//PARAM.SFO values ( possibly used elsewhere )
/*
 parental level (Atelier Rorona The Alchemist of Arland has multiple parental levels?!)


(NTSC)
 e = 3	( LittleBigPlanet™2 )
 e10+ = 4
 T = 5
 M = 9

 (eur)
 3 = 2	( Gran Turismo 5 )
 7 = 3	( LittleBigPlanet™ )
 12 = 5	(skate.)
 16 = 7	( Star Wars: The Force Unleashed, WWE All Stars, Castlevania: Lords of Shadow )
 18 = 8 (SIREN: New Translation)
 18 = 9	( Bulletstorm, Dead Space™ 2 )

 (cero) (JS/JM)
 A = 1	(InitialD EXTREME STAGE)
 B = 5	( Another Century's Episode R )
 C = 7	( アルカナハート3, キャサリン )
 D = 8	(英雄たちの楽園)
*/
/*
 audio format
 2 LPCM = 1
 5.1 LPCM = 4
 7.1 LPCM = 0x10

 DolbyDigital = 0x102 = 258
 DTS Digital Surround = 0x202
 5.1 | 2 | DolbyDigital = 0x107 = 263
 5.1 | DD = 0x106
 2.1 | DD = 0x103



 */
/*
 resolution
 480 = 1
 576 = 2
 720 = 4
 1080 = 8
 480(16:9) = 0x10
 576(16:9) = 0x20
 720 | 576(16:9) | 576 | 480 | 480 (16:9) = 0x37 = 55

 */
/*ATTRIBUTE
  1 = remote play MP4 (SP/ATRAC)
  5 = remote play MP4 (AVC/AAC) ( Dragon Age II, TRINITY: Souls of Zill O'll, METAL GEAR SOLID 4, BAYONETTA, WWE All Stars
, Bulletstorm, Castlevania: Lords of Shadow™, Catherine, Dead Space™ 2, Dragon Ball: Raging Blast 2 )

  0x20 = unknown ( BATTLEFIELD: Bad Company 2™, Dante's Inferno™, NIER, Catherine, Dead Space™ 2 , Dragon Ball: Raging Blast 2 )
  0x1000 = unknown ( gran turismo 5, The Sly Trilogy™ )
  0x20000 = unknown ( LittleBigPlanet™2, Dead Space™ 2 )

  0x100
  0x400
  0x800000
  0x00801500 ( The Sly Trilogy™ )






bad company 2: Online Players: 24
Online Features
Content Download, Leaderboards, Online Multiplayer, PS Network Compatible, Teams, Trophies, Voice
Controllers
Dualshock 3 Controller, Headset


 */
/*
Sfo::Print()
15 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000020 32
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000009 9
[6] string "PS3_SYSTEM_VER" "03.0100"
[7] number "RESOLUTION" 0x00000037 55
[8] number "SOUND_FORMAT" 0x00000103 259
[9] string "TITLE" "Dante's Inferno™"
[10] string "TITLE_01" "Dante's Inferno™"
[11] string "TITLE_02" "Dante's Inferno™"
[12] string "TITLE_03" "Dante's Inferno™"
[13] string "TITLE_ID" "BLUS30405"
[14] string "VERSION" "01.00"*/
/*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000005 5
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000009 9
[6] string "PS3_SYSTEM_VER" "03.5000"
[7] number "RESOLUTION" 0x00000007 7
[8] number "SOUND_FORMAT" 0x00000317 791
[9] string "TITLE" "Dragon Age II"
[10] string "TITLE_ID" "BLUS30645"
[11] string "VERSION" "01.01"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000005 5
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000005 5
[6] string "PS3_SYSTEM_VER" "03.5500"
[7] number "RESOLUTION" 0x00000005 5
[8] number "SOUND_FORMAT" 0x00000103 259
[9] string "TITLE" "TRINITY: Souls of Zill O'll"
[10] string "TITLE_ID" "BLUS30503"
[11] string "VERSION" "01.01"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000009 9
[6] string "PS3_SYSTEM_VER" "02.7600"
[7] number "RESOLUTION" 0x00000037 55
[8] number "SOUND_FORMAT" 0x00000117 279
[9] string "TITLE" "FALLOUT 3 GAME OF THE YEAR"
[10] string "TITLE_ID" "BLUS30451"
[11] string "VERSION" "01.00"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000005 5
[6] string "PS3_SYSTEM_VER" "02.4200"
[7] number "RESOLUTION" 0x00000037 55
[8] number "SOUND_FORMAT" 0x00000307 775
[9] string "TITLE" "Valkyria Chronicles"
[10] string "TITLE_ID" "BLUS30196"
[11] string "VERSION" "01.00"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000005 5
[6] string "PS3_SYSTEM_VER" "03.4000"
[7] number "RESOLUTION" 0x00000015 21
[8] number "SOUND_FORMAT" 0x00000107 263
[9] string "TITLE" "Another Century's Episode R"
[10] string "TITLE_ID" "BLJS10081"
[11] string "VERSION" "01.02"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000005 5
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000009 9
[6] string "PS3_SYSTEM_VER" "02.2000"
[7] number "RESOLUTION" 0x0000003f 63
[8] number "SOUND_FORMAT" 0x00000107 263
[9] string "TITLE" "METAL GEAR SOLID 4
GUNS OF THE PATRIOTS"
[10] string "TITLE_ID" "BLUS30109"
[11] string "VERSION" "01.01"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000020 32
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000009 9
[6] string "PS3_SYSTEM_VER" "03.1500"
[7] number "RESOLUTION" 0x00000037 55
[8] number "SOUND_FORMAT" 0x00000307 775
[9] string "TITLE" "NIER"
[10] string "TITLE_ID" "BLUS30481"
[11] string "VERSION" "01.00"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01180_00"
[6] number "PARENTAL_LEVEL" 0x00000003 3
[7] string "PS3_SYSTEM_VER" "03.1500"
[8] number "RESOLUTION" 0x00000037 55
[9] number "SOUND_FORMAT" 0x00000117 279
[10] string "TITLE" "3D DOT GAME HEROES"
[11] string "TITLE_ID" "BLES00875"
[12] string "VERSION" "01.01"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01464_00"
[6] number "PARENTAL_LEVEL" 0x00000007 7
[7] string "PS3_SYSTEM_VER" "03.5000"
[8] number "RESOLUTION" 0x00000037 55
[9] number "SOUND_FORMAT" 0x00000001 1
[10] string "TITLE" "アルカナハート3"
[11] string "TITLE_ID" "BLJM60248"
[12] string "VERSION" "01.02"
*//*
Sfo::Print()
18 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000000 0
[6] number "PARENTAL_LEVEL_A" 0x00000005 5
[7] number "PARENTAL_LEVEL_C" 0xffffffff 4294967295
[8] number "PARENTAL_LEVEL_E" 0x00000005 5
[9] number "PARENTAL_LEVEL_H" 0xffffffff 4294967295
[10] number "PARENTAL_LEVEL_J" 0x00000001 1
[11] number "PARENTAL_LEVEL_K" 0x00000007 7
[12] string "PS3_SYSTEM_VER" "03.4100"
[13] number "RESOLUTION" 0x0000003f 63
[14] number "SOUND_FORMAT" 0x00000117 279
[15] string "TITLE" "Atelier Rorona
The Alchemist of Arland"
[16] string "TITLE_ID" "BLUS30465"
[17] string "VERSION" "01.01"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000020 32
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR00851_00"
[6] number "PARENTAL_LEVEL" 0x00000009 9
[7] string "PS3_SYSTEM_VER" "03.0100"
[8] number "RESOLUTION" 0x0000003f 63
[9] number "SOUND_FORMAT" 0x00000303 771
[10] string "TITLE" "BAYONETTA"
[11] string "TITLE_ID" "BLUS30367"
[12] string "VERSION" "01.00"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00801500 8393984
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000003 3
[6] string "PS3_SYSTEM_VER" "03.5000"
[7] number "RESOLUTION" 0x00000026 38
[8] number "SOUND_FORMAT" 0x00000001 1
[9] string "TITLE" "The Sly Trilogy™"
[10] string "TITLE_ID" "BCES00968"
[11] string "VERSION" "01.00"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00001000 4096
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000003 3
[6] string "PS3_SYSTEM_VER" "03.5000"
[7] number "RESOLUTION" 0x0000003f 63
[8] number "SOUND_FORMAT" 0x00000317 791
[9] string "TITLE" "Gran Turismo 5"
[10] string "TITLE_ID" "BCUS98114"
[11] string "VERSION" "01.00"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000003 3
[6] string "PS3_SYSTEM_VER" "02.1000"
[7] number "RESOLUTION" 0x00000015 21
[8] number "SOUND_FORMAT" 0x00000117 279
[9] string "TITLE" "Hot Shots Golf®: Out of Bounds"
[10] string "TITLE_ID" "BCUS98115"
[11] string "VERSION" "01.00"
*//*
Sfo::Print()
16 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000002 2
[6] string "PS3_SYSTEM_VER" "02.7600"
[7] number "RESOLUTION" 0x0000002e 46
[8] number "SOUND_FORMAT" 0x00000107 263
[9] string "TITLE" "Katamari Forever"
[10] string "TITLE_01" "Katamari Forever"
[11] string "TITLE_02" "Katamari Forever"
[12] string "TITLE_03" "Katamari Forever"
[13] string "TITLE_05" "Katamari Forever"
[14] string "TITLE_ID" "BLES00658"
[15] string "VERSION" "01.00"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000005 5
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01520_00"
[6] number "PARENTAL_LEVEL" 0x00000007 7
[7] string "PS3_SYSTEM_VER" "03.4000"
[8] number "RESOLUTION" 0x00000037 55
[9] number "SOUND_FORMAT" 0x00000317 791
[10] string "TITLE" "WWE All Stars"
[11] string "TITLE_ID" "BLES00995"
[12] string "VERSION" "01.01"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR00612_00"
[6] number "PARENTAL_LEVEL" 0x00000009 9
[7] string "PS3_SYSTEM_VER" "02.7600"
[8] number "RESOLUTION" 0x0000001d 29
[9] number "SOUND_FORMAT" 0x00000317 791
[10] string "TITLE" "Borderlands™"
[11] string "TITLE_ID" "BLUS30386"
[12] string "VERSION" "01.00"
*//*
Sfo::Print()
14 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000005 5
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01203_00"
[6] number "PARENTAL_LEVEL" 0x00000009 9
[7] string "PS3_SYSTEM_VER" "03.5500"
[8] number "RESOLUTION" 0x00000037 55
[9] number "SOUND_FORMAT" 0x00000107 263
[10] string "TITLE" "Bulletstorm"
[11] string "TITLE_00" "バレットストーム"
[12] string "TITLE_ID" "BLES01134"
[13] string "VERSION" "01.01"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01247_00"
[6] number "PARENTAL_LEVEL" 0x00000009 9
[7] string "PS3_SYSTEM_VER" "03.5000"
[8] number "RESOLUTION" 0x0000003f 63
[9] number "SOUND_FORMAT" 0x00000117 279
[10] string "TITLE" "Call of Duty®: Black Ops"
[11] string "TITLE_ID" "BLUS30591"
[12] string "VERSION" "01.01"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000020 32
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000007 7
[6] string "PS3_SYSTEM_VER" "03.4100"
[7] number "RESOLUTION" 0x0000003f 63
[8] number "SOUND_FORMAT" 0x00000317 791
[9] string "TITLE" "Castlevania: Lords of Shadow™"
[10] string "TITLE_ID" "BLES01047"
[11] string "VERSION" "01.01"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000025 37
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000007 7
[6] string "PS3_SYSTEM_VER" "03.5000"
[7] number "RESOLUTION" 0x00000005 5
[8] number "SOUND_FORMAT" 0x00000307 775
[9] string "TITLE" "Catherine"
[10] string "TITLE_00" "キャサリン"
[11] string "TITLE_ID" "BLJM60215"
[12] string "VERSION" "01.01"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR00699_00"
[6] number "PARENTAL_LEVEL" 0x00000009 9
[7] string "PS3_SYSTEM_VER" "03.4000"
[8] number "RESOLUTION" 0x00000015 21
[9] number "SOUND_FORMAT" 0x00000307 775
[10] string "TITLE" "Dead Rising 2"
[11] string "TITLE_ID" "BLUS30439"
[12] string "VERSION" "01.00"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00020025 131109
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01438_00"
[6] number "PARENTAL_LEVEL" 0x00000009 9
[7] string "PS3_SYSTEM_VER" "03.5000"
[8] number "RESOLUTION" 0x0000003f 63
[9] number "SOUND_FORMAT" 0x00000117 279
[10] string "TITLE" "Dead Space™ 2"
[11] string "TITLE_ID" "BLES01040"
[12] string "VERSION" "01.01"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000025 37
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01286_00"
[6] number "PARENTAL_LEVEL" 0x00000005 5
[7] string "PS3_SYSTEM_VER" "03.5000"
[8] number "RESOLUTION" 0x00000015 21
[9] number "SOUND_FORMAT" 0x00000117 279
[10] string "TITLE" "Dragon Ball: Raging Blast 2"
[11] string "TITLE_ID" "BLUS30581"
[12] string "VERSION" "01.01"
*//*
Sfo::Print()
12 entries
[0] number "ATTRIBUTE" 0x00000000 0
[1] number "BOOTABLE" 0x00000001 1
[2] string "CATEGORY" "DG"
[3] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[4] number "PARENTAL_LEVEL" 0x00000005 5
[5] string "PS3_SYSTEM_VER" "01.8000"
[6] number "RESOLUTION" 0x00000015 21
[7] number "SOUND_FORMAT" 0x00000107 263
[8] string "TITLE" "Dynasty Warriors: GUNDAM"
[9] string "TITLE_01" "Dynasty Warriors: GUNDAM"
[10] string "TITLE_ID" "BLUS30058"
[11] string "VERSION" "01.00"
*//*
Sfo::Print()
13 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] string "NP_COMMUNICATION_ID" "NPWR01421_00"
[6] number "PARENTAL_LEVEL" 0x00000009 9
[7] string "PS3_SYSTEM_VER" "03.5500"
[8] number "RESOLUTION" 0x0000003f 63
[9] number "SOUND_FORMAT" 0x00000107 263
[10] string "TITLE" "FIGHT NIGHT CHAMPION"
[11] string "TITLE_ID" "BLUS30608"
[12] string "VERSION" "01.00"
*//*
Sfo::Print()
12 entries
[0] string "APP_VER" "01.00"
[1] number "ATTRIBUTE" 0x00000000 0
[2] number "BOOTABLE" 0x00000001 1
[3] string "CATEGORY" "DG"
[4] string "LICENSE" "Library programs ©Sony Computer Entertainment Inc. Licensed for play on the PLAYSTATION®3 Computer Entertainment System or authorized PLAYSTATION®3 format systems. For full terms and conditions see the user's manual. This product is authorized and produced under license from Sony Computer Entertainment Inc. Use is subject to the copyright laws and the terms and conditions of the user's license."
[5] number "PARENTAL_LEVEL" 0x00000009 9
[6] string "PS3_SYSTEM_VER" "03.5000"
[7] number "RESOLUTION" 0x00000015 21
[8] number "SOUND_FORMAT" 0x00000103 259
[9] string "TITLE" "Fist of the North Star: Ken's Rage"
[10] string "TITLE_ID" "BLUS30504"

*/

#endif

