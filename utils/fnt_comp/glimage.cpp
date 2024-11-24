// C_Image

//class to hold an image with a certain pixel format
// can be used with the _IMAGE_USE_OPENGL define to hold extra opengl info.

#include "glimage.h"
#include "gltarga.h"
#include "glconverter.h"

#ifdef _IMAGE_USE_OPENGL
#include "glgraph.h"
#endif

int C_Image::s_iDefaultPixelfmt = _BGRA8888;


C_Image::C_Image(int iWidth, int iHeight, int iPixelfmt, uint8_t* pBuffer)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pPal = NULL;
	m_pBuf.u8ptr = pBuffer;
	m_bUsermem = true;
	if(iPixelfmt == _DEFAULT) iPixelfmt = s_iDefaultPixelfmt;
	m_iHeight = iHeight;
	m_iWidth = iWidth;
	m_iPixelfmt = iPixelfmt;
	m_iPixelSize = iPixelfmt & 0x0f;
	m_iColorkey = 0;

	m_iLineSize = iWidth * m_iPixelSize; //usermemory images are not aligned
	m_iBufSize = m_iLineSize * iHeight;

	m_pConverter = new C_Converter(m_iPixelfmt);
}

C_Image::C_Image(int iWidth, int iHeight, int iPixelfmt, bool* o_bResult)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pConverter = new C_Converter(iPixelfmt);

	m_pBuf.u8ptr = m_pPal = NULL;
	m_bUsermem = false;
	if(iPixelfmt == _DEFAULT) iPixelfmt = s_iDefaultPixelfmt;
	m_iHeight = iHeight;
	m_iWidth = iWidth;
	m_iPixelfmt = iPixelfmt;
	m_iPixelSize = iPixelfmt & 0x0f;
	m_iColorkey = 0;

	m_iLineSize = (iWidth * m_iPixelSize + (WALIGN-1)) & ~(WALIGN-1);
	m_iBufSize = m_iLineSize * iHeight;
	if(m_iBufSize) m_pBuf.u8ptr = new uint8_t[m_iBufSize];

	if(o_bResult) *o_bResult = true;
}

C_Image::C_Image(C_Image *pSrcImg, bool *o_bResult)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pConverter = new C_Converter(s_iDefaultPixelfmt);

	if(o_bResult) *o_bResult = false; //default fail
	m_pBuf.u8ptr = m_pPal = NULL;
	m_bUsermem = false;

	//source must exist
	if(pSrcImg == NULL) return;

	pSrcImg->GetInfo(&m_iWidth, &m_iHeight, &m_iPixelfmt);
	m_pConverter->SetDstPixelFormat(m_iPixelfmt);
	m_iPixelSize = m_iPixelfmt & 0x0f;
	m_iColorkey = pSrcImg->m_iColorkey;

	m_iLineSize = pSrcImg->m_iLineSize;
	m_iBufSize = pSrcImg->m_iBufSize;

	uint8_t *pSrcBuf, *pSrcPal;
	pSrcImg->GetBufferMemory(&pSrcBuf, &pSrcPal);
	if(pSrcBuf) {
		m_pBuf.u8ptr = new uint8_t[m_iBufSize];
		memcpy(m_pBuf.u8ptr, pSrcBuf, m_iBufSize);
	}
	if(pSrcPal) {
		m_pPal = new uint8_t[1024];
		memcpy(m_pPal, pSrcPal, 1024);
	}
	if(o_bResult) *o_bResult = true;
}

C_Image::C_Image(const char *szFilename, const char *szResname, bool *o_bResult, int iPixelfmt)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pConverter = new C_Converter(iPixelfmt);

	if(o_bResult) *o_bResult = false;
	m_pBuf.u8ptr = m_pPal = NULL;
	m_bUsermem = false;
	m_iColorkey = 0;

	C_TargaImg *pTga = new C_TargaImg(); //only support tga in this special version for fnt_comp gl
	C_Image *pDst = pTga->Load(szFilename, szResname);
	delete pTga;
	if(!pDst) return;
	Swap(pDst);
	if(!ConvertFormat(iPixelfmt)) return;

	if(o_bResult) *o_bResult = true;
}

C_Image::~C_Image()
{
#ifdef _IMAGE_USE_OPENGL
	if(m_hTexture) C_GraphWrapperGL::GetGraphWrapperObject()->FreeTextureAbs(m_hTexture);
#endif

	if(!m_bUsermem) delete[] m_pBuf.u8ptr;
	delete[] m_pPal;

	delete m_pConverter;
}

