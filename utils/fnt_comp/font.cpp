// C_Font

//bitmap font class (use fnt_comp to create a font).
//this is a step back from glfont which uses true type fonts, but it was necessary to be able to
// use opengl with the same result on different platforms (no OS specific methods used).
// as a bonus tabs can be set and used, as well as multiline strings.

#include "fileresource.h"
#include "font.h"

//set default data
C_Font::C_Font(int iPixelFmt)
{
	m_pImgFont = NULL; //no font loaded yet
	m_iSpaceY = 0;       //extra pixels between rows
	m_iSpaceX = 0;       //extra pixels between chars
	m_iFontHeight = 0;
	m_iNumChars = 0;
	m_bLoaded = false;

	m_iDstPixelFmt = iPixelFmt;
	m_pTabList     = NULL;

#ifdef _IMAGE_USE_OPENGL
	S_FColor stCol = S_FCOLORMAKE(1,1,1,1); //white
	SetColor(stCol);
#endif
}

//release font from memory if loaded
C_Font::~C_Font()
{
	if(m_pTabList) delete[] m_pTabList;
	if(m_pImgFont) delete m_pImgFont;
}

//loads a fontfile of the format fnt
bool C_Font::Load(const char *szFontNameBase, const char *szResname)
{
	if(m_pImgFont) {
		delete m_pImgFont;
		m_pImgFont = NULL;
	}

	//read files (info about font and chars, and font image)
	S_FontHeader stHeader;
	S_CharHeader stChar;
	memset(&stHeader, 0, sizeof(stHeader));
	char szFilenameTGA[MAX_PATH];
	char szFilenameFNT[MAX_PATH];
	sprintf(szFilenameTGA, "%s.tga", szFontNameBase);
	sprintf(szFilenameFNT, "%s.fnt", szFontNameBase);
	C_Resource *pResource = new C_Resource();
	pResource->SetFilename(szFilenameFNT, szResname);
	pResource->Read(&stHeader, sizeof(stHeader));
	if((stHeader.iMagicF != 'F') || (stHeader.iMagicN != 'N')) {
		delete pResource;
		return false;
	}
	m_iFontHeight = stHeader.iHeight;
	m_iNumChars   = stHeader.iChars;
	m_iSpaceY     = stHeader.iSpaceY;
	for(int i=0; i<m_iNumChars; i++) {
		pResource->Read(&stChar, sizeof(S_CharHeader));
		m_astChars[stChar.iIndex] = stChar;
	}
	delete pResource;

	m_pImgFont = new C_Image(szFilenameTGA, szResname, &m_bLoaded, m_iDstPixelFmt);
	if(!m_bLoaded) {
		delete m_pImgFont;
		m_pImgFont = NULL;
	}
	return m_bLoaded;
}

//set tab positions, 1st int is the total list length
void C_Font::SetTablist(int *pTabList)
{
	if(m_pTabList) {
		delete[] m_pTabList;
		m_pTabList = NULL;
	}
	if(!pTabList) return;
	int iSize = pTabList[0];

	m_pTabList = new int[iSize];
	memcpy(m_pTabList, pTabList, iSize*sizeof(int));
}

bool C_Font::GetCharYExt(int iIndex, int *o_iBeginY, int *o_iEndY)
{
	int x, y, iLineBytes, iBytesPerPixel;
	uint8_t *pBuf;
	S_Rect *pstRect = &m_astChars[iIndex].stRect;
	m_pImgFont->GetInfo(NULL, NULL, &iBytesPerPixel);
	m_pImgFont->GetLineSizeInfo(NULL, &iLineBytes, NULL);
	m_pImgFont->GetBufferMemory(&pBuf);
	iBytesPerPixel = iBytesPerPixel&0xf;
	bool bFound = false;

	//scan for start line
	if(o_iBeginY) {
		int iStartPos = pstRect->y*iLineBytes+pstRect->x*iBytesPerPixel;
		int iSize = pstRect->width*iBytesPerPixel;
		for(y=0; y<pstRect->height; y++) {
			for(x=iStartPos; x<iStartPos+iSize; x++) {
				if(pBuf[x]!=0) {
					*o_iBeginY = y;
					bFound = true;
					break;
				}
			}
			iStartPos += iLineBytes;
			if(bFound) break;
		}
	}

	//scan for end line
	if(o_iEndY) {
		bFound = false;
		int iStartPos = (pstRect->y+pstRect->height-1)*iLineBytes+pstRect->x*iBytesPerPixel;
		int iSize = pstRect->width*iBytesPerPixel;
		for(y=pstRect->height-1; y>=0; y--) {
			for(x=iStartPos; x<iStartPos+iSize; x++) {
				if(pBuf[x]!=0) {
					*o_iEndY = y;
					bFound = true;
					break;
				}
			}
			iStartPos -= iLineBytes;
			if(bFound) break;
		}
	}

	return bFound;
}

