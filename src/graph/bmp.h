// C_BmpImg

#ifndef _BMP_H
#define _BMP_H

#include "common.h"
#include "image.h"

class C_BmpImg
{
public:
	static C_Image *Load(const char *szFilename, const char *szResname);
};

#endif
