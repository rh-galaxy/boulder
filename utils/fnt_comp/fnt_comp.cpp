
//program to create a bitmap font file (used with the class C_Font) from an OS ttf font.
// useful since ttf files are not the same on all OSes, so to be able to have a font show
// exactly the same on all OS we need to use a bitmap font.
//
//the font and size used is selected by modifying the FONTNAME and FONTSIZE defines and
// recompile this utility.

#include "common.h"
#include "glgraph.h"
#include "glfont.h"
#include "font.h"

#define FONTNAME "arial"
#define FONTSIZE -37

bool BuildFont(char *szFontName, int iFontHeightIn, int iThickness, char *szOutputFontNameBase)
{
	int i, x, y;
	bool bResult = false;
	C_GraphWrapperGL *pGraph = C_GraphWrapperGL::GetGraphWrapperObject();
	S_Color stTransCol = {0,0,0,0};
	S_FontHeader stHeader;
	S_CharHeader stChar;
	C_FontGL *pFont = NULL;
	FILE *pFile = NULL;
	stChar.iReserved = 0; //set once
	char szFilenameTGA[MAX_PATH];
	char szFilenameFNT[MAX_PATH];
	char szFontNameString[256];
#ifdef WIN32
	strcpy(szFontNameString, szFontName);
#else
	//sprintf(szFontNameString, "-*-%s-%s-r-normal--%d-*-*-*-*-*-*-*", i_szFontName, i_iThickness==700 ? "*":"bold", iFontHeightIn);
	//sadly this^ does not work any more (20240405), the below works but does not select font or thickness, use windows version to create fnt
	sprintf(szFontNameString, "r%d", abs(iFontHeightIn));
#endif
	sprintf(szFilenameTGA, "%s.tga", szOutputFontNameBase);
	sprintf(szFilenameFNT, "%s.fnt", szOutputFontNameBase);

	pFont = new C_FontGL();
	bResult = pFont->Load(szFontNameString, iFontHeightIn, iThickness);
	int iFontHeight, iTotalHeight, iBelowBase, iWasteY;
	pFont->GetHeights(&iFontHeight, &iTotalHeight, &iBelowBase);
	//iWasteY = (iTotalHeight-iFontHeight);
	iWasteY = 0;
	//the list will print iBelowBase pixels below the specified y
	// that is, to make a char fit in iFontHeight y must be set iBelowBase pixels above
	int iSurfaceWidth = pGraph->GetModeWidth();
	S_Rect stRect = {0, 0, iSurfaceWidth, 0};
	if(!bResult) goto out;
	bResult = false;

	stHeader.iMagicF  = 'F';
	stHeader.iMagicN  = 'N';
	stHeader.iChars  = 256-32;
	stHeader.iHeight = iTotalHeight;
	stHeader.iSpaceY = -iWasteY;
	pFile = fopen(szFilenameFNT, "wb");
	if(!pFile) goto out;
	fwrite(&stHeader, sizeof(stHeader), 1, pFile);

	x = 0;
	y = iFontHeight+iWasteY - iBelowBase;
	for(i=32; i<256; i++) { //ASCII 32 and up
		int w1, w2, w3;
		pFont->GetCharWidths(i, &w1, &w2, &w3);
		if(x+w2>iSurfaceWidth) {
			y += iFontHeight+iWasteY;
			x = 0;
		}
		if(x-w1<0) x=w1;
		pFont->Print(x-w1, y, "%c", i);
		stChar.iIndex = i;
		stChar.iPreX = w1;
		stChar.iPostX = w3;
		stChar.stRect.height = iFontHeight;
		stChar.stRect.width = w2;
		stChar.stRect.x = x;
		stChar.stRect.y = y-(iFontHeight-iWasteY) + iBelowBase;
		fwrite(&stChar, sizeof(stChar), 1, pFile);
		x += w2;
	}

	stRect.height = y+iWasteY + iBelowBase;
	pGraph->SaveRenderedToTGA(szFilenameTGA, &stRect, &stTransCol);

	bResult = true;
out:
	if(pFile) fclose(pFile);
	delete pFont;
	return bResult;
}

bool TestFont(char *szFontNameBase)
{
	bool bResult;
	C_GraphWrapperGL *pGraph = C_GraphWrapperGL::GetGraphWrapperObject();
	C_Font *pFont = new C_Font(_BGRA8888);
	bResult = pFont->Load(szFontNameBase, NULL);
	if(bResult) {
		S_FColor stColor = S_FCOLORMAKE(0,0,1,1); //blue
		pFont->SetColor(stColor);
		int iSurfaceWidth = pGraph->GetModeWidth();
		S_Rect stRect = {0, 512, iSurfaceWidth, 256};
		pGraph->Rect(&stRect); //make white rect
		pFont->Print(&stRect, FONT_CENTERED, "Font: %s\nTEST string 1\n:!_?/%%.,jpq^()", szFontNameBase);
	}
	delete pFont;

	return bResult;
}

#ifdef WIN32
LRESULT CALLBACK WindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	bool bTest;
	char szFontNameBase[128];

	C_GraphWrapperGL *pGraph = new C_GraphWrapperGL(WindowProc, NULL);
	bTest = pGraph->SetMode(512, 768, false);
	if(!bTest) {
		MessageBox(NULL, pGraph->GetErrorMsg(), "Error", MB_OK);
		goto out;
	}

	sprintf(szFontNameBase, "%s%d", FONTNAME, FONTSIZE);
	bTest = BuildFont((char*)FONTNAME, FONTSIZE, 700, szFontNameBase);
	if(!bTest) MessageBox(NULL, "Build font failed", "Error", MB_OK);
	bTest = TestFont(szFontNameBase);
	if(!bTest) MessageBox(NULL, "Test font failed", "Error", MB_OK);
	pGraph->Flip();

	//messageloop
	MSG stMsg;
	while(GetMessage(&stMsg, NULL, 0, 0)) {
		TranslateMessage(&stMsg);
		DispatchMessage(&stMsg);
	}
out:
	delete pGraph;
	return 0;
}
#else

bool HandleEvents() //returns true on exit
{
	XEvent event;

	C_GraphWrapperGL *pGraph = C_GraphWrapperGL::GetGraphWrapperObject();
	Display *pDisplay = pGraph->GetDisplay();
	while(XPending(pDisplay)) {
		XNextEvent(pDisplay, &event);

		switch(event.type) {
			case ClientMessage:
				//window manager messages
				if((event.xclient.format == 32) && ((unsigned int)event.xclient.data.l[0] == pGraph->m_Atom_DeleteWindow)) {
					return true;
				}
				break;
			case DestroyNotify: //apparantly does not happen (the above does the job)
				//window has been destroyed
				return true;
		}
	}
	return false;
}

int main(int argc, char *argv[])
{
	bool bTest;
	char szFontNameBase[128];

	C_GraphWrapperGL *pGraph = new C_GraphWrapperGL();
	bTest = pGraph->SetMode(512, 768, false);
	if(!bTest) {
		printf("Error: %s\n", pGraph->GetErrorMsg());
		goto out;
	}

	sprintf(szFontNameBase, "%s%d", FONTNAME, abs(FONTSIZE));
	bTest = BuildFont((char*)FONTNAME, abs(FONTSIZE), 700, szFontNameBase);
	if(!bTest) printf("error: build font failed\n");
	bTest = TestFont(szFontNameBase);
	if(!bTest) printf("error: test font failed\n");
	pGraph->Flip();

	//main loop
	while(1) {
		if(HandleEvents()) break;
		mssleep(10);
	}

out:
	delete pGraph;
	return 0;
}
#endif