//get text dimensions
bool C_Font::GetTextSize(char *szText, int *o_iWidth, int *o_iHeight,
	int *o_iActiveY, int *o_iActiveHeight, int *o_iNumLines, S_LineInfo **o_pLineInfo)
{
	int iNumLines = 1, iLineLength, iTabNumber;
	int i, iW = 0, iH = 0;

	if(!m_bLoaded || !szText) return false;

	//scan text for numlines
	char *szTemp = szText;
	while(*szTemp != 0) {
		if(*szTemp == '\n') iNumLines++;
		szTemp++;
	}
	int iBeginY = -1, iEndY = -1;

	//get numcharsinline[] textlinewidth[] and maxtextwidth from text
	S_LineInfo *pstLine = new S_LineInfo[iNumLines];
	memset(pstLine, 0, iNumLines * sizeof(S_LineInfo));
	for(i=0; i<iNumLines; i++) {
		iLineLength = 0;
		iTabNumber = 0;
		while(*szText!='\n' && *szText!='\0') {
			if(*szText == '\t') {
				if(m_pTabList) pstLine[i].iWidth = m_pTabList[++iTabNumber];
			} else {
				int iIdx = *(uint8_t*)szText;
				if(o_iActiveY && o_iActiveHeight) {
					bool bFetchBegin = iBeginY==-1;
					if(GetCharYExt(iIdx, bFetchBegin ? &iBeginY : NULL, &iEndY)) {
						if(i>0) {
							iEndY += i*(m_iFontHeight + m_iSpaceY);
							if(bFetchBegin) {
								iBeginY += i*(m_iFontHeight + m_iSpaceY);
							}
						}
					}
				}
				if(pstLine[i].iWidth+m_astChars[iIdx].iPreX>=0) pstLine[i].iWidth += m_astChars[iIdx].iPreX;
				pstLine[i].iWidth += m_astChars[iIdx].iPostX+m_astChars[iIdx].stRect.width + m_iSpaceX;
			}
			if(pstLine[i].iWidth > iLineLength) iLineLength = pstLine[i].iWidth;
			pstLine[i].iNumChars++;
			szText++;
		}
		pstLine[i].iWidth = iLineLength;
		if(pstLine[i].iWidth != 0) pstLine[i].iWidth -= m_iSpaceX;
		if(pstLine[i].iWidth > iW) iW = pstLine[i].iWidth;
		szText++; //skip end of line (13) (or 0 on the last line)
	}
	iH = iNumLines * (m_iFontHeight + m_iSpaceY) - m_iSpaceY;
	if(m_iSpaceY<0) iH -= m_iSpaceY;

	if(o_iActiveY && o_iActiveHeight) {
		*o_iActiveY = *o_iActiveHeight = 0;
		if(iBeginY!=-1 && iEndY!=-1) {
			*o_iActiveY = iBeginY;
			*o_iActiveHeight = (iEndY-iBeginY)+1;
		}
	}

	if(o_iWidth)  *o_iWidth  = iW;
	if(o_iHeight) *o_iHeight = iH;
	if(o_iNumLines) *o_iNumLines = iNumLines;
	if(o_pLineInfo) *o_pLineInfo = pstLine;
	else delete[] pstLine;
	return true;
}

//prints a formatted text with the loaded font to the given image
// within given area, centered if wanted
int C_Font::Print(C_Image **io_pImg, S_Rect *pArea, uint32_t iFlags, const char *szFmt, ...)
{
	va_list   stPList;
	char      szText[1024];

	if(!m_bLoaded || !szFmt) return 0;

	va_start(stPList, szFmt);
	vsprintf(szText, szFmt, stPList);
	va_end(stPList);

	return PutString(io_pImg, pArea, iFlags, szText);
}

#ifdef _IMAGE_USE_OPENGL
//set font color (only used when printing to screen)
void C_Font::SetColor(const S_FColor &stColor)
{
	m_stColor = stColor;
}

