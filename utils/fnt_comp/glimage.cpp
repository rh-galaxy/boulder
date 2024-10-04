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


C_Image::C_Image(int i_iWidth, int i_iHeight, int i_iPixelfmt, uint8_t* i_pBuffer)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pPal = NULL;
	m_pBuf.u8ptr = i_pBuffer;
	m_bUsermem = true;
	if(i_iPixelfmt == _DEFAULT) i_iPixelfmt = s_iDefaultPixelfmt;
	m_iHeight = i_iHeight;
	m_iWidth = i_iWidth;
	m_iPixelfmt = i_iPixelfmt;
	m_iPixelSize = i_iPixelfmt & 0x0f;
	m_iColorkey = 0;

	m_iLineSize = i_iWidth * m_iPixelSize; //usermemory images are not aligned
	m_iBufSize = m_iLineSize * i_iHeight;

	m_pclConverter = new C_Converter(m_iPixelfmt);
}

C_Image::C_Image(int i_iWidth, int i_iHeight, int i_iPixelfmt, bool *o_bResult)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pclConverter = new C_Converter(i_iPixelfmt);

	m_pBuf.u8ptr = m_pPal = NULL;
	m_bUsermem = false;
	if(i_iPixelfmt == _DEFAULT) i_iPixelfmt = s_iDefaultPixelfmt;
	m_iHeight = i_iHeight;
	m_iWidth = i_iWidth;
	m_iPixelfmt = i_iPixelfmt;
	m_iPixelSize = i_iPixelfmt & 0x0f;
	m_iColorkey = 0;

	m_iLineSize = (i_iWidth * m_iPixelSize + (WALIGN-1)) & ~(WALIGN-1);
	m_iBufSize = m_iLineSize * i_iHeight;
	if(m_iBufSize) m_pBuf.u8ptr = new uint8_t[m_iBufSize];

	if(o_bResult) *o_bResult = true;
}

C_Image::C_Image(C_Image *i_pclSrcImg, bool *o_bResult)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pclConverter = new C_Converter(s_iDefaultPixelfmt);

	if(o_bResult) *o_bResult = false; //default fail
	m_pBuf.u8ptr = m_pPal = NULL;
	m_bUsermem = false;

	//source must exist
	if(i_pclSrcImg == NULL) return;

	i_pclSrcImg->GetInfo(&m_iWidth, &m_iHeight, &m_iPixelfmt);
	m_pclConverter->SetDstPixelFormat(m_iPixelfmt);
	m_iPixelSize = m_iPixelfmt & 0x0f;
	m_iColorkey = i_pclSrcImg->m_iColorkey;

	m_iLineSize = i_pclSrcImg->m_iLineSize;
	m_iBufSize = i_pclSrcImg->m_iBufSize;

	uint8_t *pSrcBuf, *pSrcPal;
	i_pclSrcImg->GetBufferMemory(&pSrcBuf, &pSrcPal);
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

C_Image::C_Image(const char *i_szFilename, const char *i_szResname, bool *o_bResult, int i_iPixelfmt)
{
#ifdef _IMAGE_USE_OPENGL
	m_hTexture = 0;
#endif

	m_pclConverter = new C_Converter(i_iPixelfmt);

	if(o_bResult) *o_bResult = false;
	m_pBuf.u8ptr = m_pPal = NULL;
	m_bUsermem = false;
	m_iColorkey = 0;

	C_TargaImg *pclTga = new C_TargaImg(); //only support tga in this special version for fnt_comp gl
	C_Image *pclDst = pclTga->Load(i_szFilename, i_szResname);
	delete pclTga;
	if(!pclDst) return;
	Swap(pclDst);
	if(!ConvertFormat(i_iPixelfmt)) return;

	if(o_bResult) *o_bResult = true;
}

