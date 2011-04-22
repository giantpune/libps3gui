/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 * giantpune 2011
 *
 * gui_sound.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include <malloc.h>
#include <sys/thread.h>

//hermes' sound lib used for mp3/ogg
#include <audioplayer.h>

#include "gui.h"
#include "utils.h"
#include "wavdecoder.h"


#define SPU_SIZE(x) (((x)+127) & ~127)


static u32 spu = 0;
static sysSpuImage spu_image;
static sys_mutex_t soundMutex;
static bool inited = false;

//this is linked as a .a
INC_FILE( spu_soundmodule_bin );

/**
 * Constructor for the GuiSound class.
 */
GuiSound::GuiSound( const u8 * snd , s32 len, int t )
{
	sound = NULL;
	length = 0;
	voice = -1;
	if( !snd || !len )
		return;
	switch( t )
	{
	case SOUND_PCM://make sure sound data is aligned
		{
			sound = (u8*)memalign( 32, len );
			if( !sound )
				return;
			memcpy( (void*)sound, (const void*)snd, len );
			fd = NULL;
			format = VOICE_STEREO_16BIT;
			rate = 48000;
			length = len;
		}
		break;

	case SOUND_OGG://format & rate are not used
	case SOUND_MP3:
		fd = NULL;
		sound = (u8*)memalign( 32, len );
		if( !sound )
			return;
		memcpy( (void*)sound, (const void*)snd, len );
		length = len;
		break;
	case SOUND_WAV:
		{
			u32 l;
			sound = DecodeWav( snd, len, &l, &rate, &format );
			fd = NULL;
			length = l;
		}
		break;
	}


	type = t;
	volume = 100;
	loop = false;
}

GuiSound::GuiSound(const Resource &resource, int t )
{
	sound = NULL;
	length = 0;
	voice = -1;
	if( !resource.Size() || !resource.Data() )
		return;
	switch( t )
	{
	case SOUND_PCM://make sure sound data is aligned
		{
			sound = (u8*)memalign( 32, resource.Size() );
			if( !sound )
				return;
			memcpy( (void*)sound, (const void*)resource.Data(), resource.Size() );
			fd = NULL;
			format = VOICE_STEREO_16BIT;
			rate = 48000;
			length = resource.Size();
		}
		break;

	case SOUND_OGG://format & rate are not used
	case SOUND_MP3:
		fd = NULL;
		sound = (u8*)memalign( 32, resource.Size() );
		if( !sound )
			return;
		memcpy( (void*)sound, (const void*)resource.Data(), resource.Size() );
		length = resource.Size();
		break;
	case SOUND_WAV:
		{
			u32 l;
			sound = DecodeWav( resource.Data(), resource.Size(), &l, &rate, &format );
			fd = NULL;
			length = l;
		}
		break;
	}


	type = t;
	volume = 100;
	loop = false;
}

/**
 * Destructor for the GuiSound class.
 */
GuiSound::~GuiSound()
{
#ifndef NO_SOUND
	Stop();

	if( sound )
		free( (void*)sound );
#endif

}

void GuiSound::Play()
{
#ifndef NO_SOUND
	int vol;
	if( !sound )
		return;

	sysMutexLock( soundMutex, 0 );//mutex is used to make sure only 1 thread tries to aquire a voice at a time
	switch( type )
	{
	case SOUND_WAV:
	case SOUND_PCM:
		vol = 255*(volume/100.0);
		voice = SND_GetFirstUnusedVoice();
		if(voice >= 0)
		{
			if( loop )
				SND_SetInfiniteVoice( voice, format, rate, 0, (void*)sound, length, vol, vol );
			else
				SND_SetVoice(voice, format, rate, 0, (void*)sound, length, vol, vol, NULL);
		}
		break;
	case SOUND_OGG:
	case SOUND_MP3:
		{

			voice = 0;
			vol = 255*(volume/100.0);
			SetVolumeAudio( vol );
			fd = (FILE*)mem_open( (char*)sound, length );
			if( !fd )
				break;
			if( PlayAudiofd( fd, 0, loop ? AUDIO_INFINITE_TIME : AUDIO_ONE_TIME ) )
			{
				mem_close( (long)fd);
				fd = NULL;
			}
		}
		break;
	}
	sysMutexUnlock( soundMutex );
#endif
}

void GuiSound::Stop()
{
#ifndef NO_SOUND
	if( voice < 0 )
		return;

	switch(type)
	{
	case SOUND_WAV:
	case SOUND_PCM:
		SND_StopVoice( voice );
		break;

	case SOUND_OGG:
	case SOUND_MP3:
		if( fd )
		{
			StopAudio();//this function calls mem_close()
			fd = NULL;
		}
		break;
	}
#endif
}

