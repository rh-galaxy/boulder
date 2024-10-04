#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

extern "C" void FsChangeToProgramDirC();
extern "C" void FsGetAppDataDirC(char* o_szPath, int iLength, const char *szOverrideBundleId);

extern "C" bool FsFileOpenDialogC(char *szPath, int iLength, struct file_dialog_info *pFI);
extern "C" bool FsFileSaveDialogC(char *szPath, int iLength, struct file_dialog_info *pFI);
extern "C" bool FsGetClipboardTextC(char *szText, int iLength);

extern "C" int FsMessageBoxC(char* szHeader, char* szMessage);

void FsChangeToProgramDir()
{
	FsChangeToProgramDirC();
}

void FsGetAppDataDir(char* o_szPath, int iLength, const char *szOverrideBundleId)
{
	FsGetAppDataDirC(o_szPath, iLength, szOverrideBundleId);
}

bool FsFileOpenDialog(char *szPath, int iLength, struct file_dialog_info *pFI)
{
	return FsFileOpenDialogC(szPath, iLength, pFI);
}

bool FsFileSaveDialog(char *szPath, int iLength, struct file_dialog_info *pFI)
{
	return FsFileSaveDialogC(szPath, iLength, pFI);
}

bool FsGetClipboardText(char *szText, int iLength)
{
	return FsGetClipboardTextC(szText, iLength);
}

void FsMessageBox(char* szHeader, char* szMessage)
{
	FsMessageBoxC(szHeader, szMessage);
}
