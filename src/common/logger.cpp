// C_Logger

//a logger class that can append text to a file and/or stdout.
// it filters text depending on log level type.

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef WIN32
#include <share.h>
#endif

#define MAXTEXT 5*512

C_Logger::C_Logger()
{
	m_szFilename = NULL;

	m_iLogLevel = LOG_ERROR;
	m_iOutLevel = LOG_TO_STDOUT;
	m_bCreatedConsole = false;
	m_pMutex = new C_Mutex();
}

C_Logger::~C_Logger()
{
	delete[] m_szFilename;

	if(m_bCreatedConsole) {
#ifdef WIN32
		FreeConsole();
#endif
	}

	delete m_pMutex;
}

void C_Logger::SetLogLevel(int iLogLevel)
{
	m_iLogLevel = iLogLevel & LOG_ALL;
}

void C_Logger::SetOutLevel(int iOutLevel)
{
	m_iOutLevel = iOutLevel & (LOG_TO_FILE | LOG_TO_STDOUT);

#ifdef WIN32
	if((m_iOutLevel & LOG_OPEN_CONSOLE) && AllocConsole()) {
		FILE *pIgnored = NULL;
		freopen_s(&pIgnored, "CONOUT$", "wt", stdout);
		SetConsoleTitle((LPCTSTR)"Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
		m_bCreatedConsole = true;
	}
#endif
}

void C_Logger::AssignFile(const char *szFilename)
{
	m_pMutex->Enter();
	delete[] m_szFilename;
	if(szFilename == NULL) m_szFilename = NULL;
	else {
		int iLen = (int)strlen(szFilename)+1;
		m_szFilename = new char[iLen];
		strncpy(m_szFilename, szFilename, iLen);
	}
	m_pMutex->Leave();
}

void C_Logger::Print(int iLevel, const char *szFmt, ...)
{
	if(!m_iLogLevel || !m_iOutLevel || !szFmt) return; //fast pre-check
	if((iLevel & m_iLogLevel)==0) return; //level check

	char szText[MAXTEXT];
	va_list stPList;
	va_start(stPList, szFmt);
	vsnprintf(szText, sizeof(szText), szFmt, stPList);
	szText[sizeof(szText)-1] = 0;
	va_end(stPList);

	//char for log type
	int iLevelStringIndex = 0; //assume LOG_ERROR
	const char *szLevelString = "EWIT";
	if(iLevel & LOG_WARN) iLevelStringIndex = 1;
	if(iLevel & LOG_INFO) iLevelStringIndex = 2;
	if(iLevel & LOG_TRACE) iLevelStringIndex = 3;

	//time string
	char szTime[24];
	C_Logger::GetTimeString(szTime, true, true);
	//log to file if assigned
	if(m_iOutLevel & LOG_TO_FILE) {
		if(m_szFilename) {
			m_pMutex->Enter();
#ifdef WIN32
			FILE *pFile = _fsopen(m_szFilename, "at", _SH_DENYNO);
#else
			FILE *pFile = fopen(m_szFilename, "at");
#endif
			if(pFile) {
				fprintf(pFile, "[%c %s] %s", szLevelString[iLevelStringIndex], szTime, szText);
				fclose(pFile);
			}
			m_pMutex->Leave();
		}
	}

	//log to stdout
	if(m_iOutLevel & LOG_TO_STDOUT) printf("[%c %s] %s", szLevelString[iLevelStringIndex], szTime, szText);
}

void C_Logger::PrintToFile(char *szFilename, const char *szFmt, ...)
{
	if(!szFmt) return;

	char szText[MAXTEXT];
	va_list stPList;
	va_start(stPList, szFmt);
	vsnprintf(szText, sizeof(szText), szFmt, stPList);
	szText[sizeof(szText)-1] = 0;
	va_end(stPList);

#ifdef WIN32
	FILE *pFile = _fsopen(szFilename, "at", _SH_DENYNO);
#else
	FILE *pFile = fopen(szFilename, "at");
#endif
	if(pFile) {
		fprintf(pFile, "%s", szText);
		fclose(pFile);
	}
}

void C_Logger::GetTimeString(char *o_szTime, bool bMilli, bool bColon)
{
#ifdef WIN32
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	if(bColon) {
		if(bMilli) sprintf(o_szTime, "%02d:%02d:%02d.%03d", stTime.wHour, stTime.wMinute, stTime.wSecond, stTime.wMilliseconds);
		else       sprintf(o_szTime, "%02d:%02d:%02d", stTime.wHour, stTime.wMinute, stTime.wSecond);
	} else {
		if(bMilli) sprintf(o_szTime, "%02d%02d%02d.%03d", stTime.wHour, stTime.wMinute, stTime.wSecond, stTime.wMilliseconds);
		else       sprintf(o_szTime, "%02d%02d%02d", stTime.wHour, stTime.wMinute, stTime.wSecond);
	}
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm *now = localtime(&tv.tv_sec);
	if(bColon) {
		if(bMilli) sprintf(o_szTime, "%02d:%02d:%02d.%03d", now->tm_hour, now->tm_min, now->tm_sec, (int)(tv.tv_usec/1000));
		else       sprintf(o_szTime, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
	} else {
		if(bMilli) sprintf(o_szTime, "%02d%02d%02d.%03d", now->tm_hour, now->tm_min, now->tm_sec, (int)(tv.tv_usec/1000));
		else       sprintf(o_szTime, "%02d%02d%02d", now->tm_hour, now->tm_min, now->tm_sec);
	}
#endif
}

void C_Logger::GetDateString(char *o_szDate, bool bDash)
{
#ifdef WIN32
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	if(bDash) sprintf(o_szDate, "%04d-%02d-%02d", stTime.wYear, stTime.wMonth, stTime.wDay);
	else      sprintf(o_szDate, "%04d%02d%02d", stTime.wYear, stTime.wMonth, stTime.wDay);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm *now = localtime(&tv.tv_sec);
	if(bDash) sprintf(o_szDate, "%04d-%02d-%02d", now->tm_year, now->tm_mon, now->tm_mday);
	else      sprintf(o_szDate, "%04d%02d%02d", now->tm_year, now->tm_mon, now->tm_mday);
#endif
}
