// C_MovieMaker

//class to be able to save a sequence of screen images (SDL2) together with sound (SDL2).
// can also save single images.
// - each save session of files is added to a new directory named 'date_time'.
// - each image is a rle compressed .tga

#include "moviemaker.h"
#include "targa.h"
#include "fileresource.h"
#include "logger.h" //GetDateString

//thread to load balance saving
THREAD_RET C_MovieMaker::WorkThread(void *pMovieMaker)
{
	char szFilename[MAX_PATH];
	C_Image *pCurrent;

	C_MovieMaker *pThis = (C_MovieMaker*)pMovieMaker;

	bool bEmpty = true;
	while(!pThis->m_bDone || !bEmpty) {
		pCurrent = NULL;
		pThis->m_pMutex->Enter();
		if(!pThis->m_clImageQueue.empty()) {
			pCurrent = pThis->m_clImageQueue[0];
			pThis->m_clImageQueue.pop_front();
		}
		pThis->m_pMutex->Leave();
		if(pCurrent) {
			bEmpty = false;
			sprintf(szFilename, "%s/frame%06d.tga", pThis->m_szSavePath, pThis->m_iFrameNr++);
			C_TargaImg::SaveCompressed(szFilename, pCurrent, true);
			delete pCurrent;
		} else {
			mssleep(5);
			bEmpty = true;
		}
	}
	return THREAD_RET_0;
}

C_MovieMaker::C_MovieMaker(char *szSavePath, C_GraphWrapper *pGraph, C_Sound *pSound)
{
	//we start each session fresh in a new directory named after date and time
	// (single frames will also be stored in a new directory, which is not optimal but...)
	m_iFrameNr = 0;
	char szSessionDir1[48], szSessionDir2[48];
	C_Logger::GetDateString(szSessionDir1);
	C_Logger::GetTimeString(szSessionDir2, false, false);
	sprintf(m_szSavePath, "%s/%s_%s", szSavePath, szSessionDir1, szSessionDir2);
	C_Resource::CreateDir(m_szSavePath);

	m_bDone = false;
	m_pGraph = pGraph;
	m_pSound = pSound;
	m_pMutex = new C_Mutex();
	m_pThread = new C_Thread(WorkThread, this);
	if(m_pSound) {
		char szFilename[MAX_PATH];
		sprintf(szFilename, "%s/audio.wav", m_szSavePath);
		m_pSound->Init(szFilename);
	}
	m_bStopNeeded = false;
	m_pTimer = new C_Timer();
	m_iSleepTime = -1;
	m_iFrameNrBase = m_iFrameNr;
	m_dFPS = 30.0;
}

C_MovieMaker::~C_MovieMaker()
{
	if(m_pSound) m_pSound->Init(NULL);
	m_bDone = true;
	m_pThread->WaitForThreadExit();
	delete m_pThread;
	delete m_pMutex;
	if(m_bStopNeeded) Stop();
	delete m_pTimer;
}

void C_MovieMaker::NextFrame(int iNumTimesSameFrame)
{
	//get screen
	C_Image *pImageObj = m_pGraph->GetRenderedAsImage();
	//enqueue image to be saved
	m_pMutex->Enter();
	if(iNumTimesSameFrame>1) {
		for(int i=1; i<iNumTimesSameFrame; i++) {
			C_Image *pImageObj2 = new C_Image(pImageObj);
			m_clImageQueue.push_back(pImageObj2);
		}
	}
	m_clImageQueue.push_back(pImageObj);
	m_pMutex->Leave();
	m_bStopNeeded = true;
}

void C_MovieMaker::FrameRateLimit(double dFPS)
{
	m_dFPS = dFPS;
	if(m_iSleepTime==-1) m_iSleepTime = (int)(1000/dFPS)-10; //guess some initial value
	//limit frame rate to that wanted by sleeping
	//how accurate this frame rate limiter is depends on the resolution of sleep which depends on the computer and os
	double dFrameTime = m_pTimer->GetInterval(); //last loop time (including previous sleep)
	double dWantedTime = 1000.0/dFPS;
	if(dWantedTime>dFrameTime+1 && m_iSleepTime<500) m_iSleepTime++;
	else if(dWantedTime<dFrameTime-1 && m_iSleepTime>0) m_iSleepTime--;
	if(m_iSleepTime>0) mssleep(m_iSleepTime); //reduce frame rate to ~i_dFPS
}

void C_MovieMaker::SingleFrame()
{
	NextFrame(1);
	m_bStopNeeded = false;
}

void C_MovieMaker::Stop()
{
	char szSaveFile[MAX_PATH];
	char szSound[128];
	FILE *pFile = NULL;
	//get length of audio to generate FPS for perfect sound sync
	double dFPS = m_dFPS;
	if(m_pSound) {
		sprintf(szSaveFile, "%s/audio.wav", m_szSavePath);
		C_Resource *pAudio = new C_Resource();
		pAudio->SetFilename(szSaveFile);
		uint32_t iAvgBytesPerSec, iAudioDataSize = pAudio->GetSize()-44;
		pAudio->Seek(28, SEEK_SET);
		pAudio->Read(&iAvgBytesPerSec, sizeof(iAvgBytesPerSec));
		delete pAudio;
		double dAudioLengthSec = (double)iAudioDataSize/(double)iAvgBytesPerSec;
		if(dAudioLengthSec>0.0) dFPS = (m_iFrameNr-m_iFrameNrBase)/dAudioLengthSec;
	}
	//save command file (works with mencoder 4.2.5)
	sprintf(szSaveFile, "%s/make_video.bat", m_szSavePath);
	if(m_pSound) strcpy(szSound, "-audiofile audio.wav -oac mp3lame");
	else strcpy(szSound, "-nosound");
	pFile = fopen(szSaveFile, "wt");
	if(pFile) {
		fprintf(pFile, "REM file to generate divx or mp42 avi file from all .tga files in the directory together with audio.wav\r\n");
		fprintf(pFile, "pushd \"%%~dp0\"\r\n");
		int iBitrate = (int)(((50 * dFPS * m_pGraph->GetModeWidth() * m_pGraph->GetModeHeight()) / 256) *1.90);
		fprintf(pFile, "REM divx mpeg4:\r\n");
		fprintf(pFile, "mencoder mf://*.tga -mf fps=%.5lf:type=tga -MC 0 %s -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:vhq:trell:autoaspect:vbitrate=%d -o output.avi\r\n", dFPS, szSound, iBitrate);
		fprintf(pFile, "REM msmpeg4: (plays in windows without codecpack)\r\n");
		fprintf(pFile, "REM mencoder mf://*.tga -mf fps=%.5lf:type=tga -MC 0 %s -ovc lavc -lavcopts vcodec=msmpeg4v2:mbd=2:vhq:trell:autoaspect:vbitrate=%d:vpass=1 -o output_tmp.avi\r\n", dFPS, szSound, iBitrate);
		fprintf(pFile, "REM mencoder mf://*.tga -mf fps=%.5lf:type=tga -MC 0 %s -ovc lavc -lavcopts vcodec=msmpeg4v2:mbd=2:vhq:trell:autoaspect:vbitrate=%d:vpass=2 -o output.avi\r\n", dFPS, szSound, iBitrate);
		fprintf(pFile, "popd\r\n");
		fclose(pFile);
	}
	m_bStopNeeded = false;
}
