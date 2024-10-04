// C_Global

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "common.h"

class C_Global
{
public:
	static void Init(const char *szDataFile, const char *szUserDirName);

	static char* PathCombine(char *szFile, char *szPath);

	//Init() must run before the paths are changed to work
	static char szAppDataPath[MAX_PATH]; //directory to save data for this (hiscore/config)
	static char szCurrentPath[MAX_PATH]; //current dir, not to be used really
	//static char szUserPath[MAX_PATH];
	static char szExePath[MAX_PATH];
	static char szDataPath[MAX_PATH]; //may be read only, 'data' directory

	//helper functions
	static char *GetNextLineAndCommand(FILE *pFile, char *o_szLine, char *o_szCommand);
	static bool GetNextLineAndCommand(char *szSrc, char *o_szLine, char *o_szCommand, int *io_iNextLineIndex);
	static bool GetWithin(char *szString, char cCh);
	static char *GetPositionAfter(char *szString, char cCh, int iIndex);

	//os gui
	static bool OpenDialog(char *o_szFile, int iSize, char *szInitialPath, char *szzFilter, char *szzFilterDescription);
	static bool SaveDialog(char *o_szFile, int iSize, char *szInitialPath, char *szzFilter, char *szzFilterDescription);
	static bool GetClipboardBuffer(char *o_szText, int iSize);

	static bool OpenBrowser(const char *szURL);
	
private:
	static bool FindDataPath(const char *szDataFile);
	static void FixPath(char *szPath);

	static char szTemp[MAX_PATH];
};

#endif
