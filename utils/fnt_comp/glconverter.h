// C_Converter

#ifndef _CONVERTER_H
#define _CONVERTER_H

#include "common.h"
#include "glimage.h"

#define _BGR     0x00
#define _RGB     0x01

class C_Converter
{
public:
	//initialize output pixel format
	C_Converter(int iDstPixelFmt);

	//if (src == *dst) then *dst will contain the converted src.
	//src is deleted before exit and is therefore invalid.
	//if (src != *dst) then *dst will contain the converted src
	//src is not deleted. (*dst should be NULL).
	//if this function fails *result will be set to 0 else 1
	void Convert(C_Image* pSrcImg, C_Image** o_pDstImg, int* o_iResult = NULL);
	//converts the src-color from the universal RGB888 format
	//to the format specified in dst_pixelfmt
	uint32_t GetAbsoluteColor(S_Color* pColor);
	//converts the src-color from the format specified in dst_pixelfmt (!)
	//to the universal RGB888 format
	void GetColor(uint32_t iColor, S_Color* o_pColor);

	void SetDstPixelFormat(int iDstPixelFmt);
private:
	int    m_iDstBytes;
	int    m_iSrcBytes;
	int    m_iDstAlign;
	int    m_iSrcAlign;

	AllPtrType m_pSrcBuf;
	AllPtrType m_pDstBuf;
	uint8_t    *m_pSrcPalBuf;

	int   m_iDstPixelFmt;
	int   m_iSrcPixelFmt;
	int   m_iWidth, m_iHeight;

	//conversion
	void  _rgbtorgb();
	void  _8torgb();
};

#endif
