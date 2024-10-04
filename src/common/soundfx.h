// C_Sound

//a public domain SDL2 class to output wave and .ogg sounds using part of the portaudio library
// and http://www.nothings.org/stb_vorbis/.
//supports windows linux and mac.
//useful in a game for example, where you have control over what sound files you play.
//wav - 44100Hz, 16bit mono
//ogg - 44100Hz, mono or stereo
//output - all mixed 44100Hz, stereo

#ifndef _SOUNDFX_H
#define _SOUNDFX_H

#define MAX_INDEX    64
#define MAX_CHANNELS 16

//CHANGE here to one of them
//#define _NO_SOUND_
#define _SDL_SOUND_

#include "common.h"
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#ifdef _SDL_SOUND_
#include <SDL.h>
#endif

#define SAMPLE_RATE        44100
#define MIXER_SCALE        0.5f
#define OGG_FIFO_LOW       8000   //stereo 44100 -> 91ms (8000 samples)
#include "pa_ringbuffer.h"

struct S_LoadedSound
{
	int iType; //-1 not used, 0 = WAV 44 16 MONO, 1 = OGG stream 44 ST
	union {
		float *pSndData;
		uint8_t *pSndDataB;
		uint8_t *pOggData;
	};

	int iLength; //bytes
};

struct S_Channel
{
	//common
	volatile bool bPlaying;
	volatile bool bStopping;
	float fFadeOut;
	float fPanL, fPanR, fVol;
	int iSndIndex;
	int iInPos; //pos in input
	int iOutPos; //pos in output

	bool bLooping;
	int iLoopStartMs;
	int iLoopStartPos;

	//ogg stream
	stb_vorbis *pVStream;
	bool bEndReached;

	//ring buffer (FIFO) towards audio callback
	PaUtilRingBuffer rBufToRT;
	float            rBufToRTData[32768];
};

#pragma pack(1)
struct S_WaveHdr {
	char        szRiff[4];       //'RIFF'
	uint32_t    iChunkSize;      //not important
	char        szWaveFmt[8];    //'WAVEfmt '
	uint32_t    iSizeFmt;        //not important

	uint16_t    wFormatTag;      // format type
	uint16_t    nChannels;       // number of channels (i.e. mono, stereo...)
	uint32_t    nSamplesPerSec;  // sample rate
	uint32_t    nAvgBytesPerSec; // for buffer estimation
	uint16_t    nBlockAlign;     // block size of data
	uint16_t    wBitsPerSample;  // number of bits per sample of mono data
	//uint16_t    cbSize;          // the count in bytes of the size of extra information (after cbSize)

	char        szData[4];       //not important
	uint32_t    iTotLength;      //smp data length
};
#pragma pack()


class C_Sound
{
public:
	C_Sound();
	~C_Sound();
	bool Init(char *szDiskWriteFilename = NULL);

	static C_Sound *GetSoundObject() { return s_pTheSound; };

	bool Load(const char *szFilename, const char *szResname, int iIndex);
	void Unload(int iIndex);

	void *Play(int iIndex, float fVolume = 1.0f, float fPanning = 0.0f, bool bLooping = false, int iLoopStartMs = 0);
	void Stop(void *pChannel);

	void SetMasterVolume(float fVolume);
	void ToggleSoundsOnOff();

	void SetVolume(void *pChannel, float fVolume);
	void SetPanning(void *pChannel, float fPanning);

private:
	static C_Sound *s_pTheSound; //there can only be one sound obj and this is it

	//disk writer
	S_WaveHdr m_stHeader;
	FILE *m_pFile;

	S_LoadedSound m_pSounds[MAX_INDEX];
	S_Channel m_pChannels[MAX_CHANNELS];

	float m_fMasterVolume;
	bool m_bOutputEnabled;

#ifdef _SDL_SOUND_
	void Setup();
	void Free();
	void HandleDiskWriterInit(char *szDiskWriteFilename);

	float m_aBuf1[4096 * 2]; //temp buf used in callback for ogg (stereo data), read fifo
	float m_aBuf2[4096 * 2]; //temp buf used in update for ogg (stereo data), write fifo

	//update thread
	bool m_bThreadDone;
	C_Mutex *m_pMutex;
	C_Thread *m_pUpdateThread;
	void Update(int i_iChannel = -1);
	static THREAD_RET UpdateThread(void *pSound);

	static void AudioCallback(void* userdata, Uint8* stream, int len);

	SDL_AudioDeviceID m_audioDeviceID;
#endif
};

#endif
