#ifndef FSSIMPLEWINDOW_IS_INCLUDED
#define FSSIMPLEWINDOW_IS_INCLUDED

// With the move from native to SDL2 this file only contents that remain is some dialog/directory functions
// written by rh_galaxy

struct file_dialog_info
{
	char *szStartPath;
	int iNumFilters; //min 0, max 8
	char *szFilters[8];
};


#ifdef __cplusplus
// This file needs to be included from Objective-C code
// so C++ specific declaration must be enclosed by #ifdef __cplusplus

void FsChangeToProgramDir();
void FsGetAppDataDir(char* o_szPath, int iLength, const char *szOverrideBundleId);

bool FsFileOpenDialog(char *szPath, int iLength, struct file_dialog_info *pFI);
bool FsFileSaveDialog(char *szPath, int iLength, struct file_dialog_info *pFI);
bool FsGetClipboardText(char *szText, int iLength);

bool FsMessageBox(char* szHeader, char* szMessage);

#endif

#endif
