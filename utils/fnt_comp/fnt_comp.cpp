
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
	C_GraphWrapperGL *pclGraph = C_GraphWrapperGL::GetGraphWrapperObject();
	S_Color stTransCol = {0,0,0,0};
	S_FontHeader stHeader;
	S_CharHeader stChar;
	C_FontGL *pclFont = NULL;
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

	pclFont = new C_FontGL();
	bResult = pclFont->Load(szFontNameString, iFontHeightIn, iThickness);
	int iFontHeight, iTotalHeight, iBelowBase, iWasteY;
	pclFont->GetHeights(&iFontHeight, &iTotalHeight, &iBelowBase);
	//iWasteY = (iTotalHeight-iFontHeight);
	iWasteY = 0;
	//the list will print iBelowBase pixels below the specified y
	// that is, to make a char fit in iFontHeight y must be set iBelowBase pixels above
	int iSurfaceWidth = pclGraph->GetModeWidth();
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
		pclFont->GetCharWidths(i, &w1, &w2, &w3);
		if(x+w2>iSurfaceWidth) {
			y += iFontHeight+iWasteY;
			x = 0;
		}
		if(x-w1<0) x=w1;
		pclFont->Print(x-w1, y, "%c", i);
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
	pclGraph->SaveRenderedToTGA(szFilenameTGA, &stRect, &stTransCol);

	bResult = true;
out:
	if(pFile) fclose(pFile);
	delete pclFont;
	return bResult;
}

bool TestFont(char *szFontNameBase)
{
	bool bResult;
	C_GraphWrapperGL *pclGraph = C_GraphWrapperGL::GetGraphWrapperObject();
	C_Font *pclFont = new C_Font(_BGRA8888);
	bResult = pclFont->Load(szFontNameBase, NULL);
	if(bResult) {
		S_FColor stColor = S_FCOLORMAKE(0,0,1,1); //blue
		pclFont->SetColor(stColor);
		int iSurfaceWidth = pclGraph->GetModeWidth();
		S_Rect stRect = {0, 512, iSurfaceWidth, 256};
		pclGraph->Rect(&stRect); //make white rect
		pclFont->Print(&stRect, FONT_CENTERED, "Font: %s\nTEST string 1\n:!_?/%%.,jpq^()", szFontNameBase);
	}
	delete pclFont;

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

	C_GraphWrapperGL *pclGraph = new C_GraphWrapperGL(WindowProc, NULL);
	bTest = pclGraph->SetMode(512, 768, false);
	if(!bTest) {
		MessageBox(NULL, pclGraph->GetErrorMsg(), "Error", MB_OK);
		goto out;
	}

	sprintf(szFontNameBase, "%s%d", FONTNAME, FONTSIZE);
	bTest = BuildFont((char*)FONTNAME, FONTSIZE, 700, szFontNameBase);
	if(!bTest) MessageBox(NULL, "Build font failed", "Error", MB_OK);
	bTest = TestFont(szFontNameBase);
	if(!bTest) MessageBox(NULL, "Test font failed", "Error", MB_OK);
	pclGraph->Flip();

	//messageloop
	MSG stMsg;
	while(GetMessage(&stMsg, NULL, 0, 0)) {
		TranslateMessage(&stMsg);
		DispatchMessage(&stMsg);
	}
out:
	delete pclGraph;
	return 0;
}
#else

bool HandleEvents() //returns true on exit
{
	XEvent event;

	C_GraphWrapperGL *pclGraph = C_GraphWrapperGL::GetGraphWrapperObject();
	Display *pclDisplay = pclGraph->GetDisplay();
	while(XPending(pclDisplay)) {
		XNextEvent(pclDisplay, &event);

		switch(event.type) {
			case ClientMessage:
				//window manager messages
				if((event.xclient.format == 32) && ((unsigned int)event.xclient.data.l[0] == pclGraph->m_Atom_DeleteWindow)) {
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

	C_GraphWrapperGL *pclGraph = new C_GraphWrapperGL();
	bTest = pclGraph->SetMode(512, 768, false);
	if(!bTest) {
		printf("Error: %s\n", pclGraph->GetErrorMsg());
		goto out;
	}

	sprintf(szFontNameBase, "%s%d", FONTNAME, abs(FONTSIZE));
	bTest = BuildFont((char*)FONTNAME, abs(FONTSIZE), 700, szFontNameBase);
	if(!bTest) printf("error: build font failed\n");
	bTest = TestFont(szFontNameBase);
	if(!bTest) printf("error: test font failed\n");
	pclGraph->Flip();

	//main loop
	while(1) {
		if(HandleEvents()) break;
		mssleep(10);
	}

out:
	delete pclGraph;
	return 0;
}
#endif
