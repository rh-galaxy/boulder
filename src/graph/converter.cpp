// C_Converter

//class to convert images between different pixel formats.

#include "common.h"
#include "image.h"
#include "converter.h"

struct S_MaskShiftVal
{
	int r;
	int g;
	int b;
	int a;
};

struct S_MaskShift
{
	S_MaskShiftVal stShift;
	S_MaskShiftVal stMask;
	S_MaskShiftVal stNumPos;
};

S_MaskShift TBL_LOOKUP[4][4] = {
	{ //1 byte
	{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
	{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
	{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
	{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
	},{ //2 bytes
	{ {11,5,0,0}, {(31<<11),(63<<5),(31<<0), 0}, {5,6,5,0} }, //_BGR565
	{ {0,5,11,0}, {(31<<0),(63<<5),(31<<11), 0}, {5,6,5,0} }, //_RGB565
	{ {10,5,0,15}, {(31<<10),(31<<5),(31<<0), (1<<15)}, {5,5,5,1} }, //_BGR555
	{ {0,5,10,15}, {(31<<0),(31<<5),(31<<10), (1<<15)}, {5,5,5,1} }, //_RGB555
	},{ //3 bytes
	{ {16,8,0,0}, {(255<<16),(255<<8),(255<<0), 0}, {8,8,8,0} }, //_BGR888
	{ {0,8,16,0}, {(255<<0),(255<<8),(255<<16), 0}, {8,8,8,0} }, //_RGB888
	{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
	{ {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
	},{ //4 bytes
	{ {16,8,0,24}, {(255<<16),(255<<8),(255<<0), (255<<24)}, {8,8,8,8} }, //_BGRA8888
	{ {0,8,16,24}, {(255<<0),(255<<8),(255<<16), (255<<24)}, {8,8,8,8} }, //_RGBA8888
	{ {16,8,0,24}, {(255<<16),(255<<8),(255<<0), 0}, {8,8,8,8} }, //_BGRx8888
	{ {0,8,16,24}, {(255<<0),(255<<8),(255<<16), 0}, {8,8,8,8} }, //_RGBx8888
	}
};

//constructor, sets m_iDstPixelFmt format
C_Converter::C_Converter(int iDstPixelFmt)
{
	SetDstPixelFormat(iDstPixelFmt);
}

//set m_iDstPixelFmt format
void C_Converter::SetDstPixelFormat(int iDstPixelFmt)
{
	m_iDstPixelFmt = iDstPixelFmt;
	m_iDstBytes    = iDstPixelFmt & 0x0f;
}

//convert an image to m_iDstPixelFmt format
void C_Converter::Convert(C_Image *src, C_Image **dst, int *o_iResult)
{
	if(o_iResult) *o_iResult = 0;
	if(!src || !dst) return;
	bool bDeleteSrc = *dst == src;

	src->GetInfo(&m_iWidth, &m_iHeight, &m_iSrcPixelFmt);
	m_iSrcBytes = m_iSrcPixelFmt & 0x0f;

	if(m_iSrcPixelFmt == m_iDstPixelFmt) {
		*dst = src;
		if(!bDeleteSrc) *dst = new C_Image(src); //copy src to dst
		if(o_iResult) *o_iResult = 1;
		return; //same format; but maybe not original
	}

	//remove if 16->8, true->8 become supported
	if(m_iDstBytes == 1) return;
	//no palette...
	src->GetBufferMemory(&m_pSrcBuf.u8ptr, &m_pSrcPalBuf);
	if((m_iSrcBytes == 1) && !m_pSrcPalBuf) return;

	*dst = new C_Image(m_iWidth, m_iHeight, m_iDstPixelFmt);
	(*dst)->GetBufferMemory(&m_pDstBuf.u8ptr, NULL);
	src->GetLineSizeInfo(NULL, NULL, &m_iSrcAlign);
	(*dst)->GetLineSizeInfo(NULL, NULL, &m_iDstAlign);

	if(m_iDstBytes == 4) (*dst)->FillWithAbsoluteColor(NULL, 0);
	//the above clears the unused byte in 32 bit format.
	// makes sense when you use colorkey

	if(m_iSrcBytes == 1) _8torgb();
	else _rgbtorgb();

	if(bDeleteSrc) delete src;
	if(o_iResult) *o_iResult = 1;
}

//convert a color in _RGBA8888 format to m_iDstPixelFmt format
uint32_t C_Converter::GetAbsoluteColor(S_Color *pColor)
{
	uint32_t iColor = 0;

	if(m_iDstBytes == 1) return 0;
	if(m_iDstPixelFmt == _RGB888) { //same format?
		memcpy(&iColor, pColor, 3);
		return iColor;
	}
	if(m_iDstPixelFmt == _RGBA8888) { //same format?
		memcpy(&iColor, pColor, 4);
		return iColor;
	}
	m_iWidth = m_iHeight = 1;
	m_iSrcPixelFmt = _RGBA8888;
	m_pSrcBuf.u8ptr = (uint8_t *)pColor;
	m_iSrcBytes = m_iSrcPixelFmt & 0x0f;

	m_pDstBuf.u32ptr = &iColor;
	m_iSrcAlign = m_iDstAlign = 0;

	_rgbtorgb();
	return iColor;
}

//convert a color in m_iDstPixelFmt format to _RGBA8888 format
void C_Converter::GetColor(uint32_t iColor, S_Color *o_pColor)
{
	m_iSrcPixelFmt = m_iDstPixelFmt; //hack to get srcformat from constructor
	m_iSrcBytes = m_iSrcPixelFmt & 0x0f;

	SetDstPixelFormat(_RGBA8888);

	if(m_iSrcBytes == 1) return;

	o_pColor->a = 0;
	if(m_iSrcPixelFmt == _RGB888) { //same format?
		memcpy(o_pColor, &iColor, 3);
		return;
	}
	if(m_iSrcPixelFmt == _RGBA8888) { //same format?
		memcpy(o_pColor, &iColor, 4);
		return;
	}
	m_iWidth = m_iHeight = 1;
	m_iSrcAlign = m_iDstAlign = 0;
	m_pSrcBuf.u32ptr = &iColor;
	m_pDstBuf.u8ptr = (uint8_t *)o_pColor;

	_rgbtorgb();
}

//rgb24->bgr24,rgba32,bgra32,rgb16,bgr16,rgb15,bgr15
//rgba32->bgra32,rgb24,bgr24,rgb16,bgr16,rgb15,bgr15
//rgb16->rgb24,bgr24,rgba32,bgra32,bgr16,rgb15,bgr15
//rgb15->rgb24,bgr24,rgba32,bgra32,rgb16,bgr16,bgr15
//bgr24->rgb24,rgba32,bgra32,rgb16,bgr16,rgb15,bgr15
//bgra32->rgba32,rgb24,bgr24,rgb16,bgr16,rgb15,bgr15
//bgr16->rgb24,bgr24,rgba32,bgra32,rgb16,rgb15,bgr15
//bgr15->rgb24,bgr24,rgba32,bgra32,rgb16,bgr16,rgb15
void C_Converter::_rgbtorgb()
{
	S_MaskShiftVal *pstSrcShift = &TBL_LOOKUP[m_iSrcBytes-1][m_iSrcPixelFmt >> 4].stShift;
	S_MaskShiftVal *pstDstShift = &TBL_LOOKUP[m_iDstBytes-1][m_iDstPixelFmt >> 4].stShift;
	S_MaskShiftVal *pstSrcMask = &TBL_LOOKUP[m_iSrcBytes-1][m_iSrcPixelFmt >> 4].stMask;
	S_MaskShiftVal *pstDstMask = &TBL_LOOKUP[m_iDstBytes-1][m_iDstPixelFmt >> 4].stMask;
	S_MaskShiftVal *pstSrcNumPos = &TBL_LOOKUP[m_iSrcBytes-1][m_iSrcPixelFmt >> 4].stNumPos;
	S_MaskShiftVal *pstDstNumPos = &TBL_LOOKUP[m_iDstBytes-1][m_iDstPixelFmt >> 4].stNumPos;

	int iShiftRightR = (pstSrcShift->r - pstDstShift->r) - (pstDstNumPos->r - pstSrcNumPos->r);
	int iShiftRightG = (pstSrcShift->g - pstDstShift->g) - (pstDstNumPos->g - pstSrcNumPos->g);
	int iShiftRightB = (pstSrcShift->b - pstDstShift->b) - (pstDstNumPos->b - pstSrcNumPos->b);
	int iShiftRightA = (pstSrcShift->a - pstDstShift->a) - (pstDstNumPos->a - pstSrcNumPos->a);
	uint32_t iSrcCol, iDstCol, iDstColR, iDstColG, iDstColB, iDstColA;
	for(int y=0; y<m_iHeight; y++) {
		for(int x=0; x<m_iWidth; x++) {

			memcpy(&iSrcCol, m_pSrcBuf.u8ptr, m_iSrcBytes);
			
			iDstColR = iShiftRightR>=0
				? (iSrcCol >> iShiftRightR) & pstDstMask->r
				: (iSrcCol << -iShiftRightR) & pstDstMask->r;
      
			iDstColG = iShiftRightG>=0
				? (iSrcCol >> iShiftRightG) & pstDstMask->g
				: (iSrcCol << -iShiftRightG) & pstDstMask->g;

			iDstColB = iShiftRightB>=0
				? (iSrcCol >> iShiftRightB) & pstDstMask->b
				: (iSrcCol << -iShiftRightB) & pstDstMask->b;
			iDstCol = iDstColR | iDstColG | iDstColB;

			if (pstSrcMask->a == 0) { //no alpha source
				//if(iDstCol == 0) iDstColA = 0; //set transparancy //!
				//else iDstColA = 0xffffffff & pstDstMask->a; //!
				iDstColA = 0xffffffff & pstDstMask->a;
			} else {
				iDstColA = iShiftRightA>=0
					? (iSrcCol >> iShiftRightA) & pstDstMask->a
					: (iSrcCol << -iShiftRightA) & pstDstMask->a;
			}
			iDstCol |= iDstColA;

			memcpy(m_pDstBuf.u32ptr, &iDstCol, m_iDstBytes);
			//m_pDstBuf.u32ptr[0] = iDstCol;

			m_pSrcBuf.u8ptr += (int)m_iSrcBytes;
			m_pDstBuf.u8ptr += (int)m_iDstBytes;
		}
		m_pDstBuf.u8ptr += m_iDstAlign;
		m_pSrcBuf.u8ptr += m_iSrcAlign;
	}
}

//8pal->rgb24,bgr24,rgba32,bgra32,rgb16,bgr16,rgb15,bgr15
void C_Converter::_8torgb()
{
	S_MaskShiftVal *pstDstShift = &TBL_LOOKUP[m_iDstBytes-1][m_iDstPixelFmt >> 4].stShift;
	S_MaskShiftVal *pstDstMask = &TBL_LOOKUP[m_iDstBytes-1][m_iDstPixelFmt >> 4].stMask;
	S_MaskShiftVal *pstDstNumPos = &TBL_LOOKUP[m_iDstBytes-1][m_iDstPixelFmt >> 4].stNumPos;

	int iShiftRightR = -pstDstShift->r - (pstDstNumPos->r - 8);
	int iShiftRightG = -pstDstShift->g - (pstDstNumPos->g - 8);
	int iShiftRightB = -pstDstShift->b - (pstDstNumPos->b - 8);
	int iShiftRightA = -pstDstShift->a - (pstDstNumPos->a - 8);
	uint32_t iDstCol, iDstColR, iDstColG, iDstColB, iDstColA, iIdx;
	for(int y=0; y<m_iHeight; y++) {
		for(int x=0; x<m_iWidth; x++) {
			iIdx = *m_pSrcBuf.u8ptr++;
			iIdx <<= 2; //get to start in palette
			
			iDstColR = iShiftRightR>=0
				? (m_pSrcPalBuf[iIdx+0] >> iShiftRightR) & pstDstMask->r
				: (m_pSrcPalBuf[iIdx+0] << -iShiftRightR) & pstDstMask->r;

			iDstColG = iShiftRightG>=0
				? (m_pSrcPalBuf[iIdx+1] >> iShiftRightG) & pstDstMask->g
				: (m_pSrcPalBuf[iIdx+1] << -iShiftRightG) & pstDstMask->g;

			iDstColB = iShiftRightB>=0
				? (m_pSrcPalBuf[iIdx+2] >> iShiftRightB) & pstDstMask->b
				: (m_pSrcPalBuf[iIdx+2] << -iShiftRightB) & pstDstMask->b;
			iDstCol = iDstColR | iDstColG | iDstColB;

			//if(iDstCol == 0) iDstColA = 255; //set transparancy
			iDstColA = iShiftRightA>=0
				? (m_pSrcPalBuf[iIdx+3] >> iShiftRightA) & pstDstMask->a
				: (m_pSrcPalBuf[iIdx+3] << -iShiftRightA) & pstDstMask->a;
			iDstCol |= iDstColA;

			memcpy(m_pDstBuf.u32ptr, &iDstCol, m_iDstBytes);
			//m_pDstBuf.u32ptr[0] = iDstCol;

			m_pDstBuf.u8ptr += (int)m_iDstBytes;
		}
		m_pDstBuf.u8ptr += m_iDstAlign;
		m_pSrcBuf.u8ptr += m_iSrcAlign;
	}
}
