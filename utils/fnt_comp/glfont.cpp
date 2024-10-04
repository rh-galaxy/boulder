// C_FontGL

//class to print .ttf OS specific fonts with opengl.
//the non WIN32 version of this class is only for linux
// and it is not well tested since it is not used in the game

#include "glfont.h"
#include <stdio.h>

#ifdef WIN32
#pragma comment (lib, "opengl32.lib")
#endif

#define CHARS_BASE  32
#define CHARS_COUNT (256-32)

C_FontGL::C_FontGL()
{
	m_hListBase = 0;
	m_pclGraph = C_GraphWrapperGL::GetGraphWrapperObject();
	m_stColor = S_FCOLORMAKE(1.0f, 1.0f, 1.0f, 1.0f); //white
}

C_FontGL::~C_FontGL()
{
	Free();
}

void C_FontGL::Free()
{
	if(m_hListBase) {
		glDeleteLists(m_hListBase, CHARS_COUNT);
	}
}

bool C_FontGL::Load(char *i_szFontname, int i_iFontHeight, int i_iThickness, bool i_bUnderLined, bool i_bItallic)
{
	int i;
	bool bResult = false;

	m_hListBase = glGenLists(CHARS_COUNT);

#ifdef WIN32
	HFONT hFont;     // Windows Font ID
	HFONT hOldFont;  // Used For Good House Keeping
	hFont = CreateFont(
		i_iFontHeight,  // Height Of Font
		0,              // Width Of Font
		0,              // Angle Of Escapement
		0,              // Orientation Angle
		i_iThickness,   // Font Weight
		i_bItallic,     // Italic
		i_bUnderLined,  // Underline
		false,          // Strikeout
		ANSI_CHARSET,   // Character Set Identifier
		OUT_TT_PRECIS,  // Output Precision
		CLIP_DEFAULT_PRECIS, // Clipping Precision
		ANTIALIASED_QUALITY, // Output Quality
		FF_DONTCARE|DEFAULT_PITCH, // Family And Pitch
		i_szFontname);
	if(hFont==NULL) goto error;

	HDC hDC = m_pclGraph->GetWindowDC();
	hOldFont = (HFONT)SelectObject(hDC, hFont);  // Selects The Font We Want

	TEXTMETRIC stTM;
	GetTextMetrics(hDC, &stTM);
	m_iCharHeight = stTM.tmHeight;
	m_iTotalHeight = /*stTM.tmInternalLeading +*/0+ stTM.tmHeight;
	m_iBelowBaseline = stTM.tmDescent;

	ABC astWidth[256];
	BOOL bWidthResult = GetCharABCWidths(hDC, 0, 255, astWidth);
	if(bWidthResult) {
		for(i=0; i<256; i++) {
			m_aiWidthA[i] = astWidth[i].abcA;
			m_aiWidthB[i] = astWidth[i].abcB;
			m_aiWidthC[i] = astWidth[i].abcC;
			m_aiWidth[i] = astWidth[i].abcA + astWidth[i].abcB + astWidth[i].abcC;
		}
	} else {
		bWidthResult = GetCharWidth(hDC, 0, 255, m_aiWidth);
		if(!bWidthResult) goto error;
		for(i=0; i<256; i++) {
			m_aiWidthA[i] = 0;
			m_aiWidthB[i] = m_aiWidth[i];
			m_aiWidthC[i] = 0;
		}
	}

	bResult = (wglUseFontBitmaps(hDC, CHARS_BASE, CHARS_COUNT, m_hListBase) != 0); // Builds Characters Starting At Character CHARS_BASE
	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);
#else
	Display *pDisplay;
	XFontStruct *pFontInfo;
	int iFirst, iLast;

	//need an X Display before calling any Xlib routines
	pDisplay = m_pclGraph->GetDisplay();
	if(!pDisplay) goto error;

	//load the font
	pFontInfo = XLoadQueryFont(pDisplay, i_szFontname);
	if(!pFontInfo) goto error;

	m_iBelowBaseline = 0; //?

	//tell GLX which font & glyphs to use
	iFirst = pFontInfo->min_char_or_byte2;
	iLast  = pFontInfo->max_char_or_byte2;
	glXUseXFont(pFontInfo->fid, iFirst, iLast-iFirst+1, 0+iFirst);
	char szString[2];
	szString[1] = 0;
	for(i=0; i<256; i++) {
		szString[0] = i;
		m_aiWidth[i] = XTextWidth(pFontInfo, szString, 1);
		m_aiWidthA[i] = 0;            //?
		m_aiWidthB[i] = m_aiWidth[i]; //?
		m_aiWidthC[i] = 0;            //?
	}
	m_iTotalHeight = (pFontInfo->max_bounds.ascent-pFontInfo->min_bounds.ascent) + pFontInfo->max_bounds.descent; //?
	m_iCharHeight = i_iFontHeight;                                                                                //?

	XFreeFont(pDisplay, pFontInfo);
	bResult = true;
#endif
error:
	return bResult;
}

bool C_FontGL::GetHeights(int *o_iChar, int *o_iTotal, int *o_BelowBaseline)
{
	if(o_iChar) *o_iChar = m_iCharHeight;
	if(o_iTotal) *o_iTotal = m_iTotalHeight;
	if(o_BelowBaseline) *o_BelowBaseline = m_iBelowBaseline;
	return (o_iChar || o_iTotal || m_iBelowBaseline);
}

int C_FontGL::GetWidth(const char *i_szFmt, ...)
{
	va_list   stPList;
	char      szText[512];

	if(i_szFmt == NULL) return 0;

	va_start(stPList, i_szFmt);
	vsprintf(szText, i_szFmt, stPList);
	va_end(stPList);

	int iWidth = 0;
	int i, iSize = (int)strlen(szText);

	for(i=0; i<iSize; i++) {
		iWidth += m_aiWidth[(unsigned char)szText[i]];
	}

	return iWidth;
}

bool C_FontGL::GetCharWidths(const char i_cChar, int *o_iPreX, int *o_iCharX, int *o_iPostX)
{
	if(o_iPreX)  *o_iPreX  = m_aiWidthA[(unsigned char)i_cChar];
	if(o_iCharX) *o_iCharX = m_aiWidthB[(unsigned char)i_cChar];
	if(o_iPostX) *o_iPostX = m_aiWidthC[(unsigned char)i_cChar];
	return true;
}

void C_FontGL::Print(int i_iX, int i_iY, const char *i_szFmt, ...)
{
	va_list   stPList;
	char      szText[512];
	int       iBase = CHARS_BASE;

	if(i_szFmt == NULL) return;

	va_start(stPList, i_szFmt);
	vsprintf(szText, i_szFmt, stPList);
	va_end(stPList);

	GLboolean bTexture = glIsEnabled(GL_TEXTURE_2D);
	if(bTexture) glDisable(GL_TEXTURE_2D);

	glColor4fv(&m_stColor.r);
	glRasterPos2f((float)i_iX, (float)(m_pclGraph->GetModeHeight()-i_iY));

	glPushAttrib(GL_LIST_BIT);

#ifndef WIN32
	iBase = 1;
#endif
	glListBase(m_hListBase - iBase);

	glCallLists((int)strlen(szText), GL_UNSIGNED_BYTE, szText);

	glPopAttrib();

	if(bTexture) glEnable(GL_TEXTURE_2D);
}
