// C_Sound

//a public domain SDL2 class to output wave and .ogg sounds using part of the portaudio library
// and http://www.nothings.org/stb_vorbis/.
//supports windows linux and mac.
//useful in a game for example, where you have control over what sound files you play.
//wav - 44100Hz, 16bit mono
//ogg - 44100Hz, mono or stereo
//output - all mixed 44100Hz, stereo

#include "soundfx.h"

C_Sound *C_Sound::s_pTheSound = NULL;

#ifdef _NO_SOUND_

C_Sound::C_Sound() {}
C_Sound::~C_Sound() {}
bool C_Sound::Init(char *szDiskWriteFilename) { return true; }
bool C_Sound::Load(char *szFilename, int iIndex) { return true; }
void C_Sound::Unload(int iIndex) {}
void *C_Sound::Play(int iIndex, float fVolume, float fPanning, bool bLooping,
	int iLoopStartMs) { return NULL; }
void C_Sound::Stop(void *pChannel) {}
void C_Sound::SetMasterVolume(float fVolume) {}
void C_Sound::ToggleSoundsOnOff() {}
void C_Sound::SetVolume(void *pChannel, float fVolume) {}
void C_Sound::SetPanning(void *pChannel, float fPanning) {}

#endif

#ifdef _SDL_SOUND_

#ifdef WIN32
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#endif

#include "fileresource.h"

C_Sound::C_Sound()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	Setup();
}

C_Sound::~C_Sound()
{
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	Free();
}

void C_Sound::AudioCallback(void* userdata, Uint8* stream, int len)
{
	int i, j;

	C_Sound *pSnd = C_Sound::GetSoundObject();

	//reset
	int error = len % 4;
	if (error != 0) return;

	SDL_LockAudioDevice(pSnd->m_audioDeviceID);

	len /= 4;
	for (i = 0; i < len*2; i++) {
		((int16_t*)stream)[i] = 0;
	}

	for (j = 0; j < MAX_CHANNELS; j++) {
		S_Channel *pCh = &pSnd->m_pChannels[j];
		if (pCh->bPlaying) {
			int idx = pCh->iSndIndex;
			float fLeft = pSnd->m_fMasterVolume*pCh->fVol*pCh->fPanL;
			float fRight = pSnd->m_fMasterVolume*pCh->fVol*pCh->fPanR;

			if (pSnd->m_pSounds[idx].iType == 0) {
				//sound data is wave
				for (i = 0; i < len; i++) {
					//full fade out over this buffer, then stop (reaches 5% of volume in 300 samples)
					if (pCh->bStopping) pCh->fFadeOut *= 0.99f;

					float fS = MIXER_SCALE * pSnd->m_pSounds[idx].pSndData[pCh->iInPos] * pCh->fFadeOut;

					int iSL = (int)(fLeft * fS * 32767.0f);
					int iSR = (int)(fRight * fS * 32767.0f);
					if (iSL > 32767) iSL = 32767;
					if (iSL < -32767) iSL = -32767;
					if (iSR > 32767) iSR = 32767;
					if (iSR < -32767) iSR = -32767;

					((int16_t*)stream)[i * 2 + 0] += iSL;
					((int16_t*)stream)[i * 2 + 1] += iSR;

					pCh->iInPos += 1;
					if (pCh->iInPos >= pSnd->m_pSounds[idx].iLength) {
						if (!pCh->bLooping) {
							pCh->bPlaying = false;
							break;
						}
						else pCh->iInPos = pCh->iLoopStartPos;
					}
				}
			} else {
				//sound data is ogg

				//consume the input queue
				int iLength = PaUtil_GetRingBufferReadAvailable(&pCh->rBufToRT);
				if (iLength >= len * 2) iLength = len * 2;
				PaUtil_ReadRingBuffer(&pCh->rBufToRT, &pSnd->m_aBuf1[0], iLength);

				for (i = 0; i < iLength / 2; i++) {
					//full fade out over this buffer, then stop (reaches 5% of volume in 300 samples)
					if (pCh->bStopping) pCh->fFadeOut *= 0.99f;

					float fSL = MIXER_SCALE * fLeft*pSnd->m_aBuf1[i * 2 + 0] * pCh->fFadeOut;
					float fSR = MIXER_SCALE * fRight*pSnd->m_aBuf1[i * 2 + 1] * pCh->fFadeOut;

					int iSL = (int)(fLeft * fSL * 32767.0f);
					int iSR = (int)(fRight * fSR * 32767.0f);
					if (iSL > 32767) iSL = 32767;
					if (iSL < -32767) iSL = -32767;
					if (iSR > 32767) iSR = 32767;
					if (iSR < -32767) iSR = -32767;

					((int16_t*)stream)[i * 2 + 0] += iSL;
					((int16_t*)stream)[i * 2 + 1] += iSR;

				}
				if (pCh->bEndReached) {
					if (!pCh->bLooping) {
						pCh->bPlaying = false;
						break;
					}
				}
			}

			if (pCh->bStopping && pCh->fFadeOut < 0.05f) pCh->bPlaying = false; //stop
		}
	}

	//disk writer (breaks the rule of doing as little as possible in this callback)
	if (pSnd->m_pFile) {
		pSnd->m_stHeader.iTotLength += len * 4;
		fwrite(stream, len * 4, 1, pSnd->m_pFile);
	}

	SDL_UnlockAudioDevice(pSnd->m_audioDeviceID);
}

