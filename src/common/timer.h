// C_Timer

#ifndef _TIMER_H
#define _TIMER_H

#include "common.h"

class C_Timer
{
public:
	C_Timer();
	~C_Timer();
	double GetInterval();
	//returns time in milliseconds since last call to GetInterval()
	double PeekInterval();
	//returns time in milliseconds since last call to GetInterval()

	void Reset();
	//sets time counter to 0
private:
#ifdef WIN32
	int m_iHiresTimer;
	int64_t m_qFreq, m_qLastTime;
#else
	timeval m_stLastTime;
#endif

	double m_dPeekTime;  //milliseconds 
};

#endif
