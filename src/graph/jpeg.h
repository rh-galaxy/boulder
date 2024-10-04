// C_JpegImg

#ifndef _JPEG_H
#define _JPEG_H

#include "common.h"
#include "jpeg_include.h"

class C_JpegImg
{
public:
	C_JpegImg();
	~C_JpegImg();
	C_Image *Load(const char *szFilename, const char *szResname);
	C_Image *Load(void *pJpegData);

private:
	void      FreeMem();

	//file buffers
	AllPtrType fb; //pointer to current position in jpeg-file

	//decoder
	bool      ProcessData(); //gets info for jpeg decoder, and starts decoder
	void      DefineQT();
	void      DefineHT();
	bool      HandleFrame0();
	void      HandleSOS();
	void      DecodeMCUs();

	uint16_t  Swap(uint16_t iParam);
	void      MakeHuffCodes(S_HuffTable *pstHs);
	void      InvDCT(int16_t *piMcu, double *pdQt);

	//info from jpeg "header"
	double      qt[4][64];
	S_HuffTable ac[4];
	S_HuffTable dc[4];
	//int         m_iDataPrecision;
	int         m_iNumComponents; //1=Y 3=YCbCr (4=CMYK)
	C_Component *m_pComponent;
	int         m_iWidth, m_iHeight;

	//info for mcu decoder
	int       m_iWidthMCUs, m_iHeightMCUs;
	int       m_iDimMCUx, m_iDimMCUy;
	int       m_iMCUDcts;
	int       m_iMCUMember[10];
	int16_t   m_aiBlock[64];

	static C_ColorYUV *s_pYUV;
	static int s_iObjCount;

	//error recovery info
	int m_iRestartInterval;
};

#endif