C_Image::~C_Image()
{
#ifdef _IMAGE_USE_OPENGL
	if(m_hTexture) C_GraphWrapperGL::GetGraphWrapperObject()->FreeTextureAbs(m_hTexture);
#endif

	if(!m_bUsermem) delete[] m_pBuf.u8ptr;
	delete[] m_pPal;

	delete m_pclConverter;
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
void C_Image::GetBufferMemory(const uint8_t **o_pBuf, const uint8_t **o_pPal) const
{
	if(o_pBuf) *o_pBuf = m_pBuf.u8ptr;
	if(o_pPal) *o_pPal = m_pPal;
}

void C_Image::SetPal(uint8_t *i_pPal)
{
	if(!i_pPal) return;
	if(!m_pPal) m_pPal = new uint8_t[1024];
	memcpy(m_pPal, i_pPal, 1024);
}

void C_Image::FillWithAbsoluteColor(S_Rect *i_stArea, uint32_t i_iColor)
{
	if(!m_pBuf.u8ptr) return;

	//fill every pixel with that color
	if(i_iColor == 0 && !i_stArea) {
		memset(m_pBuf.u8ptr, 0, m_iBufSize);
	} else {
		AllPtrType b = m_pBuf;
		int x, y, x_count, y_count;
		uint16_t color2;
		uint8_t *tempsave;
		if(!i_stArea) {
			y_count = m_iHeight;
			x_count = m_iWidth;
		} else {
			b.u8ptr += i_stArea->y * m_iLineSize + (i_stArea->x*m_iPixelSize);
			y_count = i_stArea->height;
			x_count = i_stArea->width;
		}
		switch(m_iPixelSize) {
			case 1:
				for(y=0; y<y_count; y++) {
					for(x=0; x<x_count; x++) b.u8ptr[x] = (uint8_t)i_iColor;
					b.u8ptr += m_iLineSize;
				}
				break;
			case 2:
				for(y=0; y<y_count; y++) {
					for(x=0; x<x_count; x++) b.u16ptr[x] = (uint16_t)i_iColor;
					b.u8ptr += m_iLineSize;
				}
				break;
			case 3:
				color2 = (uint16_t)(i_iColor >> 8);
				for(y=0; y<y_count; y++) {
					tempsave = b.u8ptr;
					for(x=0; x<x_count; x++) {
						*b.u8ptr++ = (uint8_t)i_iColor;
						*b.u16ptr++ = color2;
					}
					b.u8ptr = tempsave + m_iLineSize;
				}
				break;
			case 4:
				for(y=0; y<y_count; y++) {
					for(x=0; x<x_count; x++) b.u32ptr[x] = i_iColor;
					b.u8ptr += m_iLineSize;
				}
				break;
		}
	}
}

void C_Image::FillWithRGBColor(S_Rect *i_stArea, S_Color *i_stColor)
{
	m_pclConverter->SetDstPixelFormat(m_iPixelfmt);
	uint32_t iOutcolor = m_pclConverter->GetAbsoluteColor(i_stColor);
	FillWithAbsoluteColor(i_stArea, iOutcolor);
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

	m_pclConverter->SetDstPixelFormat(m_iPixelfmt);
	m_pclConverter->GetColor(iIncolor, &stColor);

	return stColor;
}

void C_Image::SetTransparentColor(S_Color *i_stColor)
{
	m_pclConverter->SetDstPixelFormat(m_iPixelfmt);
	m_iColorkey = m_pclConverter->GetAbsoluteColor(i_stColor);
}

bool C_Image::ConvertFormat(int i_iPixelfmt)
{
	if(i_iPixelfmt == _DEFAULT) i_iPixelfmt = s_iDefaultPixelfmt;
	if(i_iPixelfmt == m_iPixelfmt) return true; //same format

	int result;
	C_Image *pclDst = NULL;
	m_pclConverter->SetDstPixelFormat(i_iPixelfmt);
	m_pclConverter->Convert(this, &pclDst, &result);

	if(!result) return false;
	Swap(pclDst);

	return true;
}

void C_Image::Swap(C_Image *i_pclSrc)
{
	if(i_pclSrc == this) return;

	uint8_t *temp;
	temp = i_pclSrc->m_pBuf.u8ptr; i_pclSrc->m_pBuf.u8ptr = m_pBuf.u8ptr; m_pBuf.u8ptr = temp;
	temp = i_pclSrc->m_pPal; i_pclSrc->m_pPal = m_pPal; m_pPal = temp;
	bool temp2;
	temp2 = i_pclSrc->m_bUsermem; i_pclSrc->m_bUsermem = m_bUsermem; m_bUsermem = temp2;
	m_iWidth = i_pclSrc->m_iWidth;
	m_iHeight = i_pclSrc->m_iHeight;
	m_iPixelfmt = i_pclSrc->m_iPixelfmt;
	m_iPixelSize = i_pclSrc->m_iPixelSize;
	m_iLineSize = i_pclSrc->m_iLineSize;
	m_iBufSize = i_pclSrc->m_iBufSize;

	delete i_pclSrc;
}

bool C_Image::Blt2(C_Image *i_pclSrcImg, S_Rect *i_pstSrcRect, int i_iX, int i_iY, uint32_t *i_piTranspCol)
{
	int iX = 0, iY = 0, iSrcW, iSrcH, iSrcBytes;
	int iDstBytes;

	if(!i_pclSrcImg) return false;

	i_pclSrcImg->GetInfo(&iSrcW, &iSrcH, &iSrcBytes);
	GetInfo(NULL, NULL, &iDstBytes);
	if(iSrcBytes!=iDstBytes) return false;

	iSrcBytes &= 0x07;
	iDstBytes &= 0x07;

	if(i_pstSrcRect) {
		iX = i_pstSrcRect->x;
		iY = i_pstSrcRect->y;
		iSrcW = i_pstSrcRect->width;
		iSrcH = i_pstSrcRect->height;
	}

	uint8_t *pSrc, *pDst;
	int srcline, dstline;
	i_pclSrcImg->GetBufferMemory(&pSrc, NULL);
	GetBufferMemory(&pDst, NULL);

	i_pclSrcImg->GetLineSizeInfo(NULL, &srcline, NULL);
	GetLineSizeInfo(NULL, &dstline, NULL);

	int x, y;
	int wb = iSrcBytes*iSrcW;
	pSrc += iY*srcline+iSrcBytes*iX;
	pDst += i_iY*dstline+iDstBytes*i_iX;

	bool bUseTransparent = (i_piTranspCol!=NULL) && iSrcBytes!=3; //not working on 24bpp
	if(bUseTransparent) {
		AllPtrType pS, pD;
		pS.u8ptr = pSrc; pD.u8ptr = pDst;
		for(y=0; y<iSrcH; y++) {
			if(iSrcBytes==4) {
				for(x=0; x<iSrcW; x++) {
					if(pS.u32ptr[x]!=*i_piTranspCol) pD.u32ptr[x] = pS.u32ptr[x];
				}
			} else if(iSrcBytes==2) {
				for(x=0; x<iSrcW; x++) {
					if(pS.u16ptr[x]!=*i_piTranspCol) pD.u16ptr[x] = pS.u16ptr[x];
				}
			} else if(iSrcBytes==1) {
				for(x=0; x<iSrcW; x++) {
					if(pS.u8ptr[x]!=*i_piTranspCol) pD.u8ptr[x] = pS.u8ptr[x];
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

	C_Image *pclNewImage = new C_Image(iW, iH, m_iPixelfmt);
	pclNewImage->FillWithAbsoluteColor(NULL, 0);

	pclNewImage->Blt2(this, NULL, 0,0);

	Swap(pclNewImage);

	m_iGLTextureW = m_iWidth;
	m_iGLTextureH = m_iHeight;
	m_iWidth  = iOldW;
	m_iHeight = iOldH;
}
#endif