//prints a formatted text with the loaded font to the screen
// within given area, centered if wanted
int C_Font::Print(S_Rect *pArea, uint32_t iFlags, const char *szFmt, ...)
{
	va_list   stPList;
	char      szText[1024];

	if(!m_bLoaded || !szFmt || !pArea) return 0;
	S_Rect stArea = *pArea;
	int x = stArea.x;
	int y = stArea.y;
	stArea.x = 0;
	stArea.y = 0;

	va_start(stPList, szFmt);
	vsprintf(szText, szFmt, stPList);
	va_end(stPList);

	C_Image *pImg = NULL;
	iFlags |= FONT_CREATESURFACE;
	int iW = PutString(&pImg, &stArea, iFlags, szText);

	if(pImg) {
		C_GraphWrapperGL *pGraph = C_GraphWrapperGL::GetGraphWrapperObject();
		pGraph->SetBltColor(&m_stColor);
		pGraph->Blt(pImg, NULL, x, y, true);
		pGraph->SetBltColor(NULL);
		delete pImg;
	}

	return iW;
}

//prints a formatted text with the loaded font to the screen
int C_Font::Print(int x, int y, const char *szFmt, ...)
{
	va_list   stPList;
	char      szText[1024];

	if(!m_bLoaded || !szFmt) return 0;

	va_start(stPList, szFmt);
	vsprintf(szText, szFmt, stPList);
	va_end(stPList);

	C_Image *pImg = NULL;
	int iFlags = FONT_CREATESURFACE;
	int iW = PutString(&pImg, NULL, iFlags, szText);

	if(pImg) {
		C_GraphWrapperGL *pGraph = C_GraphWrapperGL::GetGraphWrapperObject();
		pGraph->SetBltColor(&m_stColor);
		pGraph->Blt(pImg, NULL, x, y, true);
		pGraph->SetBltColor(NULL);
		delete pImg;
	}

	return iW;
}
#endif

//prints out a text with the loaded font to the given image
// within given area, centered if wanted
int C_Font::PutString(C_Image **io_pImg, S_Rect *pArea, uint32_t iFlags, char *szText)
{
	int         i, iNumLines, iTextW, iTextH;

	S_Rect      stTmpArea;
	int         iPixelFmt;

	S_LineInfo  *pstLine = NULL;

	//check if parameters are valid
	if(!m_bLoaded || !szText || !io_pImg) return 0;
	GetTextSize(szText, &iTextW, &iTextH, NULL, NULL, &iNumLines, &pstLine);

	iPixelFmt = m_iDstPixelFmt;
	if(!(iFlags & FONT_CREATESURFACE)) (*io_pImg)->GetInfo(&stTmpArea.width, &stTmpArea.height, &iPixelFmt);
	if(!pArea) {
		pArea = &stTmpArea;
		stTmpArea.x = stTmpArea.y = 0;
		if(iFlags & FONT_CREATESURFACE) {
			stTmpArea.width = iTextW;
			stTmpArea.height = iTextH;
		}
	}
	if(iPixelFmt != m_iDstPixelFmt || iTextH > pArea->height || iTextW > pArea->width)
		return 0;

	if(iFlags & FONT_CREATESURFACE) { //new surface
		*io_pImg = new C_Image(pArea->width, pArea->height, m_iDstPixelFmt);
		(*io_pImg)->FillWithAbsoluteColor(NULL, 0);
	}

	//put lines on image
	uint32_t iTransparentCol = 0x00000000;
	int iPos = 0;
	int iYStart = pArea->y, iXStart = pArea->x;
	if(iFlags & FONT_CENTERED) iYStart += (pArea->height - iTextH)/2;
	for(i=0; i<iNumLines; i++) {
		int iTabNumber = 0;
		if(iFlags & FONT_CENTERED) iXStart += (pArea->width - pstLine[i].iWidth)/2;

		//put line on image
		while(szText[iPos]!='\n' && szText[iPos]!='\0') {
			if (szText[iPos] == '\t') {
				iXStart = m_pTabList[++iTabNumber];
			} else {
				int iIndex = (unsigned char)szText[iPos];
				iXStart += m_astChars[iIndex].iPreX; //can be negative
				if (iXStart < pArea->x) iXStart = pArea->x; //fix if negative
				(*io_pImg)->Blt2(m_pImgFont, &m_astChars[iIndex].stRect, iXStart, iYStart, &iTransparentCol);
				iXStart += m_astChars[iIndex].stRect.width + m_astChars[iIndex].iPostX + m_iSpaceX;
			}
			iPos++;
		}
		iPos++;
		iXStart = pArea->x;
		iYStart += m_iFontHeight+m_iSpaceY;
	}
	delete[] pstLine;

	return iTextW;
}

