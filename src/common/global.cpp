// C_Global

//class with global helpers with OS specific impl
// - special directories handling
// - open file dialog (mac and windows, linux has a way of doing it with /usr/bin/zenity)
// - get clipboard text (mac and windows, linux via xclip)
// - helper methods to read text files in the format used in the game

#include "global.h"
#include "fileresource.h"
#include <string.h>

#if defined WIN32
 #include <shlobj.h>
#elif defined __APPLE__
 #include <mach-o/dyld.h>
 #ifndef NOGUI
  #include "fssimplewindow.h"
 #endif
#endif

char C_Global::szAppDataPath[MAX_PATH];
char C_Global::szCurrentPath[MAX_PATH];
//char C_Global::szUserPath[MAX_PATH];
char C_Global::szExePath[MAX_PATH];
char C_Global::szDataPath[MAX_PATH];
char C_Global::szTemp[MAX_PATH];

char *C_Global::GetNextLineAndCommand(FILE *pFile, char *o_szLine, char *o_szCommand)
{
	o_szCommand[0] = 0; //reset command
	char *szRet = fgets(o_szLine, 512, pFile);
	if (szRet) sscanf(o_szLine, "%s", o_szCommand); //COMMAND
	return szRet;
}

bool C_Global::GetNextLineAndCommand(char *szSrc, char *o_szLine, char *o_szCommand, int *io_iNextLineIndex)
{
	int iStartLineIndex = 0;
	int iEndLineIndex = 0;
	bool bEnd = false;

	//get line
	do {
		char iCur = szSrc[iEndLineIndex];

		if (iCur == '\n' || iCur == '\r' || iCur == 0) {
			if (iStartLineIndex < iEndLineIndex || iCur == 0) {
				bEnd = true;
			}
			else {
				iStartLineIndex++;
				iEndLineIndex++;
			}
		}
		else iEndLineIndex++;
	} while (!bEnd);

	int iLen = iEndLineIndex - iStartLineIndex;
	if (iLen <= 0) return false;

	memcpy(o_szLine, szSrc + iStartLineIndex, iLen);
	o_szLine[iLen] = 0; //zero terminate
	*io_iNextLineIndex += iEndLineIndex;

	//get command from line
	if (o_szCommand) {
		bEnd = false;
		iStartLineIndex = 0;
		iEndLineIndex = 0;
		do {
			char iCur = o_szLine[iEndLineIndex];

			if (iCur == ' ' || iCur == '\t' || iCur == 0) {
				if (iStartLineIndex < iEndLineIndex) {
					bEnd = true;
				}
				else {
					iStartLineIndex++;
					iEndLineIndex++;
				}
			}
			else iEndLineIndex++;
		} while (!bEnd);

		iLen = iEndLineIndex - iStartLineIndex;
		if (iLen <= 0) iLen = 0;
		else memcpy(o_szCommand, o_szLine + iStartLineIndex, iLen);
		o_szCommand[iLen] = 0; //zero terminate
	}

	return true;
}

bool C_Global::GetWithin(char *szString, char cCh)
{
	int i = 0, iStart = 0;

	while (szString[i] != cCh && szString[i] != 0) i++;
	if (szString[i] == 0) return false;
	i++;

	while (szString[i] != cCh && szString[i] != 0) szString[iStart++] = szString[i++];
	if (szString[i] != cCh) return false;
	szString[iStart] = 0;
	return true;
}

char *C_Global::GetPositionAfter(char *szString, char cCh, int iIndex)
{
	int j, i = 0;
	for (j = 0; j < iIndex; j++) {
		while (szString[i] != cCh && szString[i] != 0) i++;
		if (szString[i] == 0) return NULL;
		i++;
	}
	return szString + i;
}

//removes ../ and set the correct path, this is because in windows
// ../ does not work everywhere (in the GetOpenFileName for example)
void C_Global::FixPath(char *szPath)
{
	char *szPos = szPath;
	while (szPos) {
		szPos = strstr(szPath, "..");
		if (szPos) {
			char *szClipPosStart = szPos;
			char *szClipPosEnd = szPos + 3;

			int iCount = 0;
			while (szClipPosStart > szPath) {
				szClipPosStart--;
				if (szClipPosStart[0] == '\\' || szClipPosStart[0] == '/') {
					iCount++;
					if (iCount == 2) {
						szClipPosStart++;
						break;
					}
				}
			}
			if (iCount == 2) {
				//do the string clipping
				strcpy(szClipPosStart, szClipPosEnd);
			}
		}
	}
}