void C_Image::GetInfo(int *o_iWidth, int *o_iHeight, int *o_iPixelfmt) const
{
	if(o_iWidth) *o_iWidth = m_iWidth;
	if(o_iHeight) *o_iHeight = m_iHeight;
	if(o_iPixelfmt) *o_iPixelfmt = m_iPixelfmt;
}

void C_Image::GetLineSizeInfo(int *o_iWidthBytes, int *o_iLineBytes, int *o_iPaddingBytes) const
{
	int wb = m_iPixelSize * m_iWidth;
	if(o_iWidthBytes) *o_iWidthBytes = wb;
	if(o_iLineBytes) *o_iLineBytes = m_iLineSize;
	if(o_iPaddingBytes) *o_iPaddingBytes = m_iLineSize - wb;
}

void C_Image::GetBufferMemory(uint8_t **o_pBuf, uint8_t **o_pPal) const
{
	if(o_pBuf) *o_pBuf = m_pBuf.u8ptr;
	if(o_pPal) *o_pPal = m_pPal;
}

void C_Image::SetPal(uint8_t *pPal)
{
	if(!pPal) return;
	if(!m_pPal) m_pPal = new uint8_t[1024];
	memcpy(m_pPal, pPal, 1024);
}

void C_Image::FillWithAbsoluteColor(S_Rect *stArea, uint32_t iColor)
{
	if(!m_pBuf.u8ptr) return;

	//fill every pixel with that color
	if(iColor == 0 && !stArea) {
		memset(m_pBuf.u8ptr, 0, m_iBufSize);
	} else {
		AllPtrType b = m_pBuf;
		int x, y, x_count, y_count;
		uint16_t color2;
		uint8_t *tempsave;
		if(!stArea) {
			y_count = m_iHeight;
			x_count = m_iWidth;
		} else {
			b.u8ptr += stArea->y * m_iLineSize + (stArea->x*m_iPixelSize);
			y_count = stArea->height;
			x_count = stArea->width;
		}
		switch(m_iPixelSize) {
			case 1:
				for(y=0; y<y_count; y++) {
					for(x=0; x<x_count; x++) b.u8ptr[x] = (uint8_t)iColor;
					b.u8ptr += m_iLineSize;
				}
				break;
			case 2:
				for(y=0; y<y_count; y++) {
					for(x=0; x<x_count; x++) b.u16ptr[x] = (uint16_t)iColor;
					b.u8ptr += m_iLineSize;
				}
				break;
			case 3:
				color2 = (uint16_t)(iColor >> 8);
				for(y=0; y<y_count; y++) {
					tempsave = b.u8ptr;
					for(x=0; x<x_count; x++) {
						*b.u8ptr++ = (uint8_t)iColor;
						*b.u16ptr++ = color2;
					}
					b.u8ptr = tempsave + m_iLineSize;
				}
				break;
			case 4:
				for(y=0; y<y_count; y++) {
					for(x=0; x<x_count; x++) b.u32ptr[x] = iColor;
					b.u8ptr += m_iLineSize;
				}
				break;
		}
	}
}

void C_Image::FillWithRGBColor(S_Rect *stArea, S_Color *stColor)
{
	m_pConverter->SetDstPixelFormat(m_iPixelfmt);
	uint32_t iOutcolor = m_pConverter->GetAbsoluteColor(stColor);
	FillWithAbsoluteColor(stArea, iOutcolor);
}

S_Color C_Image::GetPixelRGB(int x, int y)
{
	S_Color stColor;
	AllPtrType b = m_pBuf;
	b.u8ptr += m_iLineSize*y;
	uint32_t iIncolor = 0;
	switch(m_iPixelSize) {
		case 1: iIncolor = b.u8ptr[x];  break;
		case 2: iIncolor = b.u16ptr[x]; break;
		case 3: b.u8ptr += x*3; iIncolor = (*b.u32ptr) & 0x00ffffff; break;
		case 4: iIncolor = b.u32ptr[x]; break;
	}

	m_pConverter->SetDstPixelFormat(m_iPixelfmt);
	m_pConverter->GetColor(iIncolor, &stColor);

	return stColor;
}

void C_Image::SetTransparentColor(S_Color *stColor)
{
	m_pConverter->SetDstPixelFormat(m_iPixelfmt);
	m_iColorkey = m_pConverter->GetAbsoluteColor(stColor);
}

bool C_Image::ConvertFormat(int iPixelfmt)
{
	if(iPixelfmt == _DEFAULT) iPixelfmt = s_iDefaultPixelfmt;
	if(iPixelfmt == m_iPixelfmt) return true; //same format

	int result;
	C_Image *pDst = NULL;
	m_pConverter->SetDstPixelFormat(iPixelfmt);
	m_pConverter->Convert(this, &pDst, &result);

	if(!result) return false;
	Swap(pDst);

	return true;
}

