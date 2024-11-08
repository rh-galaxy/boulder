// C_ImageDiskOP

#ifndef _IMAGEDISKOP_H
#define _IMAGEDISKOP_H

#include "image.h"

//file formats
#define _ERROR  0
#define _GIF87  3
#define _TARGA  4
#define _JPEG   5
#define _BMP    6

class C_ImageDiskOP
{
public:
	C_ImageDiskOP();
	~C_ImageDiskOP();

	//a simple way of creating the obj, load one image and deleting the obj.
	//the obj will delete itself after its done loading. dst is filled with a
	//new image of the same type as the image on file(you might need to convert it
	//before use) or NULL if fail
	C_ImageDiskOP(const char *szFilename, const char *szRsname, C_Image **o_pDstImg);

	//filename is the name(and path) of the file, or file within a resource.
	//resname should be name(and path) of a resource file or NULL.
	//dst is filled with a new image of the same type as the image on file
	//(you might need to convert it before use) or NULL if fail
	bool Load(const char *szFilename, const char *szRsname, C_Image **o_pDstImg);

	//saves pSrcImg to the file i_szFilename (with path).
	//the extension of i_szFilename can be: (.tga)
	bool Save(const char *szFilename, C_Image *pSrcImg);
private:
	//decides what file format, based on the extension
	int DetectFormat(const char *szFilename);
};

#endif
