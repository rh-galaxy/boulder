// C_Image

#ifndef _GLIMAGE_H
#define _GLIMAGE_H

#include "common.h"

//comment this if opengl is not used
#define _IMAGE_USE_OPENGL

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
#ifdef _IMAGE_USE_OPENGL
class C_GraphWrapperGL;
#endif

#define WALIGN 8

class C_Image
{
public:
	/* Constructors, Destructor */
	C_Image(int i_iWidth, int i_iHeight, int i_iPixelfmt, uint8_t* i_pBuffer);
	/* creates a new image, with user specified pixelbuffer-memory.
	this is always successful so no result needed. the destructor
	does not try to free the mem. Memory may also be lost to the class
	if the user for example does a ::ConvertFormat() */
	C_Image(int i_iWidth, int i_iHeight, int i_iPixelfmt, bool *o_bResult = NULL);
	/* creates a new empty image with the dimensions specified. pixel-
	format can be any one of the pixelformat-constants above. result
	(if specified) will be filld with 1(success) or 0(fail)*/
	C_Image(C_Image *i_pclSrcImg, bool *o_bResult = NULL);
	/* creates a new image with exactly the same contents (copied)
	as the source. result (if specified) will be filld with 1(success)
	or 0(fail) */
	C_Image(const char *i_szFilename, const char *i_szResname, bool *o_bResult = NULL, int i_iPixelfmt = _DEFAULT);
	/* can be used to load gif, tga, jpg (not raw since it has no header).
	the format of the image will be converted to that specified in pixelfmt */
	~C_Image();
	/* clean up buffermem */

	/* Get-Functions */
	void GetInfo(int *o_iWidth, int *o_iHeight, int *o_iPixelfmt) const;
	void GetLineSizeInfo(int *o_iWidthBytes, int *o_iLineBytes, int *o_iPaddingBytes) const;
	void GetBufferMemory(uint8_t **o_pBuf, uint8_t **o_pPal = NULL) const;
	void GetBufferMemory(const uint8_t **o_pBuf, const uint8_t **o_pPal = NULL) const;
	uint32_t GetTransparentColor() {return m_iColorkey;};
	/* returns the transparent color (in the same format as the image) */
	/* Set-Functions */
	void SetPal(uint8_t *i_pPal);
	/* copies the supplied pal1024 into local memory,
	and destroys any previous pal */
	void SetTransparentColor(S_Color *i_stColor);
	/* sets the transparent color in image format independent colorformat */
	static void SetDefaultFormat(int i_iPixelfmt) {s_iDefaultPixelfmt = i_iPixelfmt;};
	static int GetDefaultFormat() {return s_iDefaultPixelfmt;};

	/* other functions */
	void FillWithAbsoluteColor(S_Rect *i_stArea, uint32_t i_iColor);
	/* use this to fill _PAL8 images or if you know the exact color.
	nothing is done with the color, it is just put to every pixel */
	void FillWithRGBColor(S_Rect *i_stArea, S_Color *i_stColor);
	/* use this fillmethod to fill an image with any RGB
	pixelformat, by using a universal colorformat (RGB888) */
	S_Color GetPixelRGB(int x, int y);
	/* use this method to get the pixel RGB value at pos(x,y) */

	bool Blt2(C_Image *i_pclSrcImg, S_Rect *i_pstSrcRect, int i_iX, int i_iY, uint32_t *i_piTranspCol = NULL);
	/* use this to blt a rect of the image into another image, the other 
	image must have the same pixelformat and be able to fit the src rect */

	bool ConvertFormat(int i_iPixelfmt);
	/* converts the image to another pixelformat. the old pixelbuffer
	is discarded. */

	bool       m_bUsermem; //user manage buffer mem?
private:
	static int s_iDefaultPixelfmt;
	int m_iWidth, m_iHeight; //dimensions
	int m_iPixelfmt;      //one of the pixelformat-constants

	int m_iPixelSize;     //bytes per pixel
	int m_iLineSize;      //(pixelsize * width) rounded up to be aligned by 8
	int m_iBufSize;       //size of entire image buffer (linesize * height)

	AllPtrType m_pBuf;    //image data
	uint8_t    *m_pPal;   //palette data (if any)

	uint32_t   m_iColorkey;

	void Swap(C_Image *i_pclSrc);

	C_Converter *m_pclConverter;

#ifdef _IMAGE_USE_OPENGL
	void ExpandToOpenGLCompatibleDim();
	int m_iGLTextureW, m_iGLTextureH;
	unsigned int m_hTexture;
	friend class C_GraphWrapperGL; //to make the above rows available only to C_GraphWrapperGL
#endif
};

#endif
