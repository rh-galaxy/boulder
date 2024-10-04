// C_Font

#ifndef FONT_H
#define FONT_H

#include "common.h"
#include "glimage.h"
#ifdef _IMAGE_USE_OPENGL
#include "glgraph.h"
#endif

//font flags
#define FONT_CENTERED      1
#define FONT_CREATESURFACE 2

#pragma pack(1)
struct S_FontHeader
{
	uint8_t iMagicN;    //'N'
	uint8_t iMagicF;    //'F'
	uint16_t iHeight;   //height of font (pixels)
	uint16_t iChars;    //num chars in font
	int16_t  iSpaceY;   //char position modifier
};

struct S_CharHeader
{
	uint16_t iIndex;    //ASCII value
	int16_t  iPreX;     //char position modifier
	int16_t  iPostX;    //char space after
	uint16_t iReserved; //0
	S_Rect   stRect;    //char x,y,w,h (pixels)
};
#pragma pack()

struct S_LineInfo
{
	int iNumChars;
	int iWidth;
};

class C_Font
{
public:
	C_Font(int iPixelFmt);
	~C_Font();
	//vital functions
	bool Load(const char *szFontNameBase, const char *szResname);
	int Print(C_Image **io_pImg, S_Rect *pArea, uint32_t iFlags, const char *szFmt, ...); //color is always white
#ifdef _IMAGE_USE_OPENGL
	int Print(S_Rect *pArea, uint32_t iFlags, const char *szFmt, ...);
	int Print(int x, int y, const char *szFmt, ...);
	void SetColor(const S_FColor &stColor);
#endif

	//set operations
	void SetTablist(int *pTabList);
	void SetSpaceX(int iX) {m_iSpaceX = iX;};
	void SetSpaceY(int iY) {m_iSpaceY = iY;};

	//info operations
	int GetRowHeight() {return m_iFontHeight + m_iSpaceY;};
	bool GetTextSize(char *szText, int *o_iWidth, int *o_iHeight = NULL, int *o_iActiveY = NULL, int *o_iActiveHeight = NULL,
		int *o_iNumLines = NULL, S_LineInfo **o_pLineInfo = NULL);
private:
	bool m_bLoaded;

	bool GetCharYExt(int iIndex, int *o_iBeginY, int *o_iEndY);
	int PutString(C_Image **io_pImg, S_Rect *pArea, uint32_t iFlags, char *szText);

	//static for each font
	int       m_iDstPixelFmt; //output pixel format
	C_Image   *m_pImgFont;
	int       m_iFontHeight;
	int       m_iNumChars;
	S_CharHeader m_astChars[256]; //max index is 255, so we only support ASCII

	//user changeable
	S_FColor  m_stColor; //color used when printing to screen
	int       m_iSpaceX; //space between chars (pixels)
	int       m_iSpaceY; //space between lines (pixels)
	int       *m_pTabList; //tablist, el 0 = size, el 1..size = tab positions
};

#endif