bool C_Global::FindDataPath(const char *szDataFile)
{
	char szPath[MAX_PATH];
	if (szDataFile && szDataFile[0]) {
		//first try exe_path, to find data when normally installed
		strcpy(szDataPath, szExePath);
		sprintf(szPath, "%s/%s", szExePath, szDataFile);
		FILE *pFile = fopen(szPath, "rb");
		if (!pFile) {
			//try exe_path/../data, to find data when running from the project/exe/os/ directory
			sprintf(szDataPath, "%s/../data", szExePath);
			FixPath(szDataPath);
			sprintf(szPath, "%s/%s", szDataPath, szDataFile);
			pFile = fopen(szPath, "rb");
		}
		if (!pFile) {
			//try exe_path/data, to find data when running certain versions (mac)
			sprintf(szDataPath, "%s/data", szExePath);
			sprintf(szPath, "%s/%s", szDataPath, szDataFile);
			pFile = fopen(szPath, "rb");
		}
		if (!pFile) {
			//try current_directory/../data, to find data if running debug from visual studio
			sprintf(szDataPath, "%s/../data", szCurrentPath);
			FixPath(szDataPath);
			sprintf(szPath, "%s/%s", szDataPath, szDataFile);
			pFile = fopen(szPath, "rb");
		}
		if (!pFile) {
			//try current_directory/data, to find data if running debug from visual studio
			sprintf(szDataPath, "%s/data", szCurrentPath);
			FixPath(szDataPath);
			sprintf(szPath, "%s/%s", szDataPath, szDataFile);
			pFile = fopen(szPath, "rb");
		}
		if (!pFile) {
			//try current_directory/
			sprintf(szDataPath, "%s/", szCurrentPath);
			FixPath(szDataPath);
			sprintf(szPath, "%s/%s", szDataPath, szDataFile);
			pFile = fopen(szPath, "rb");
		}
		if (!pFile) {
			//try exe_path/-Resources (Mac sandbox)
			sprintf(szDataPath, "%s/../Contents/Resources/data", szExePath);
			FixPath(szDataPath);
			sprintf(szPath, "%s/%s", szDataPath, szDataFile);
			pFile = fopen(szPath, "rb");
		}
		if (!pFile) {
			szDataPath[0] = 0;
		}
		if (pFile) fclose(pFile);
	}
	return szDataPath[0] != 0;
}


char* C_Global::PathCombine(char *szFile, char *szPath)
{
	sprintf(szTemp, "%s/%s", szPath, szFile);
	return szTemp;
}

void C_Global::Init(const char *szDataFile, const char *szUserDirName)
{
	bool bCreateUserPath = false;

	srand((uint32_t)time(NULL)); //init random numbers once

	//make the best of it if finding fails
	strcpy(szCurrentPath, "./");
	strcpy(szAppDataPath, "./");
	//strcpy(szUserPath, "./");
	strcpy(szExePath, "./");
	strcpy(szDataPath, ""); //not used/found

#if defined WIN32
	//find exe path
	char szPath[MAX_PATH];
	int iStringLen = GetModuleFileName(NULL, szExePath, sizeof(szExePath));
	bool bFound = iStringLen > 0 && iStringLen < sizeof(szExePath);
	if (bFound) {
		//locate the last "\" or "/" in the filename
		char* szLastSlash = strrchr(szExePath, '\\');
		if (!szLastSlash) szLastSlash = strrchr(szExePath, '/');
		if (szLastSlash) {
			*szLastSlash = 0; //cut on last "\" or "/"
		}
	}

	//find current path
	iStringLen = GetCurrentDirectory(sizeof(szCurrentPath), szCurrentPath);
	bFound = iStringLen > 0 && iStringLen < sizeof(szCurrentPath);

	//find appdata path
	if (!SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, false)) {
		//make the best of it
		strcpy(szAppDataPath, szExePath);
	}
	else {
		sprintf(szAppDataPath, "%s/%s", szPath, szUserDirName);
		bCreateUserPath = true;
	}

	//find user path
	//use "My Documents" folder to support none admin users
	/*if(!SHGetSpecialFolderPath(NULL, szPath, CSIDL_PERSONAL, false)) {
		//make the best of it
		strcpy(szUserPath, szExePath);
	}
	else {
		sprintf(szUserPath, "%s/%s", szPath, szUserDirName);
		bCreateUserPath = true;
	}*/
