// C_FontGL

#ifndef _GLFONT_H
#define _GLFONT_H

#include <stdarg.h>
#include "glgraph.h"

class C_FontGL
{
public:
	C_FontGL();
	~C_FontGL();

	bool Load(char *szFontname, int iFontHeight = 24, int iThickness = 700/*(100-900 (400=normal, 700=bold)*/, bool bUnderLined = false, bool bItallic = false);
	bool GetHeights(int *o_iChar, int *o_iTotal, int *o_BelowBaseline);
	void Free();

	void SetColor(const S_FColor &stColor) {m_stColor = stColor;};
	int GetWidth(const char *szFmt, ...);
	void Print(int iX, int iY, const char *szFmt, ...);
	bool GetCharWidths(const char cChar, int *o_iPreX, int *o_iCharX, int *o_iPostX);
private:
	C_GraphWrapperGL *m_pGraph;

	//properties
	S_FColor m_stColor;

	//font
	GLuint m_hListBase;

	//dimensions
	int m_aiWidthA[256];
	int m_aiWidthB[256];
	int m_aiWidthC[256];
	int m_aiWidth[256];
	int m_iCharHeight;
	int m_iTotalHeight;
	int m_iBelowBaseline;
};

#endif
