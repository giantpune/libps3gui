

#include <audioplayer.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "wavdecoder.h"

//this function is based off a couple from wiixplorer by dimok
u8* DecodeWav( const u8* mem, u32 len, u32 *outlen, u32 *sRate, u32 *fmt )
{
	if( !mem )
		return NULL;

	u32 dataOffset = 0;
	SWaveHdr *Header = (SWaveHdr *)mem;
	dataOffset += sizeof( SWaveHdr );
	SWaveFmtChunk *FmtChunk = (SWaveFmtChunk *)( mem + dataOffset );
	dataOffset += sizeof( SWaveFmtChunk );
	SWaveChunk  *DataChunk;
	u8* ret;


	if( Header->magicRIFF != 0x52494646 )
	{
		printf("DecodeWav():  mising RIFF magic\n");
		return NULL;
	}
	else if( Header->magicWAVE != 0x57415645 )
	{
		printf("DecodeWav() missing WAVE magic\n");
		return NULL;
	}
	else if( FmtChunk->magicFMT != 0x666d7420 )
	{
		printf("DecodeWav() missing fmt magic\n");
		return NULL;
	}

	dataOffset = sizeof( SWaveHdr ) + le32( FmtChunk->size ) + 8;
	DataChunk = (SWaveChunk*)( mem + dataOffset );
	if(DataChunk->magicDATA == 0x66616374 )
	{
		dataOffset += 8 + le32( DataChunk->size );
		DataChunk = (SWaveChunk*)( mem + dataOffset );
	}
	if(DataChunk->magicDATA != 0x64617461 )
	{
		printf("DecodeWav() expected \"data\"\n");
		return NULL;
	}

	dataOffset += 8;
	u32 dataSize = le32( DataChunk->size );
	//bool is16bit = ( le16( FmtChunk->bps ) == 16 );
	u32 sampleRate = le32( FmtChunk->freq );
	u32 format = 0x6969;

	if( dataSize + dataOffset > len )
	{
		printf("DecodeWav() this wav looks busted :(   dataSize + dataOffset > len\n");
		return NULL;
	}

	if( le16( FmtChunk->channels ) == 1 && le16( FmtChunk->bps ) == 8 && le16( FmtChunk->alignment) <= 1 )
		format = VOICE_MONO_8BIT;
	else if( le16( FmtChunk->channels ) == 1 && le16( FmtChunk->bps ) == 16 && le16( FmtChunk->alignment ) <= 2 )
		format = VOICE_MONO_16BIT;
	else if( le16( FmtChunk->channels ) == 2 && le16( FmtChunk->bps ) == 8 && le16( FmtChunk->alignment ) <= 2 )
		format = VOICE_STEREO_8BIT;
	else if( le16( FmtChunk->channels ) == 2 && le16( FmtChunk->bps ) == 16 && le16( FmtChunk->alignment ) <= 4 )
		format = VOICE_STEREO_16BIT;
	else
	{
		printf("DecodeWav() unknown format\n");
		return NULL;
	}

	//allocate output buffer
	ret = (u8*)memalign( 32, dataSize );
	if( !ret )
	{
		printf("DecodeWav() failed to allocate %08x bytes\n", dataSize );
		return NULL;
	}

	u16* outPtr16 = (u16*)ret;
	u16* inPtr16 = (u16*)( mem + dataOffset );
	u32 read = dataSize / 2;

	for( u32 i = 0; i < read; ++i )
		outPtr16[ i ] = le16( inPtr16[ i ] );


	if( sRate )
		*sRate = sampleRate;
	if( fmt )
		*fmt = format;
	if( outlen )
		*outlen = dataSize;

	return ret;
}