#else

#if defined __APPLE__
	//find exe path
	uint32_t iSize = sizeof(szExePath);
	if (_NSGetExecutablePath(szExePath, &iSize) == 0) {
		//locate the last "/" in the filename
		char* szLastSlash = NULL, *szTmp = szExePath;
		while ((szTmp = strstr(szTmp + 1, "/")) != NULL) szLastSlash = szTmp;
		if (szLastSlash) {
			*szLastSlash = 0; //cut on last ".app/"
			szLastSlash = strrchr(szExePath, '/');
			if (szLastSlash) {
				*szLastSlash = 0; //cut on last "/"
			}
		}
	}

	//find appdata path
	FsGetAppDataDir(szAppDataPath, MAX_PATH, szUserDirName);

#elif defined __linux__
	//find exe path
	if (readlink("/proc/self/exe", szExePath, sizeof(szExePath)) > 0) {
		//locate the last "/" in the filename
		char* szLastSlash = strrchr(szExePath, '/');
		if (szLastSlash) {
			*szLastSlash = 0; //cut on last "/"
		}
	}

	//find appdata path
	strcpy(szAppDataPath, szExePath);
#endif

	//find current path
	getcwd(szCurrentPath, sizeof(szCurrentPath));

	//find user path
	/*char *szHome = getenv("HOME");
	if(szHome && szUserDirName) {
		snprintf(szUserPath, sizeof(szUserPath), "%s/%s", szHome, szUserDirName);
		bCreateUserPath = true;
	}*/
#endif

	//find data path, relative to the exe path or current directory
	FindDataPath(szDataFile);

#ifdef WIN32
	//forward slashes are mostly ok on windows,
	// but some functions are not happy with them (the opendialog for example)
	char *szSlash;
	while (szSlash = strchr(szAppDataPath, '/')) *szSlash = '\\';
	while (szSlash = strchr(szCurrentPath, '/')) *szSlash = '\\';
	//while(szSlash = strchr(szUserPath, '/')) *szSlash = '\\';
	while (szSlash = strchr(szExePath, '/')) *szSlash = '\\';
	while (szSlash = strchr(szDataPath, '/')) *szSlash = '\\';
#endif

	//create user dirs if they don't exist
	if (bCreateUserPath) {
		C_Resource::CreateDir(szAppDataPath);
		//C_Resource::CreateDir(szUserPath);
	}
}

#ifndef NOGUI

