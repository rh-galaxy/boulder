// C_Image

#ifndef _IMAGE_H
#define _IMAGE_H

#include "common.h"

//comment this if SDL is not used
#define _IMAGE_USE_SDL

//pixel formats
#define _DEFAULT  0x00
#define _PAL8     0x11
#define _BGR565   0x02
#define _RGB565   0x12
#define _BGR555   0x22
#define _RGB555   0x32
#define _BGR888   0x03
#define _RGB888   0x13
#define _BGRA8888 0x04
#define _RGBA8888 0x14

class C_Converter;
#ifdef _IMAGE_USE_SDL
#include <SDL.h>
class C_GraphWrapper;
#endif

#define WALIGN 8

class C_Image
{
public:
	//creates a new image, with user specified pixel-buffer memory.
	//this is always successful so no result needed. the destructor
	//does not try to free the mem. Memory may also be lost to the class
	//if the user for example does a ::ConvertFormat()
	C_Image(int iWidth, int iHeight, int iPixelfmt, uint8_t* pBuffer);
	//creates a new empty image with the dimensions specified. pixel-
	//format can be any one of the pixel format constants above. result
	//(if specified) will be filled with 1(success) or 0(fail)
	C_Image(int iWidth, int iHeight, int iPixelfmt, bool *o_bResult = NULL);
	//creates a new image with exactly the same contents (copied)
	//as the source. result (if specified) will be filled with 1(success)
	//or 0(fail)
	C_Image(C_Image *pSrcImg, bool *o_bResult = NULL);
	//can be used to load gif, tga, jpg (not raw since it has no header).
	//the format of the image will be converted to that specified in pixelfmt
	C_Image(const char *szFilename, const char *szResname, bool *o_bResult = NULL, int iPixelfmt = _DEFAULT);
	//clean up buffermem
	~C_Image();

	void GetInfo(int *o_iWidth, int *o_iHeight, int *o_iPixelfmt) const;
	void GetLineSizeInfo(int *o_iWidthBytes, int *o_iLineBytes, int *o_iPaddingBytes) const;
	void GetBufferMemory(uint8_t **o_pBuf, uint8_t **o_pPal = NULL) const;
	//returns the transparent color (in the same format as the image)
	uint32_t GetTransparentColor() {return m_iColorkey;};

	//copies the supplied pal1024 into local memory, and destroys any previous pal
	void SetPal(uint8_t *pPal);
	//sets the transparent color in image format independent color format
	void SetTransparentColor(S_Color *pColor);
	static void SetDefaultFormat(int iPixelfmt) {s_iDefaultPixelfmt = iPixelfmt;};
	static int GetDefaultFormat() {return s_iDefaultPixelfmt;};

	//use this to fill _PAL8 images or if you know the exact color.
	//nothing is done with the color, it is just put to every pixel
	void FillWithAbsoluteColor(S_Rect *pArea, uint32_t iColor);
	//use this fill method to fill an image with any RGB
	//pixel format, by using a universal color format (RGB888)
	void FillWithRGBColor(S_Rect *pArea, S_Color *pColor);
	//use this method to get the pixel RGB value at pos(x,y)
	S_Color GetPixelRGB(int x, int y);

	//use this to blt a rect of the image into another image, the other 
	//image must have the same pixel format and be able to fit the src rect
	bool Blt2(C_Image *pSrcImg, S_Rect *pSrcRect, int iX, int iY, uint32_t *piTranspCol = NULL);

	//converts the image to another pixel format. the old pixelbuffer is discarded.
	bool ConvertFormat(int iPixelfmt);

	bool       m_bUsermem; //user manage buffer mem?
private:
	static int s_iDefaultPixelfmt;
	int m_iWidth, m_iHeight; //dimensions
	int m_iPixelfmt;      //one of the pixel format constants

	int m_iPixelSize;     //bytes per pixel
	int m_iLineSize;      //(pixelsize * width) rounded up to be aligned by 8
	int m_iBufSize;       //size of entire image buffer (linesize * height)

	AllPtrType m_pBuf;    //image data
	uint8_t    *m_pPal;   //palette data (if any)

	uint32_t   m_iColorkey;

	void Swap(C_Image *pSrc);

	C_Converter *m_pConverter;

#ifdef _IMAGE_USE_SDL
	void ExpandToCompatibleDim();
	int m_iTextureW, m_iTextureH;
	SDL_Texture *m_hTexture;
	friend class C_GraphWrapper; //to make the above rows available only to C_GraphWrapper
#endif
};

#endif
