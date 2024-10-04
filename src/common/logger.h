// C_Logger

#ifndef __LOGGER_H
#define __LOGGER_H

#include "common.h"

#define LOG_NONE  0x0000
#define LOG_ERROR 0x0001
#define LOG_WARN  0x0002
#define LOG_INFO  0x0004
#define LOG_TRACE 0x0008
#define LOG_ALL   0x00FF

#define LOG_TO_FILE      0x1000
#define LOG_TO_STDOUT    0x2000
#define LOG_OPEN_CONSOLE 0x4000

class C_Logger
{
public:
	C_Logger();
	~C_Logger();

	void SetLogLevel(int iLogLevel); //default is LOG_ERROR
	void SetOutLevel(int iOutLevel); //default is LOG_TO_STDOUT
	void AssignFile(const char *szFilename);

	void Print(int iLevel, const char *szFmt, ...);
	static void PrintToFile(char *szFilename, const char *szFmt, ...);

	static void GetTimeString(char *o_szTime, bool bMilli, bool bColon = true);
	static void GetDateString(char *o_szDate, bool bDash = true);

private:
	int   m_iLogLevel;
	int   m_iOutLevel;
	char  *m_szFilename;
	bool  m_bCreatedConsole;

	//thread safety
	C_Mutex *m_pMutex;
};

#endif //__LOGGER_H