#define MAX_FILE_FILTERS 8
bool C_Global::OpenDialog(char *o_szFile, int iSize, char *szInitialPath, char *szzFilter, char *szzFilterDescription)
{
	bool bResult = false;
	int i, iNumFilters = 0;
	char *szFilters[MAX_FILE_FILTERS];
	char *szFilterDescs[MAX_FILE_FILTERS];

	memset(&szFilterDescs, 0, sizeof(szFilterDescs));

	if (!o_szFile || iSize < 2) return false;

	if (szzFilter) {
		char *szTmp_1 = szzFilter;
		char *szTmp_2 = szzFilterDescription;
		do { //at least one
			szFilters[iNumFilters] = szTmp_1;
			if (szTmp_2 && *szTmp_2 != 0) {
				szFilterDescs[iNumFilters] = szTmp_2;
				szTmp_2 = szTmp_2 + strlen(szTmp_2) + 1;
			}
			szTmp_1 += strlen(szTmp_1) + 1; //next string
			iNumFilters++;
			if (iNumFilters > MAX_FILE_FILTERS - 1) return false;
		} while (*szTmp_1 != 0);
	}

#if defined WIN32
	if (szInitialPath && (int)(strlen(szInitialPath) + 32) > iSize) return false;
	if (iNumFilters <= 0) sprintf(o_szFile, "%s\\*", szInitialPath);
	else sprintf(o_szFile, "%s\\*.%s", szInitialPath, szzFilter);

	OPENFILENAME stOFN;
	char szFilter[1024];
	int iPos = 0;
	szFilter[0] = 0;
	if (iNumFilters <= 0) iPos++;
	for (i = 0; i < iNumFilters; i++) {
		int iLen = (int)strlen(szFilters[i]);
		if (szFilterDescs[i]) {
			int iLen2 = (int)strlen(szFilterDescs[i]);
			memcpy(szFilter + iPos, szFilterDescs[i], iLen2 + 1);
			iPos += iLen2 + 1;
		}
		else {
			memcpy(szFilter + iPos, szFilters[i], iLen + 1);
			iPos += iLen + 1;
		}
		sprintf(szFilter + iPos, "*.%s", szFilters[i]);
		iLen = (int)strlen(szFilter + iPos);
		iPos += iLen + 1;
	}
	szFilter[iPos] = 0; //double termination

	memset(&stOFN, 0, sizeof(OPENFILENAME));
	stOFN.lStructSize = sizeof(OPENFILENAME);
	stOFN.hwndOwner = NULL;
	stOFN.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES; //OFN_FILEMUSTEXIST
	stOFN.lpstrFile = o_szFile;
	stOFN.nMaxFile = iSize;
	if (iNumFilters > 0) {
		stOFN.lpstrFilter = szFilter;
		stOFN.nFilterIndex = 1;
	}

	bResult = GetOpenFileName(&stOFN) != 0;
#elif defined __APPLE__
	struct file_dialog_info stFI;
	stFI.szStartPath = szInitialPath;
	stFI.iNumFilters = iNumFilters;
	for (i = 0; i < iNumFilters; i++) {
		stFI.szFilters[i] = szFilters[i];
	}
	bResult = FsFileOpenDialog(o_szFile, iSize, &stFI);
#elif defined __linux__
	const char zenityP[] = "/usr/bin/zenity";
	char szCall[2048];

	sprintf(szCall, "%s  --file-selection --modal --title=\"Select file\" --file-filter=*.%s --filename=\"%s/*.%s\" ", zenityP, szFilters[0], szInitialPath, szFilters[0]);
	FILE *f = popen(szCall, "r");
	if (fgets(o_szFile, iSize, f)) {
		char *szLast;
		if (szLast = strrchr(o_szFile, '\r')) *szLast = 0;
		if (szLast = strrchr(o_szFile, '\n')) *szLast = 0;
	}
	int ret = pclose(f);
#endif

	return bResult;
}

