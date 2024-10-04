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
	C_Converter(int i_iDstPixelFmt);
	/* initialize output pixelformat */
	void Convert(C_Image *i_pclSrcImg, C_Image **o_ppclDstImg, int *o_iResult = NULL);
	/* if (src == *dst) then *dst will contain the converted src.
	src is deleted before exit and is therefore invalid.
	if (src != *dst) then *dst will contain the converted src
	src is not deleted. (*dst should be NULL).
	if this function fails *result will be set to 0 else 1 */
	uint32_t GetAbsoluteColor(S_Color *i_pstColor);
	/* converts the src-color from the universal RGB888 format
	to the format specified in dst_pixelfmt */
	void GetColor(uint32_t i_iColor, S_Color *o_pstColor);
	/* converts the src-color from the format specified in dst_pixelfmt (!)
	to the universal RGB888 format */

	void SetDstPixelFormat(int i_iDstPixelFmt);
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