void GuiSound::Pause()
{
#ifndef NO_SOUND
	if(voice < 0)
		return;

	switch(type)
	{
	case SOUND_PCM:
	case SOUND_WAV:
		SND_PauseVoice( voice, 1 );
		break;

	case SOUND_OGG:
	case SOUND_MP3:
		PauseAudio( 1 );
		break;
	}
#endif
}

void GuiSound::Resume()
{
#ifndef NO_SOUND
	if(voice < 0)
		return;

	switch(type)
	{
	case SOUND_PCM:
	case SOUND_WAV:
		SND_PauseVoice( voice, 0 );
		break;

	case SOUND_OGG:
	case SOUND_MP3:
		PauseAudio( 0 );
		break;
	}
#endif
}

bool GuiSound::IsPlaying()
{
	if(voice < 0)
		return false;

	if( SND_StatusVoice( voice ) == SND_WORKING || SND_StatusVoice( voice ) == SND_WAITING )
		return true;
	else
		return false;
}

void GuiSound::SetVolume(int vol)
{
#ifndef NO_SOUND
	volume = vol;

	if(voice < 0)
		return;

	int newvol = 255*(volume/100.0);

	switch(type)
	{
	case SOUND_PCM:
	case SOUND_WAV:
		SND_ChangeVolumeVoice( voice, newvol, newvol );
		break;

	case SOUND_OGG:
	case SOUND_MP3:
		SetVolumeAudio( 255*(volume/100.0) );
		break;
	}
#endif
}

void GuiSound::SetLoop(bool l)
{
	loop = l;
}

bool GuiSound::Init()
{
	if( inited )
		return true;
	//init spu
	u32 entry = 0;
	u32 segmentcount = 0;
	sysSpuSegment* segments = NULL;

	int ii = sysSpuInitialize( 6, 5 );
	if( ii )
	{
		printf("sysSpuInitialize(6, 5): %i\n", ii );
		return false;
	}

	ii = sysSpuRawCreate( &spu, NULL );
	if( ii )
	{
		printf("lv2SpuRawCreate( %08x ): %i\n", ii, spu );
		return false;
	}

	ii = sysSpuElfGetInformation( spu_soundmodule_bin, &entry, &segmentcount);
	if( ii )
	{
		sysSpuRawDestroy( spu );
		printf("sysSpuElfGetInformation: %i  %08x, %08x\n", ii, entry, segmentcount );
		return false;
	}

	size_t segmentsize = sizeof(sysSpuSegment) * segmentcount;
	segments = (sysSpuSegment*)memalign(128, SPU_SIZE(segmentsize)); // must be aligned to 128 or it break malloc() allocations
	if( !segments )
	{
		sysSpuRawDestroy( spu );
		printf("(sysSpuSegment*)memalign(128, SPU_SIZE(segmentsize)) failed\n" );
		return false;
	}

	memset( segments, 0, segmentsize );

	ii = sysSpuElfGetSegments( spu_soundmodule_bin, segments, segmentcount );
	if( ii )
	{
		sysSpuRawDestroy( spu );
		free( segments );
		printf("sysSpuElfGetSegments(): %i\n", ii );
		return false;
	}

	ii = sysSpuImageImport( &spu_image, spu_soundmodule_bin, 0 );
	if( ii )
	{
		sysSpuRawDestroy( spu );
		free( segments );
		printf("sysSpuImageImport(&spu_image, spu_soundmodule_bin, 0): %i\n", ii );
		return false;
	}

	ii = sysSpuRawImageLoad( spu, &spu_image );
	if( ii )
	{
		sysSpuRawDestroy( spu );
		free( segments );
		printf("sysSpuRawImageLoad(spu, &spu_image): %i\n", ii );
		return false;
	}

	//init sound library
	ii = SND_Init( spu );
	if( ii )
	{
		free( segments );
		sysSpuRawDestroy( spu );
		sysSpuImageClose( &spu_image );
		printf("SND_Init( %08x ): %i\n", ii, spu );
		return false;
	}

	//init mutex
	mutex_init( &soundMutex );

	SND_Pause( 0 );
	inited = true;
	return true;
}

void GuiSound::UnInit()
{
	if( !inited )
		return;

	SND_Pause( 1 );
	SND_End();
	sysMutexDestroy( soundMutex );
	sysSpuRawDestroy( spu );
	sysSpuImageClose( &spu_image );
	inited = false;
}