bool C_Global::SaveDialog(char *o_szFile, int iSize, char *szInitialPath, char *szzFilter, char *szzFilterDescription)
{
	bool bResult = false;
	int i, iNumFilters = 0;
	char *szFilters[MAX_FILE_FILTERS];
	char *szFilterDescs[MAX_FILE_FILTERS];

	memset(&szFilterDescs, 0, sizeof(szFilterDescs));

	if (!o_szFile || iSize < 2) return false;

	if (szzFilter) {
		char *szTmp_1 = szzFilter;
		char *szTmp_2 = szzFilterDescription;
		do { //at least one
			szFilters[iNumFilters] = szTmp_1;
			if (szTmp_2 && *szTmp_2 != 0) {
				szFilterDescs[iNumFilters] = szTmp_2;
				szTmp_2 = szTmp_2 + strlen(szTmp_2) + 1;
			}
			szTmp_1 += strlen(szTmp_1) + 1; //next string
			iNumFilters++;
			if (iNumFilters > MAX_FILE_FILTERS - 1) return false;
		} while (*szTmp_1 != 0);
	}

#if defined WIN32
	if (szInitialPath && (int)(strlen(szInitialPath) + 32) > iSize) return false;
	if (iNumFilters <= 0) sprintf(o_szFile, "%s\\*", szInitialPath);
	else sprintf(o_szFile, "%s\\*.%s", szInitialPath, szzFilter);

	OPENFILENAME stOFN;
	char szFilter[1024];
	int iPos = 0;
	szFilter[0] = 0;
	if (iNumFilters <= 0) iPos++;
	for (i = 0; i < iNumFilters; i++) {
		int iLen = (int)strlen(szFilters[i]);
		if (szFilterDescs[i]) {
			int iLen2 = (int)strlen(szFilterDescs[i]);
			memcpy(szFilter + iPos, szFilterDescs[i], iLen2 + 1);
			iPos += iLen2 + 1;
		}
		else {
			memcpy(szFilter + iPos, szFilters[i], iLen + 1);
			iPos += iLen + 1;
		}
		sprintf(szFilter + iPos, "*.%s", szFilters[i]);
		iLen = (int)strlen(szFilter + iPos);
		iPos += iLen + 1;
	}
	szFilter[iPos] = 0; //double termination

	memset(&stOFN, 0, sizeof(OPENFILENAME));
	stOFN.lStructSize = sizeof(OPENFILENAME);
	stOFN.hwndOwner = NULL;
	stOFN.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES; //OFN_FILEMUSTEXIST
	stOFN.lpstrFile = o_szFile;
	stOFN.nMaxFile = iSize;
	if (iNumFilters > 0) {
		stOFN.lpstrFilter = szFilter;
		stOFN.nFilterIndex = 1;
	}

	bResult = GetOpenFileName(&stOFN) != 0;
#elif defined __APPLE__
	struct file_dialog_info stFI;
	stFI.szStartPath = szInitialPath;
	stFI.iNumFilters = iNumFilters;
	for (i = 0; i < iNumFilters; i++) {
		stFI.szFilters[i] = szFilters[i];
	}
	bResult = FsFileSaveDialog(o_szFile, iSize, &stFI);
#elif defined __linux__
	const char zenityP[] = "/usr/bin/zenity";
	char szCall[2048];

	sprintf(szCall, "%s  --file-selection --modal --title=\"Select file\" --file-filter=*.%s --filename=\"%s/*.%s\" ", zenityP, szFilters[0], szInitialPath, szFilters[0]);
	FILE *f = popen(szCall, "r");
	if (fgets(o_szFile, iSize, f)) {
		char *szLast;
		if (szLast = strrchr(o_szFile, '\r')) *szLast = 0;
		if (szLast = strrchr(o_szFile, '\n')) *szLast = 0;
	}
	int ret = pclose(f);
#endif

	return bResult;
}

bool C_Global::GetClipboardBuffer(char *o_szText, int iSize)
{
	bool bResult = false;
#if defined WIN32
	HANDLE hClip;
	if (OpenClipboard(NULL)) {
		hClip = GetClipboardData(CF_TEXT);
		if (hClip != NULL) {
			char *szText = (char*)hClip;
			strncpy(o_szText, szText, iSize);
			o_szText[iSize - 1] = 0;
			bResult = true;
		}
		CloseClipboard();
	}
#elif defined __APPLE__
	bResult = FsGetClipboardText(o_szText, iSize);
#elif defined __linux__
	//need: sudo apt-get install xclip
	FILE *hPipe = (FILE *)popen("xclip -o", "r");
	if (hPipe) {
		int r = fread(o_szText, 1, iSize, hPipe);
		if (r > 0) o_szText[r] = 0;
		bResult = r > 0;
		pclose(hPipe);
	}
#endif
	return bResult;
}

bool C_Global::OpenBrowser(const char *szURL)
{
	bool bResult = false;
#if defined WIN32
	ShellExecute(NULL, "open", szURL, NULL, NULL, SW_SHOWNORMAL);
#elif defined __APPLE__	
	char szCommand[MAX_PATH];
	sprintf(szCommand, "open %s", szURL);
	bResult = system(szCommand);
#elif defined __linux__
	char szCommand[MAX_PATH];
	sprintf(szCommand, "xdg-open %s", szURL);
	bResult = system(szCommand);
#endif
	return bResult;
}

#endif
