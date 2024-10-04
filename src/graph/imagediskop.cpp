// C_ImageDiskOP

//loads and saves images of different formats depending on extension
// .tga load/save (save is done as rle compressed)
// .bmp load only
// .jpg load only

#include "imagediskop.h"

#include "targa.h"
#include "jpeg.h"
#include "bmp.h"

struct S_FormatExt
{
	char const *szExt;
	int        iFormat;
};

#define NUMEXTENSIONS 7
S_FormatExt TBL_FORMATEXTENSION[NUMEXTENSIONS] = {
	{"tga", _TARGA}, {"targa", _TARGA}, {"jpg", _JPEG},
	{"jpeg", _JPEG}, {"jif", _JPEG}, {"jfif", _JPEG}, {"bmp", _BMP}
};

C_ImageDiskOP::C_ImageDiskOP()
{
}

C_ImageDiskOP::~C_ImageDiskOP()
{
}

C_ImageDiskOP::C_ImageDiskOP(const char *szFilename, const char *szRsname, C_Image **o_pDstImg)
{
	if(!o_pDstImg) return;
	Load(szFilename, szRsname, o_pDstImg);
	delete this;
}

bool C_ImageDiskOP::Load(const char *szFilename, const char *szRsname, C_Image **o_pDstImg)
{
	bool bResult = false;
	C_JpegImg  *pJpg;

	if(!szFilename) return false;

	int iFormat = DetectFormat(szFilename); //normal resource
	switch(iFormat) {
		case _TARGA:
			*o_pDstImg = C_TargaImg::Load(szFilename, szRsname);
			if(*o_pDstImg) bResult = 1;
			break;
		case _JPEG:
			pJpg = new C_JpegImg();
			*o_pDstImg = pJpg->Load(szFilename, szRsname);
			if(*o_pDstImg) bResult = 1;
			delete pJpg;
			break;
		case _BMP:
			*o_pDstImg = C_BmpImg::Load(szFilename, szRsname);
			if(*o_pDstImg) bResult = 1;
			break;
	}
	return bResult;
}

bool C_ImageDiskOP::Save(const char *szFilename, C_Image *pSrcImg)
{
	bool bResult = false;

	if(!szFilename) return false;

	int iFormat = DetectFormat(szFilename);
	switch(iFormat) {
		case _TARGA:
			bResult = C_TargaImg::SaveCompressed(szFilename, pSrcImg);
			break;
		//case _JPEG:  //will not happen
	}
	return bResult;
}

int C_ImageDiskOP::DetectFormat(const char *szFilename)
{
	int i = (int)strlen(szFilename) - 1;
	while(i>=0 && szFilename[i]!='.') i--;    //find last .
	if(i<0) return _ERROR;

	char szExt[16];
	strncpy(szExt, &szFilename[i+1], sizeof(szExt));
	szExt[sizeof(szExt)-1] = 0;

	int iFormat = _ERROR;
	for(i=0; i<NUMEXTENSIONS; i++) {
		if(_stricmp(TBL_FORMATEXTENSION[i].szExt, szExt) == 0) {
			iFormat = TBL_FORMATEXTENSION[i].iFormat;
			break;
		}
	}
	return iFormat;
}
