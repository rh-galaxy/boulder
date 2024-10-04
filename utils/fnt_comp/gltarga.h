// C_TargaImg

#ifndef _TARGA_H
#define _TARGA_H

#include "common.h"
#include "glimage.h"

class C_TargaImg
{
public:
	static C_Image *Load(const char *i_szFilename, const char *i_szResname);
	static C_Image *Load(void *i_pTargaData);

	static bool Save(const char *i_szFilename, C_Image *i_pclSrc, bool i_bFlip = false);
	static bool SaveCompressed(const char *i_szFilename, C_Image *i_pclSrc, bool i_bFlip = false);
private:
	static C_Image *LoadPrivate(const char *i_szFilename, const char *i_szResname, void *i_pTargaData);
};

#endif
