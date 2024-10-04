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

	bool Load(char *i_szFontname, int i_iFontHeight = 24, int i_iThickness = 700/*(100-900 (400=normal, 700=bold)*/, bool i_bUnderLined = false, bool i_bItallic = false);
	bool GetHeights(int *o_iChar, int *o_iTotal, int *o_BelowBaseline);
	void Free();

	void SetColor(const S_FColor &i_stColor) {m_stColor = i_stColor;};
	int GetWidth(const char *i_szFmt, ...);
	void Print(int i_iX, int i_iY, const char *i_szFmt, ...);
	bool GetCharWidths(const char i_cChar, int *o_iPreX, int *o_iCharX, int *o_iPostX);
private:
	C_GraphWrapperGL *m_pclGraph;

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
