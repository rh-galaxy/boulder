// C_Resource

//class to handle file reading from a single file or a .res file
// (use res_comp to create a compiled resource).
// - file reading from a single file
// - file writing to a single file
// - file reading from a .res file
// - all files (and resource files) must be less than 4 GB
//also handles other file system operations in an OS independent way.
// - directory creation in a given existing directory
// - file search with wild-cards in a given directory (not recursive)

#include "fileresource.h"

C_Resource::C_Resource()
{
	m_bReading = true;
	m_pFileHandle = NULL;
	m_iFileSize = 0;
	m_iFileStart = 0;
}

void C_Resource::Reset()
{
	if(m_pFileHandle) fclose(m_pFileHandle);
	m_pFileHandle = NULL;
	m_iFileSize = 0;
	m_iFileStart = 0;
}

C_Resource::~C_Resource()
{
	Reset();
}

void C_Resource::SetMode(bool bReading)
{
	m_bReading = bReading;
	Reset();
}

bool C_Resource::SetFilename(const char* szFilename, const char* szResname)
{
	if(!m_bReading && szResname) return false; //cannot write to a resource
	if(m_pFileHandle) Reset();

	uint32_t iNumItems, i;
	char     szName[256];

	if(!szResname) { //no resource file, just a file?
		m_pFileHandle = fopen(szFilename, m_bReading ? "rb" : "wb");
		if(m_pFileHandle == NULL) return false; //error opening
	} else {
		m_pFileHandle = fopen(szResname, "rb");
		if(m_pFileHandle == NULL) return false; //error opening
	}
	fseek(m_pFileHandle, 0, SEEK_END);
	m_iFileSize = (uint32_t)ftell(m_pFileHandle); //we will never handle more than 4GB
	fseek(m_pFileHandle, 0, SEEK_SET);
	if(!szResname) return true; //just a normal file that are now open for reading or writing at pos 0

	bool bFound = false;
	if(fread(&iNumItems, 4, 1, m_pFileHandle)==1) {
		for(i=0; i<iNumItems; i++) {
			uint8_t iNameLength = 0;
			fread(&iNameLength, 1, 1, m_pFileHandle);
			memset(szName, 0, sizeof(szName));
			fread(szName, iNameLength, 1, m_pFileHandle);
			fread(&m_iFileStart, 4, 1, m_pFileHandle);
			if(fread(&m_iFileSize, 4, 1, m_pFileHandle)!=1) break;
			if(strcmp(szName, szFilename)==0) {
				fseek(m_pFileHandle, m_iFileStart, SEEK_SET);
				bFound = true; break; //resource found
			}
		}
	}

	if(!bFound) Reset();
	return bFound;
}

bool C_Resource::Read(void *pData, uint32_t iLength)
{
	bool bResult = false;
	if(m_pFileHandle && m_bReading) {
		if(iLength==0) return true;
		if(iLength==0xffffffff) iLength = (m_iFileStart + m_iFileSize)
			- (uint32_t)ftell(m_pFileHandle); //we will never handle more than 4GB
		if(fread(pData, iLength, 1, m_pFileHandle)==1) bResult = true;
	}
	return bResult;
}

bool C_Resource::Write(void *pData, uint32_t iLength)
{
	bool bResult = false;
	if(m_pFileHandle && !m_bReading) {
		if(iLength==0) return true;
		if(fwrite(pData, iLength, 1, m_pFileHandle)==1) bResult = true;
	}
	return bResult;
}

void C_Resource::Seek(uint32_t iPos, int iMode)
{
	if(m_pFileHandle) {
		switch(iMode) {
			case SEEK_SET: fseek(m_pFileHandle, m_iFileStart + iPos, iMode); break;
			case SEEK_CUR: fseek(m_pFileHandle, iPos, iMode); break;
			case SEEK_END: fseek(m_pFileHandle, (m_iFileStart + m_iFileSize) - iPos, SEEK_SET); break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//file search and directory creation
#ifdef WIN32
//windows
struct S_FileSearch
{
	WIN32_FIND_DATA stFindData;
	HANDLE          stFile;
	int             iState; //0 not started, 1st name given
};

void *C_Resource::FileSearchOpen(const char *szPath, const char *szWild)
{
	if(!szPath || !szWild) return NULL;
	char szPathAndWild[MAX_PATH];
	sprintf(szPathAndWild, "%s/%s", szPath, szWild);
	S_FileSearch *pSearch = new S_FileSearch;
	pSearch->iState = 0;
	pSearch->stFile = FindFirstFile(szPathAndWild, &pSearch->stFindData);
	return pSearch;
}

bool C_Resource::FileSearchNext(void *pHandle, char *o_szFilename, int iStringSize)
{
	S_FileSearch *pSearch = (S_FileSearch *)pHandle;
	if(!pSearch || !o_szFilename || pSearch->stFile==INVALID_HANDLE_VALUE) return false;
	if(pSearch->iState==0) {
		pSearch->iState++;
		strncpy(o_szFilename, pSearch->stFindData.cFileName, iStringSize);
		o_szFilename[iStringSize-1] = 0;
		return true;
	}
	bool bResult = FindNextFile(pSearch->stFile, &pSearch->stFindData) ? true : false;
	if(bResult) {
		strncpy(o_szFilename, pSearch->stFindData.cFileName, iStringSize);
		o_szFilename[iStringSize-1] = 0;
	}
	return bResult;
}

void C_Resource::FileSearchClose(void *pHandle)
{
	S_FileSearch *pSearch = (S_FileSearch *)pHandle;
	if(!pSearch) return;
	if(pSearch->stFile != INVALID_HANDLE_VALUE) FindClose(pSearch->stFile);
	delete pSearch;
}

void C_Resource::CreateDir(const char *szDirname)
{
	CreateDirectory(szDirname, NULL);
}

#else
//linux, mac
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h> //for mkdir
#include <sys/types.h> //for mkdir

struct S_FileSearch
{
	DIR *pDir;
	char szWild[32];
};

void *C_Resource::FileSearchOpen(const char *szPath, const char *szWild)
{
	if(!szPath || !szWild) return NULL;
	S_FileSearch *pSearch = new S_FileSearch();
	pSearch->pDir = opendir(szPath);
	strcpy(pSearch->szWild, szWild);
	return pSearch;
}

bool C_Resource::FileSearchNext(void *pHandle, char *o_szFilename, int iSize)
{
	bool bFound = false;
	S_FileSearch *pSearch = (S_FileSearch *)pHandle;
	if(!pSearch || !o_szFilename || !pSearch->pDir) return false;
	struct dirent *pDE = NULL;
	while(!bFound) {
		pDE = readdir(pSearch->pDir);
		if(!pDE) return false;
		if(fnmatch(pSearch->szWild, pDE->d_name, FNM_PATHNAME | FNM_PERIOD | FNM_NOESCAPE)==0) {
			strncpy(o_szFilename, pDE->d_name, iSize);
			o_szFilename[iSize-1] = 0;
			bFound = true;
		}
	}
	return true;
}

void C_Resource::FileSearchClose(void *pHandle)
{
	S_FileSearch *pSearch = (S_FileSearch *)pHandle;
	if(!pSearch) return;
	if(pSearch->pDir) closedir(pSearch->pDir);
	delete pSearch;
}

void C_Resource::CreateDir(const char *szDirname)
{
	mkdir(szDirname, 0755);
}

#endif
