// C_Component

#ifndef _JPEG_INCLUDE_H
#define _JPEG_INCLUDE_H

#include "common.h"
#include "image.h"

//private to object
#define HUFF_LOOKAHEAD 16 //must be 16 right now
struct S_HuffTable
{
	int8_t  pcHT[256];    //all symbols in a huffman table
	uint8_t piBits[17];   //#codes with 1..16 bits
	uint8_t *piLookNBits; //#bits, or 0 if too long
	uint8_t *piLookSym;   //symbol, or unused
};

//color component in an image
class C_Component
{
public:
	C_Component();
	~C_Component();
	void AllocBuffer(int iWidthMCUs, int iHeightMCUs);
	void PutDCT(int16_t *piSrc);

	void SetNumDcts(int iNumDCTs, int iDctDimX, int iDctDimY) {
		m_iNumDCTs = iNumDCTs; m_iDimMCUx = iDctDimX; m_iDimMCUy = iDctDimY; };

	int     m_iLastDCVal;
	int     m_iACTab, m_iDCTab, m_iQTNum;

	uint8_t *m_piBuffer;

	int     m_iDimMCUx, m_iDimMCUy;
private:
	int     m_iNumMCUsX, m_iCurMCUX;
	int     m_iNumDCTs, m_iCurDCT;
	uint8_t *m_piBufPos;

	static uint8_t *s_piYUVRangeLimit;
	static int     s_iObjCount;
};

//handles bit input from stream
class C_JpegBitStream
{
public:
	C_JpegBitStream(uint8_t *piBitStream);
	uint32_t GetBits(int iNumBits); //works with maximum 24 bits
	void     IncPos(int iNumBits);  //works with many bits
	void     *GetStreamPosPtr() {return m_piBitPtr;};
private:
	uint8_t *m_piBitPtr;
	int     m_iCurBit;
};

//color conversion (YUV -> RGB)
class C_ColorYUV
{
public:
	C_ColorYUV();
	~C_ColorYUV();
	void Convert(C_Image *pImage, C_Component *pComponent, int dimx, int dimy);
private:
	static uint8_t *s_piRGBRangeLimit;
	static int     *s_CrRTab;
	static int     *s_CbBTab;
	static int     *s_CrGTab;
	static int     *s_CbGTab;
	static int     s_iObjCount;
};

#endif