bool C_Sound::Init(char* szDiskWriteFilename)
{
	HandleDiskWriterInit(szDiskWriteFilename);

	SDL_AudioSpec wantedSpec, obtainedSpec;
	wantedSpec.freq = 44100;
	wantedSpec.format = AUDIO_S16LSB;
	wantedSpec.channels = 2;
	wantedSpec.samples = 4096;
	wantedSpec.callback = AudioCallback;
	wantedSpec.userdata = this;
	wantedSpec.padding = 0;
	wantedSpec.silence = 0;
	wantedSpec.size = 16384;

	m_audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &wantedSpec, &obtainedSpec, 0);
	if (m_audioDeviceID == 0) {
		printf("Failed to open audio device: %s\n", SDL_GetError());
		return false;
	}
	SDL_PauseAudioDevice(m_audioDeviceID, 0);

	//start update thread
	m_bThreadDone = false;
	m_pUpdateThread = new C_Thread(UpdateThread, this);

	return true;
}

void C_Sound::Setup()
{
	m_fMasterVolume = 1.0f;

	m_bOutputEnabled = true;
	for (int i = 0; i < MAX_INDEX; i++) {
		m_pSounds[i].iType = -1;
	}
	for (int i = 0; i < MAX_CHANNELS; i++) {
		m_pChannels[i].bPlaying = false;
		m_pChannels[i].pVStream = NULL;
	}
	s_pTheSound = this;
	m_pFile = NULL;

	m_pMutex = new C_Mutex();
	m_pUpdateThread = NULL;
}

void C_Sound::Free()
{
	m_bThreadDone = true;
	if (m_pUpdateThread != NULL) {
		m_pUpdateThread->WaitForThreadExit();
		m_pUpdateThread = NULL;
	}

	for (int i = 0; i < MAX_CHANNELS; i++) {
		if (m_pChannels[i].pVStream) stb_vorbis_close(m_pChannels[i].pVStream);
		m_pChannels[i].pVStream = NULL;
		m_pChannels[i].bPlaying = false;
	}

	if (m_pFile) {
		fclose(m_pFile);
	}

	for (int i = 0; i < MAX_INDEX; i++) {
		if (m_pSounds[i].iType != -1) {
			delete[] m_pSounds[i].pSndData;
			m_pSounds[i].iType = -1;
		}
	}

	delete m_pMutex;

	s_pTheSound = NULL;
}

void C_Sound::HandleDiskWriterInit(char *szDiskWriteFilename)
{
	//disk writer code
	if (m_pFile) {
		strncpy(m_stHeader.szRiff, "RIFF", 4);
		m_stHeader.iChunkSize = m_stHeader.iTotLength - 36;
		strncpy(m_stHeader.szWaveFmt, "WAVEfmt ", 8);
		m_stHeader.iSizeFmt = 16;

		m_stHeader.wFormatTag = 1;
		m_stHeader.nChannels = 2;
		m_stHeader.nSamplesPerSec = 44100;
		m_stHeader.nAvgBytesPerSec = 176400;
		m_stHeader.nBlockAlign = 4;
		m_stHeader.wBitsPerSample = 16;

		strncpy(m_stHeader.szData, "data", 4);

		fseek(m_pFile, 0, SEEK_SET);
		fwrite(&m_stHeader, sizeof(m_stHeader), 1, m_pFile);
		fclose(m_pFile);
	}
	if (szDiskWriteFilename) {
		m_pFile = fopen(szDiskWriteFilename, "wb");
		memset(&m_stHeader, 0, sizeof(m_stHeader));
		fwrite(&m_stHeader, sizeof(m_stHeader), 1, m_pFile); //header rewritten later
	}
	else {
		m_pFile = NULL;
	}
}

