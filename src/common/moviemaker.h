// C_MovieMaker

#ifndef _MOVIEMAKER_H
#define _MOVIEMAKER_H

#include "common.h"
#include "graph.h"
#include "soundfx.h"
#include "timer.h"

#include <deque>

class C_MovieMaker {
public:
	C_MovieMaker(char *szSavePath, C_GraphWrapper *pGraph, C_Sound *pSound);
	~C_MovieMaker();
	void SingleFrame();
	void NextFrame(int iNumTimesSameFrame = 1);
	void FrameRateLimit(double dFPS);
	void Stop(); //writes a .bat file to generate movie
private:
	static THREAD_RET WorkThread(void *pMovieMaker);
	C_GraphWrapper *m_pGraph;
	C_Sound        *m_pSound;
	std::deque<C_Image*>  m_clImageQueue;
	C_Mutex  *m_pMutex;
	C_Thread *m_pThread;
	volatile bool m_bDone;
	char m_szSavePath[MAX_PATH];

	bool m_bStopNeeded;
	int m_iFrameNr;
	int m_iFrameNrBase;
	double m_dFPS;

	C_Timer *m_pTimer;
	int m_iSleepTime;
};

#endif
