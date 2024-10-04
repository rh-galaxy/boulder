// C_TargaImg

#ifndef _TARGA_H
#define _TARGA_H

#include "common.h"
#include "image.h"

class C_TargaImg
{
public:
	static C_Image *Load(const char *szFilename, const char *szResname);
	static C_Image *Load(void *pTargaData);

	static bool Save(const char *szFilename, C_Image *pSrc, bool bFlip = false);
	static bool SaveCompressed(const char *szFilename, C_Image *pSrc, bool bFlip = false);
private:
	static C_Image *LoadPrivate(const char *szFilename, const char *szResname, void *pTargaData);
};

#endif
