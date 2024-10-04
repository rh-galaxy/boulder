// C_Resource

#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "common.h"

class C_Resource
{
public:
	C_Resource();
	~C_Resource();

	void SetMode(bool bReading); //default reading (resets filenames)
	bool SetFilename(const char* szFilename, const char* szResname = NULL); //resname valid when reading (mode should be set first)

	bool   Read(void *pData, uint32_t iLength); //length==0xffffffff means entire resource
	void   Seek(uint32_t iPos, int iMode);
	uint32_t GetSize() {return m_iFileSize;};

	bool Write(void *pData, uint32_t iLength);

	//functions to find files in a directory
	static void *FileSearchOpen(const char *szPath, const char *szWild);
	static bool FileSearchNext(void *pHandle, char *o_szFilename, int iStringSize);
	static void FileSearchClose(void *pHandle);

	static void CreateDir(const char *szDirname);

private:
	void Reset();
	bool     m_bReading;
	uint32_t m_iFileSize;
	uint32_t m_iFileStart;
	FILE     *m_pFileHandle;
};

#endif
