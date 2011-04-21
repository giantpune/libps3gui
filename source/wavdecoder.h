#ifndef WAVDECODER_H
#define WAVDECODER_H

typedef struct
{
	u32 magicRIFF;
	u32 size;
	u32 magicWAVE;
} SWaveHdr;

typedef struct
{
	u32 magicFMT;
	u32 size;
	u16 format;
	u16 channels;
	u32 freq;
	u32 avgBps;
	u16 alignment;
	u16 bps;
} SWaveFmtChunk;

typedef struct
{
	u32 magicDATA;
	u32 size;
} SWaveChunk;

//decode a wave into pcm data
u8* DecodeWav( const u8* mem, u32 len, u32 *outlen, u32 *sRate, u32 *fmt );


#endif // WAVDECODER_H