THREAD_RET C_Sound::UpdateThread(void *pSound)
{
	C_Sound *pThis = (C_Sound*)pSound;

	while (!pThis->m_bThreadDone) {
		pThis->m_pMutex->Enter();
		pThis->Update();
		pThis->m_pMutex->Leave();

		mssleep(10);
	}
	return THREAD_RET_0;
}

void C_Sound::Update(int iChannel)
{
	//ogg only, fill up the fifo consumed in the callback
	for (int i = 0; i < MAX_CHANNELS; i++) {
		S_Channel *pCh = &m_pChannels[i];
		if (pCh->bPlaying || i == iChannel) {
			int idx = pCh->iSndIndex;
			if (m_pSounds[idx].iType == 1) {
				int iNumCh, n;
				float **outputs;
				while (PaUtil_GetRingBufferReadAvailable(&pCh->rBufToRT) < OGG_FIFO_LOW && !pCh->bEndReached) {
					int iUsed = stb_vorbis_decode_frame_pushdata(pCh->pVStream, m_pSounds[idx].pOggData + pCh->iInPos,
						m_pSounds[idx].iLength - pCh->iInPos, &iNumCh, &outputs, &n);
					if (iUsed == 0) {
						if (!pCh->bLooping) pCh->bEndReached = true;
						else {
							stb_vorbis_flush_pushdata(pCh->pVStream);
							pCh->iInPos = pCh->iLoopStartPos; //restart at the loop start
						}
					}
					pCh->iInPos += iUsed;
					if (n == 0) continue;
					pCh->iOutPos += n;
					if (pCh->iLoopStartPos == -1 && ((pCh->iOutPos * 1000) / 44100) > pCh->iLoopStartMs) {
						pCh->iLoopStartPos = pCh->iInPos;
					}

					for (i = 0; i < n; i++) {
						m_aBuf2[i * 2 + 0] = outputs[0][i];
						m_aBuf2[i * 2 + 1] = (iNumCh >= 2) ? outputs[1][i] : outputs[0][i];
					}
					PaUtil_WriteRingBuffer(&pCh->rBufToRT, m_aBuf2, n * 2);
				}
			}
		}
	}
}

void* C_Sound::Play(int iIndex, float fVolume, float fPanning, bool bLooping, int iLoopStartMs)
{
	if (!m_bOutputEnabled) return NULL;

	if (m_pSounds[iIndex].iType == -1) return NULL;

	for (int i = 0; i < MAX_CHANNELS; i++) {
		if (!m_pChannels[i].bPlaying) {
			//use the first free channel found

			//free any previous stream running on this channel
			if (m_pChannels[i].pVStream) {
				stb_vorbis_close(m_pChannels[i].pVStream);
				m_pChannels[i].pVStream = NULL;
			}

			m_pChannels[i].iOutPos = 0;
			m_pChannels[i].bLooping = bLooping;
			m_pChannels[i].iLoopStartMs = iLoopStartMs;
			m_pChannels[i].iLoopStartPos = -1; //before it is known

			if (m_pSounds[iIndex].iType == 1) {
				//sound data is ogg
				int iUsed, iError;
				m_pChannels[i].pVStream = stb_vorbis_open_pushdata(m_pSounds[iIndex].pOggData,
					m_pSounds[iIndex].iLength, &iUsed, &iError, NULL);
				m_pChannels[i].iInPos = iUsed;
				m_pChannels[i].bEndReached = false;

				//initialize communication buffer (fifo)
				PaUtil_InitializeRingBuffer(&m_pChannels[i].rBufToRT, sizeof(float),
					32768, m_pChannels[i].rBufToRTData);
			}
			else {
				//sound data is wave
				m_pChannels[i].iInPos = 0;
				m_pChannels[i].iLoopStartPos = (int)((iLoopStartMs*SAMPLE_RATE) / 1000);
			}

			m_pChannels[i].iSndIndex = iIndex;
			SetVolume(&m_pChannels[i], fVolume); ;
			SetPanning(&m_pChannels[i], fPanning);
			m_pChannels[i].bStopping = false;
			m_pChannels[i].fFadeOut = 1.0f;

			//fill buffer for ogg sound this first time
			m_pMutex->Enter();
			Update(i);
			m_pMutex->Leave();

			//start playing when all is set
			m_pChannels[i].bPlaying = true;

			return &m_pChannels[i];
		}
	}

	return NULL;
}