void C_Image::Swap(C_Image *pSrc)
{
	if(pSrc == this) return;

	uint8_t *temp;
	temp = pSrc->m_pBuf.u8ptr; pSrc->m_pBuf.u8ptr = m_pBuf.u8ptr; m_pBuf.u8ptr = temp;
	temp = pSrc->m_pPal; pSrc->m_pPal = m_pPal; m_pPal = temp;
	bool temp2;
	temp2 = pSrc->m_bUsermem; pSrc->m_bUsermem = m_bUsermem; m_bUsermem = temp2;
	m_iWidth = pSrc->m_iWidth;
	m_iHeight = pSrc->m_iHeight;
	m_iPixelfmt = pSrc->m_iPixelfmt;
	m_iPixelSize = pSrc->m_iPixelSize;
	m_iLineSize = pSrc->m_iLineSize;
	m_iBufSize = pSrc->m_iBufSize;

	delete pSrc;
}

bool C_Image::Blt2(C_Image *pSrcImg, S_Rect *pstSrcRect, int iX, int iY, uint32_t *piTranspCol)
{
	int iX2 = 0, iY2 = 0, iSrcW, iSrcH, iSrcBytes;
	int iDstBytes;

	if(!pSrcImg) return false;

	pSrcImg->GetInfo(&iSrcW, &iSrcH, &iSrcBytes);
	GetInfo(NULL, NULL, &iDstBytes);
	if(iSrcBytes!=iDstBytes) return false;

	iSrcBytes &= 0x07;
	iDstBytes &= 0x07;

	if(pstSrcRect) {
		iX2 = pstSrcRect->x;
		iY2 = pstSrcRect->y;
		iSrcW = pstSrcRect->width;
		iSrcH = pstSrcRect->height;
	}

	uint8_t *pSrc, *pDst;
	int srcline, dstline;
	pSrcImg->GetBufferMemory(&pSrc, NULL);
	GetBufferMemory(&pDst, NULL);

	pSrcImg->GetLineSizeInfo(NULL, &srcline, NULL);
	GetLineSizeInfo(NULL, &dstline, NULL);

	int x, y;
	int wb = iSrcBytes*iSrcW;
	pSrc += iY2*srcline+iSrcBytes*iX2;
	pDst += iY*dstline+iDstBytes*iX;

	bool bUseTransparent = (piTranspCol!=NULL) && iSrcBytes!=3; //not working on 24bpp
	if(bUseTransparent) {
		AllPtrType pS, pD;
		pS.u8ptr = pSrc; pD.u8ptr = pDst;
		for(y=0; y<iSrcH; y++) {
			if(iSrcBytes==4) {
				for(x=0; x<iSrcW; x++) {
					if(pS.u32ptr[x]!=*piTranspCol) pD.u32ptr[x] = pS.u32ptr[x];
				}
			} else if(iSrcBytes==2) {
				for(x=0; x<iSrcW; x++) {
					if(pS.u16ptr[x]!=*piTranspCol) pD.u16ptr[x] = pS.u16ptr[x];
				}
			} else if(iSrcBytes==1) {
				for(x=0; x<iSrcW; x++) {
					if(pS.u8ptr[x]!=*piTranspCol) pD.u8ptr[x] = pS.u8ptr[x];
				}
			}

			pS.u8ptr += srcline;
			pD.u8ptr += dstline;
		}
	} else {
		for(y=0; y<iSrcH; y++) {
			memcpy(pDst, pSrc, wb);
			pSrc += srcline;
			pDst += dstline;
		}
	}

	return true;
}

#ifdef _IMAGE_USE_OPENGL
void C_Image::ExpandToOpenGLCompatibleDim()
{
	#define POW_MAX 11
	int i, aiPow[POW_MAX] = {2,4,8,16,32,64,128,256,512,1024,2048};
	int iOldW = m_iWidth;
	int iOldH = m_iHeight;
	int iW = m_iWidth;
	int iH = m_iHeight;

	for(i=0; i<POW_MAX; i++) {
		if(aiPow[i]>=m_iWidth) {
			iW = aiPow[i];
			break;
		}
	}
	for(i=0; i<POW_MAX; i++) {
		if(aiPow[i]>=m_iHeight) {
			iH = aiPow[i];
			break;
		}
	}

	C_Image *pNewImage = new C_Image(iW, iH, m_iPixelfmt);
	pNewImage->FillWithAbsoluteColor(NULL, 0);

	pNewImage->Blt2(this, NULL, 0,0);

	Swap(pNewImage);

	m_iGLTextureW = m_iWidth;
	m_iGLTextureH = m_iHeight;
	m_iWidth  = iOldW;
	m_iHeight = iOldH;
}
#endif