bool C_Sound::Load(const char *szFilename, const char *szResname, int iIndex)
{
	S_WaveHdr stHDR;
	S_LoadedSound *pSnd = &m_pSounds[iIndex];

	bool bResult = false;
	if (iIndex >= MAX_INDEX) return false;

	//unload previouse file (if any)
	if (pSnd->iType != -1) {
		delete[] pSnd->pSndData;
		pSnd->iType = -1;
	}

	C_Resource *pResource = new C_Resource();
	if (!pResource->SetFilename(szFilename, szResname)) {
		delete pResource;
		return false;
	}

	int iSize = pResource->GetSize();
	char *pBuffer = new char[iSize];
	if (!pResource->Read(pBuffer, iSize)) {
		delete pResource;
		delete[]pBuffer;
		return false;
	}
	delete pResource;

	const char *szExt = strrchr(szFilename, '.');
	if (szExt && _stricmp(szExt, ".ogg") == 0) {
		pSnd->iType = 1;

		pSnd->pOggData = (uint8_t*)pBuffer; pBuffer = NULL;
		pSnd->iLength = iSize;
	} else {
		memcpy(&stHDR, pBuffer, sizeof(S_WaveHdr));

		int r = strncmp(stHDR.szRiff, "RIFF", 4);
		if (r != 0 || strncmp(stHDR.szWaveFmt, "WAVEfmt ", 8) != 0) {
			delete[] pBuffer;
			return false;
		}

		int iNumSamples = stHDR.iTotLength / 2;

		pSnd->iType = 0;
		pSnd->iLength = iNumSamples;
		pSnd->pSndData = new float[iNumSamples];
		int16_t *pSndData16 = (int16_t*)(pBuffer + sizeof(S_WaveHdr));

		for (int i = 0; i < iNumSamples; i++) {
			pSnd->pSndData[i] = pSndData16[i] / 32768.0f;
		}
		delete[] pBuffer; pBuffer = NULL;

	}
	return true;
}

void C_Sound::Unload(int iIndex)
{
	S_LoadedSound *pSnd = &m_pSounds[iIndex];
	if (pSnd->iType != -1) {
		delete[] pSnd->pSndData;
		pSnd->iType = -1;
	}
}

void C_Sound::Stop(void *pChannel)
{
	S_Channel *pCh = (S_Channel*)pChannel;
	if (pCh == NULL) return;

	//fade out will be done on this channel, then it will stop playing
	pCh->bStopping = true;
}

void C_Sound::ToggleSoundsOnOff()
{
	m_bOutputEnabled = !m_bOutputEnabled;
}

void C_Sound::SetMasterVolume(float fVolume)
{
	if (fVolume > 1.0f) fVolume = 1.0f; //max
	if (fVolume < 0.0f) fVolume = 0.0f; //min

	m_fMasterVolume = fVolume;
}

void C_Sound::SetVolume(void *pChannel, float fVolume)
{
	if (fVolume > 1.0f) fVolume = 1.0f; //max
	if (fVolume < 0.0f) fVolume = 0.0f; //min

	if (pChannel) {
		((S_Channel*)pChannel)->fVol = fVolume;
	}
}

void C_Sound::SetPanning(void *pChannel, float fPanning)
{
	if (fPanning > 1.0f) fPanning = 1.0f; //max
	if (fPanning < -1.0f) fPanning = -1.0f; //min
	if (pChannel) {
		if (fPanning > 0) {
			((S_Channel*)pChannel)->fPanL = 1.0f - fPanning;
			((S_Channel*)pChannel)->fPanR = 1.0f;
		}
		else {
			((S_Channel*)pChannel)->fPanL = 1.0f;
			((S_Channel*)pChannel)->fPanR = 1.0f + fPanning;
		}
	}
}

#endif
