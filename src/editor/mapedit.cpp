
#include <vector>

#include "tileset.h"
#include "editor_world.h"

#include "graph.h"
#include "font.h"
#include "input.h"

#include "global.h"
#include "gui.h"

#include <stdio.h>

#define DEFAULT_WINDOW_W 1024
#define DEFAULT_WINDOW_H 768

#define PROGRAM_TITLE ("Boulder MapEditor v1.00")

//checks if a position is within a rectangle (_x1, _y1 noninclusive)
#define CHKRECT(_x, _y, _x1, _y1, _valx, _valy) ((_valx >= _x) && (_valx < _x1) && (_valy >= _y) && (_valy < _y1))

#ifdef WIN32
#define ERROR_MSG(szMsg) \
	MessageBox(NULL, szMsg, "Error", MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_SERVICE_NOTIFICATION);
#else
#define ERROR_MSG(szMsg) \
	printf("%s\n", szMsg);
#endif

#define STARTXY     16
#define TILESPACE   2
#define TILESETW    500
#define SPACING     16
#define BORDER_SIZE 6 //must be less than SPACING/2

////////////////////////////////////////////////////////////////////////////////

class C_Editor
{
public:
	C_Editor();
	~C_Editor();

	bool IsInited() { return m_bInited; };
	bool IsLoaded() { return m_pMap != NULL; };
	C_World* GetMap() { return m_pMap; };

	bool Open(const char* szFilename);

	void SetSize(int w, int h);
	void SetDrawingToolSize(int iSize);
	void CycleSelectedParam();
	void AddToSelectedParam(int n);
	void RedrawScreen();

	void Load();
	void Save();

	void SetTile(int x, int y);
	void MouseClick(int x, int y);
	void MouseClickR(int x, int y);
	void CropMap(int iSide);
	void ExtendMap(int iSide);

	bool InEditText() { return m_iSelectedParam == 4; };
	void Update();

private:
	static void ResizeCallback();

	bool InitInstance();
	void FreeAll();

	void DrawCurTile(int x, int y);
	void PrintParams(int x, int y);
	void DrawBackgroundRect(int x, int y, int w, int h, bool bFillBlack);

	char             m_szSavePath[MAX_PATH];

	//graph data
	C_GraphWrapper*  m_pGraph;
	C_Image*         m_pBKImg;
	C_Font*          m_pFont;
	bool             m_bInited;
	bool             m_bWasDrawn;

	//tileset data
	char             m_iCurTile;        //what tile user has selected
	int              m_iCurTileSquare;  //size of drawing tool

	int              m_iSelectedParam; //(1, 2, 3 or 4)
	int              m_iObjParam1; //param 3

	C_TileHandler*   m_pTileHandler = NULL;
	std::vector<int> m_oTileList;
	int              m_iNumTiles;
	int              m_iTileSize;
	int              m_iNumTilesX, m_iNumTilesY;
	int              m_iTilesetX, m_iTilesetY;

	//mapdata
	C_World*         m_pMap;
	int              m_iTime; //param 1
	int              m_iDiamonds; //param 2

	C_GUI* m_pDlg1;
	char m_szLevelDescription[32];
};

C_Editor* g_pEditor = NULL;

////////////////////////////////////////////////////////////////////////////////

C_Editor::C_Editor()
{
	char szPath[MAX_PATH];
	m_szSavePath[0] = 0;

	m_pGraph = NULL;
	m_pBKImg = NULL;
	m_pFont = NULL;

	m_bWasDrawn = false;
	m_bInited = InitInstance();

	if (m_bInited) {
		bool bResult;
		sprintf(szPath, "%s/editor_bk.jpg", C_Global::szDataPath);
		m_pBKImg = new C_Image(szPath, NULL, &bResult);
		m_bInited = bResult;
	}

	if (m_bInited) {
		S_FColor stColor = { 0.3f, 0.3f, 0.3f, 1.0f };
		m_pFont = new C_Font(C_Image::GetDefaultFormat());
		char szPath[MAX_PATH];
		sprintf(szPath, "%s/arial-14", C_Global::szDataPath);
		m_bInited = m_pFont->Load(szPath, NULL);
		m_pFont->SetColor(stColor);
		int iTabStop[2] = { 2, 90 };
		m_pFont->SetTablist(iTabStop);
	}

	sprintf(szPath, "%s/%s", C_Global::szDataPath, "gui_desc1.txt");
	m_pDlg1 = new C_GUI();
	m_pDlg1->LoadTemplate(szPath, NULL, 0, 0);

	m_pMap = NULL;

	m_iSelectedParam = 3;
	m_iObjParam1 = 0;
	m_iTime = 60;
	m_iDiamonds = 10;
	
}

C_Editor::~C_Editor()
{
	FreeAll();
}

void C_Editor::ResizeCallback()
{
	g_pEditor->RedrawScreen();
}

bool C_Editor::InitInstance()
{
	m_pGraph = new C_GraphWrapper();

	bool bTest = m_pGraph->SetMode(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H, false, true);
	if (!bTest) {
		ERROR_MSG("Error initializing SDL2");
		return false;
	}
	m_pGraph->SetWindowTitle(PROGRAM_TITLE);

	m_pGraph->SetLiveResizeCallback(&ResizeCallback);

	m_pTileHandler = new C_TileHandler(m_pGraph);
	m_pTileHandler->Load(C_Global::szDataPath, "tiledesc.txt");

	for (int i = 0; i < TILENUMBERLIMIT; i++) {
		S_TileInfo* pTileInfo = m_pTileHandler->GetTileInfo(i);
		if (pTileInfo->iNumFrames > 0) {
			//do not add objects that are not to be in editor
			if (i != 0x0016 && i != 0x00A2 && i != 0x00B0 && i != 0x00FF)
			{
				m_oTileList.push_back(i);
			}
		}
	}
	m_iNumTiles = (int)m_oTileList.size();
	m_iTileSize = m_pTileHandler->m_iTileSize;
	m_iNumTilesX = TILESETW / (m_iTileSize + TILESPACE);
	m_iNumTilesY = (m_iNumTiles / m_iNumTilesX) + 1;

	m_iCurTile = 0;
	m_iCurTileSquare = 1;

	return true;
}

void C_Editor::FreeAll()
{
	delete m_pTileHandler; m_pTileHandler = NULL;
	delete m_pMap;   m_pMap = NULL;
	delete m_pFont;  m_pFont = NULL;
	delete m_pGraph; m_pGraph = NULL;
	delete m_pDlg1;  m_pDlg1 = NULL;
	delete m_pBKImg; m_pBKImg = NULL;
}

bool C_Editor::Open(const char* szFilename)
{
	delete m_pMap;
	m_pMap = NULL;

	char szFile[MAX_PATH];
	strcpy(szFile, szFilename);
	char* szExt = strrchr(szFile, '.');
	if (szExt && (_stricmp(szExt, ".map") == 0)) {
	}
	else {
		ERROR_MSG("No file selected!");
		return false;
	}

	bool bNewFile = true;
	FILE* pFile = fopen(szFilename, "rb");
	if (pFile) {
		fclose(pFile);
		bNewFile = false;
	}
	if (bNewFile) {
		m_pMap = new C_World(szFilename, 64, 64, m_pTileHandler);
		m_pMap->SetMapParameters(m_iDiamonds, m_iTime);
	}
	else {
		m_pMap = new C_World(szFilename, m_pTileHandler);
		m_pMap->GetMapParameters(&m_iDiamonds, &m_iTime);
		strcpy(m_szLevelDescription, m_pMap->GetMapDescription());
		m_pDlg1->SetText(1, m_szLevelDescription);
	}

	sprintf(szFile, "%s %s", PROGRAM_TITLE, szFilename);
	m_pGraph->SetWindowTitle(szFile);
	RedrawScreen();
	return true;
}

void C_Editor::SetSize(int w, int h)
{
	m_pGraph->SetSize(w, h);
	RedrawScreen();
}

void C_Editor::Load()
{
	m_pMap->Load();
	m_pMap->GetMapParameters(&m_iDiamonds, &m_iTime);
	strcpy(m_szLevelDescription, m_pMap->GetMapDescription());
	m_pDlg1->SetText(1, m_szLevelDescription);
}

void C_Editor::Save()
{
	m_pDlg1->GetText(1, m_szLevelDescription);
	m_pMap->SetMapDescription(m_szLevelDescription);
	if (!m_pMap->Save()) {
		ERROR_MSG("Cannot open file for writing.");
	}
}

void C_Editor::CropMap(int iSide)
{
	m_pMap->Crop(iSide);
}

void C_Editor::ExtendMap(int iSide)
{
	m_pMap->Extend(iSide);
}

void C_Editor::CycleSelectedParam()
{
	m_iSelectedParam++;
	if (m_iSelectedParam > 4) m_iSelectedParam = 1;
	if (m_iSelectedParam == 4) {
		m_pDlg1->SetFocused(1);
	} else m_pDlg1->SetFocused(1, STATE_DISABLED);
}
void C_Editor::AddToSelectedParam(int n)
{
	if (m_iSelectedParam == 3) {
		m_iObjParam1 += n;
		int iLimit = 0;

		switch (m_oTileList[m_iCurTile]) {
		case OBJ_ENEMY0:
		case OBJ_ENEMY1:
		case OBJ_ENEMY2:
		case OBJ_ENEMY3:
		case OBJ_ENEMY4:
			iLimit = 8;
			break;
		case OBJ_SLIME:
			iLimit = 1;
			break;
		case OBJ_KEY:
		case OBJ_DOOR:
		case OBJ_DOOR_GREY:
			iLimit = 3;
			break;
		case OBJ_MUD:
			iLimit = 1;
			break;
		case OBJ_OBJECT_MACHINE:
			iLimit = 3;
			break;
		default:
			break;
		}

		if (m_iObjParam1 < 0) m_iObjParam1 = 0;
		if (m_iObjParam1 > iLimit) m_iObjParam1 = iLimit;
	}
	if (m_iSelectedParam == 1) {
		m_iTime += n;
		if (m_iTime < 10) m_iTime = 10;
		if (m_iTime > 3600) m_iTime = 3600;
		m_pMap->SetMapParameters(m_iDiamonds, m_iTime);
	}
	if (m_iSelectedParam == 2) {
		m_iDiamonds += n;
		if (m_iDiamonds < 0) m_iDiamonds = 0;
		if (m_iDiamonds > 1000) m_iDiamonds = 1000;
		m_pMap->SetMapParameters(m_iDiamonds, m_iTime);
	}
}

void C_Editor::SetDrawingToolSize(int iSize)
{
	m_iCurTileSquare = iSize;
	if (m_iCurTileSquare < 1) m_iCurTileSquare = 1;
	if (m_iCurTileSquare > 5) m_iCurTileSquare = 5;
}

void C_Editor::MouseClick(int x, int y)
{
	//if (!m_bWasDrawn) return;
	int rx, ry, rx1, ry1, j, i;
	char iTilenum = 0;
	int iPosX = m_iTilesetX;
	int iPosY = m_iTilesetY;

	int iTileSize = m_pMap->GetTileSize();

	//change current tile
	for (j = 0; j < m_iNumTilesY; j++) {
		for (i = 0; i < m_iNumTilesX; i++) {
			rx = iPosX + i * (iTileSize + TILESPACE);
			ry = iPosY + j * (iTileSize + TILESPACE);
			rx1 = rx + iTileSize;
			ry1 = ry + iTileSize;
			if (CHKRECT(rx, ry, rx1, ry1, x, y)) {
				m_iCurTile = iTilenum;
				j = m_iNumTilesY;
				break;
			}
			iTilenum++;
			if (iTilenum >= m_iNumTiles) {
				j = m_iNumTilesY;
				break;
			}
		}
	}
	AddToSelectedParam(0); //update param limits

	SetTile(x, y);
}

void C_Editor::MouseClickR(int x, int y)
{
	int iTile;

	S_Rect* pstRect = m_pMap->GetScreenRect();
	if (CHKRECT(pstRect->x, pstRect->y, pstRect->x + pstRect->width, pstRect->y + pstRect->height, x, y)) {
		m_pMap->GetElement(x, y, &iTile, &m_iObjParam1);
		for (int i = 0; i < (int)m_oTileList.size(); i++) {
			if (m_oTileList[i] == iTile) {
				m_iCurTile = i;
				break;
			}
		}
	}
}

//if the coordinate is within the edit-window, the map is updated
void C_Editor::SetTile(int x, int y)
{
	//if (!m_bWasDrawn) return;
	int iTileSize = m_pMap->GetTileSize();
	S_Rect* pstRect = m_pMap->GetScreenRect();

	if (CHKRECT(pstRect->x, pstRect->y, pstRect->x + pstRect->width, pstRect->y + pstRect->height, x, y)) {
		int i, j;
		for (i = 0; i < m_iCurTileSquare; i++) {
			for (j = 0; j < m_iCurTileSquare; j++) {
				m_pMap->SetElement(x + i * m_iTileSize, y + j * m_iTileSize, m_oTileList[m_iCurTile], m_iObjParam1);
			}
		}
	}
}

void C_Editor::DrawCurTile(int x, int y)
{
	int i, j;

	//fill with black first
	S_FColor stC = S_FCOLORMAKE(0.0f, 0.0f, 0.0f, 1.0f);
	m_pGraph->SetColor(&stC);
	S_Rect stR = { x, y, m_iCurTileSquare * m_iTileSize, m_iCurTileSquare * m_iTileSize };
	m_pGraph->Rect(&stR);

	for (i = 0; i < m_iCurTileSquare; i++) {
		for (j = 0; j < m_iCurTileSquare; j++) {
			m_pTileHandler->Draw(m_oTileList[m_iCurTile], 0, x + i * m_iTileSize, y + j * m_iTileSize, NULL);
		}
	}
}

void C_Editor::PrintParams(int x, int y)
{
	int iOffs[4] = { 2 + 9, 18 + 9, 40 + 9, 190 + 9 };
	S_FColor stCol = S_FCOLORMAKE(0, 1, 0, 1);
	m_pGraph->SetColor(&stCol);
	m_pGraph->Circle(x, y + iOffs[m_iSelectedParam - 1], 4);

	m_pFont->Print(x + 10, y + 2, "Time\t%d", m_iTime);
	m_pFont->Print(x + 10, y + 18, "Diamonds\t%d", m_iDiamonds);
	m_pFont->Print(x + 10, y + 40, "Parameter\t%d", m_iObjParam1);

	m_pDlg1->SetPos(x + 10, y + 182);
	m_pDlg1->Draw();

	int i, j;
	char szTemp[50]; szTemp[0] = 0;
	C_Image* pImage = NULL;

	switch (m_oTileList[m_iCurTile]) {
	case OBJ_ENEMY0:
	case OBJ_ENEMY1:
	case OBJ_ENEMY2:
	case OBJ_ENEMY3:
	case OBJ_ENEMY4:
		sprintf(szTemp, "EXPLODES INTO: ");
		if (m_iObjParam1 > 0 && m_iObjParam1 <= 8) {
			//fill with black first
			S_FColor stC = S_FCOLORMAKE(0.0f, 0.0f, 0.0f, 1.0f);
			m_pTileHandler->m_pGraph->SetColor(&stC);
			S_Rect stR = { x + 10, y + 78, 3 * m_iTileSize, 3 * m_iTileSize };
			m_pTileHandler->m_pGraph->Rect(&stR);
			//draw 3x3
			for (i = 0; i < 3; i++) {
				for (j = 0; j < 3; j++) {
					int iTile = ENEMY_EXPLODE_INTO[m_iObjParam1][j * 3 + i];
					int iFrame = ENEMY_EXPLODE_INTO_PARAM[m_iObjParam1][j * 3 + i];
					m_pTileHandler->Draw(iTile, iFrame, x + 10 + i * m_iTileSize, y + 78 + j * m_iTileSize, NULL);
				}
			}
		}
		else if (m_iObjParam1 == 0) {
			strcat(szTemp, "\n-");
		}
		else strcat(szTemp, "\nERROR");
		break;
	case OBJ_SLIME:
		if (m_iObjParam1 == 0) sprintf(szTemp, "SLIME IS\nSTABLE");
		else if (m_iObjParam1 > 0) sprintf(szTemp, "SLIME IS\nGROWING");
		else sprintf(szTemp, "ERROR");
		break;
	case OBJ_KEY:
	case OBJ_DOOR:
	case OBJ_DOOR_GREY:
		if (m_iObjParam1 == 0) sprintf(szTemp, "RED");
		else if (m_iObjParam1 == 1) sprintf(szTemp, "BLUE");
		else if (m_iObjParam1 == 2) sprintf(szTemp, "YELLOW");
		else if (m_iObjParam1 == 3) sprintf(szTemp, "GREEN");
		else sprintf(szTemp, "ERROR");
		break;
	case OBJ_MUD:
		if (m_iObjParam1 == 0) sprintf(szTemp, "MUD IS\nEMPTY");
		else if (m_iObjParam1 > 0) sprintf(szTemp, "MUD IS\nHOLDING STONE");
		else sprintf(szTemp, "ERROR");
		break;
	case OBJ_OBJECT_MACHINE:
		if (m_iObjParam1 == 0) sprintf(szTemp, "MACHINE PRODUCE\nSTONE");
		else if (m_iObjParam1 == 1) sprintf(szTemp, "MACHINE PRODUCE\nRED DIAM");
		else if (m_iObjParam1 == 2) sprintf(szTemp, "MACHINE PRODUCE\nBLUE DIAM");
		else if (m_iObjParam1 == 3) sprintf(szTemp, "MACHINE PRODUCE\nBOMB");
		else sprintf(szTemp, "ERROR");
		break;
	default:
		sprintf(szTemp, "PARAMS UNUSED");
		break;
	}

	if (strlen(szTemp) > 0) {
		S_Rect stR = { x + 10, y + 56, 150, 44 };
		m_pFont->Print(&stR, 0, "%s", szTemp);
	}
}

void C_Editor::DrawBackgroundRect(int x, int y, int w, int h, bool bFillBlack)
{
	int iImgW, iImgH;
	m_pBKImg->GetInfo(&iImgW, &iImgH, NULL);

	S_Rect stBR[9];
	stBR[0] = S_RECTMAKE(0, 0, BORDER_SIZE, BORDER_SIZE);                   //u left corner
	stBR[1] = S_RECTMAKE(iImgW - BORDER_SIZE, 0, BORDER_SIZE, BORDER_SIZE); //u right corner
	stBR[2] = S_RECTMAKE(0, iImgH - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE); //l left corner
	stBR[3] = S_RECTMAKE(iImgW - BORDER_SIZE, iImgH - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE); //l right corner
	stBR[4] = S_RECTMAKE(BORDER_SIZE, 0, 2, BORDER_SIZE);                   //upper
	stBR[5] = S_RECTMAKE(BORDER_SIZE, iImgH - BORDER_SIZE, 2, BORDER_SIZE); //lower
	stBR[6] = S_RECTMAKE(0, BORDER_SIZE, BORDER_SIZE, 2);                   //left
	stBR[7] = S_RECTMAKE(iImgW - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, 2); //right
	stBR[8] = S_RECTMAKE(BORDER_SIZE, BORDER_SIZE, 4, 4);                   //middle

	//dst rects
	int w2 = w - BORDER_SIZE;
	int h2 = h - BORDER_SIZE;

	S_Rect dstRectW1 = S_RECTMAKE(x + BORDER_SIZE, y, w - 2 * BORDER_SIZE, BORDER_SIZE);
	S_Rect dstRectW2 = S_RECTMAKE(x + BORDER_SIZE, y + h - BORDER_SIZE, w - 2 * BORDER_SIZE, BORDER_SIZE);
	S_Rect dstRectH1 = S_RECTMAKE(x, y + BORDER_SIZE, BORDER_SIZE, h - 2 * BORDER_SIZE);
	S_Rect dstRectH2 = S_RECTMAKE(x + w - BORDER_SIZE, y + BORDER_SIZE, BORDER_SIZE, h - 2 * BORDER_SIZE);
	S_Rect dstRectM = S_RECTMAKE(x + BORDER_SIZE, y + BORDER_SIZE, w - 2 * BORDER_SIZE, h - 2 * BORDER_SIZE);

	//draw
	m_pGraph->Blt(m_pBKImg, &stBR[0], x, y);
	m_pGraph->Blt(m_pBKImg, &stBR[1], x + w2, y);
	m_pGraph->Blt(m_pBKImg, &stBR[2], x, y + h2);
	m_pGraph->Blt(m_pBKImg, &stBR[3], x + w2, y + h2);

	m_pGraph->Blt(m_pBKImg, &stBR[4], &dstRectW1);
	m_pGraph->Blt(m_pBKImg, &stBR[5], &dstRectW2);
	m_pGraph->Blt(m_pBKImg, &stBR[6], &dstRectH1);
	m_pGraph->Blt(m_pBKImg, &stBR[7], &dstRectH2);

	if (bFillBlack) {
		//fill with black in the middle
		S_FColor stC = S_FCOLORMAKE(0.0f, 0.0f, 0.0f, 1.0f);
		m_pGraph->SetColor(&stC);
		m_pGraph->Rect(&dstRectM);
	}
	else {
		m_pGraph->Blt(m_pBKImg, &stBR[8], &dstRectM);
	}
}

void C_Editor::RedrawScreen()
{
	int w = m_pGraph->GetModeWidth();
	int h = m_pGraph->GetModeHeight();

	int iPosX, iPosY, iHelpW, iHelpH;
	m_pBKImg->GetInfo(&iHelpW, &iHelpH, NULL);
	iPosX = w - (iHelpW + SPACING - BORDER_SIZE);
	iPosY = h - (iHelpH + SPACING - BORDER_SIZE);

	//background
	S_FColor stBlack = S_FCOLORMAKE(0, 0, 0, 1);
	m_pGraph->SetColor(&stBlack);
	m_pGraph->Rect(NULL, false);
	//help
	if (iPosX > 0 && iPosY > 0) m_pGraph->Blt(m_pBKImg, NULL, iPosX, iPosY);

	m_bWasDrawn = false;
	if (m_pMap) {
		int iTileSize = m_pMap->GetTileSize();
		int iNumTilesX = TILESETW / (iTileSize + TILESPACE);
		int iNumTilesY = m_iNumTiles / iNumTilesX;
		if (m_iNumTiles % iNumTilesX > 0) iNumTilesY++;
		int iMapAreaW = w - ((5 * iTileSize) + 2 * SPACING + STARTXY);
		iHelpH -= 2 * BORDER_SIZE;
		if (iHelpH < iNumTilesY * (TILESPACE + iTileSize)) iHelpH = iNumTilesY * (TILESPACE + iTileSize);
		int iMapAreaH = h - (STARTXY + 2 * SPACING + iHelpH);

		//draw?
		if (iMapAreaH > (368 + STARTXY + SPACING) && w > (iNumTilesX * (iTileSize + TILESPACE) + SPACING + iHelpW + SPACING)) {
			//center map
			int w2, h2;
			S_Rect stRect = { STARTXY, STARTXY, iMapAreaW, iMapAreaH };
			m_pMap->GetMapSize(&w2, &h2);
			if (iMapAreaW > w2) {
				stRect.x = STARTXY + (iMapAreaW - w2) / 2;
				stRect.width = w2;
			}
			if (iMapAreaH > h2) {
				stRect.y = STARTXY + (iMapAreaH - h2) / 2;
				stRect.height = h2;
			}
			m_pMap->SetScreenRect(&stRect);
			m_pMap->MoveRelative(0, 0);

			//tileset
			m_iTilesetX = iPosX = STARTXY;
			m_iTilesetY = iPosY = STARTXY + iMapAreaH + SPACING;
			w = iNumTilesX * (iTileSize + TILESPACE);
			h = iNumTilesY * (iTileSize + TILESPACE);
			DrawBackgroundRect(iPosX - BORDER_SIZE, iPosY - BORDER_SIZE, w + 2 * BORDER_SIZE, h + 2 * BORDER_SIZE, true);
			int i, j;
			char iTilenum = 0;
			for (j = 0; j < iNumTilesY; j++) {
				for (i = 0; i < iNumTilesX; i++) {
					m_pTileHandler->Draw(m_oTileList[iTilenum], 0, iPosX + i * (iTileSize + TILESPACE), iPosY + j * (iTileSize + TILESPACE), NULL);
					iTilenum++;
					if (iTilenum >= m_iNumTiles) {
						j = iNumTilesY;
						break;
					}
				}
			}

			//drawing tool
			iPosX = STARTXY + iMapAreaW + SPACING;
			iPosY = STARTXY;
			w = 5 * iTileSize;
			h = 5 * iTileSize;
			DrawBackgroundRect(iPosX - BORDER_SIZE, iPosY - BORDER_SIZE, w + 2 * BORDER_SIZE, h + 2 * BORDER_SIZE, false);
			DrawCurTile(iPosX, iPosY);

			//text
			iPosX = STARTXY + iMapAreaW + SPACING;
			iPosY = STARTXY + 5 * iTileSize + SPACING;
			w = 5 * iTileSize;
			h = 7 * iTileSize;
			DrawBackgroundRect(iPosX - BORDER_SIZE, iPosY - BORDER_SIZE, w + 2 * BORDER_SIZE, h + 2 * BORDER_SIZE, false);
			PrintParams(iPosX + 6, iPosY);

			//map
			DrawBackgroundRect(stRect.x - BORDER_SIZE, stRect.y - BORDER_SIZE, stRect.width + 2 * BORDER_SIZE, stRect.height + 2 * BORDER_SIZE, false);
			m_pMap->Draw();
			m_bWasDrawn = true;
		}
	}

	m_pGraph->Flip();
}

void C_Editor::Update()
{
	if (m_iSelectedParam == 4) {
		m_pDlg1->UpdateInput();
	}
}

////////////////////////////////////////////////////////////////////////////////
//os

C_Control* g_pControl = NULL;
bool       g_bRedraw = false; //global redraw, set by message handler

//mouseinfo
bool       g_bLButton = false;
bool       g_bRButton = false;
int        g_iMouseX;
int        g_iMouseY;

C_Timer*   g_pAccTimer = new C_Timer();
double     g_dAccTime = 0;

void Update()
{
	bool bRedraw = false;

	g_pControl->Update();

	//get and handle input events
	int iX, iY;
	int iMEvent = g_pControl->GetMouseEvent(&iX, &iY, NULL);
	bool bShiftPressed = g_pControl->IsPressed(DIK_RSHIFT) || g_pControl->IsPressed(DIK_LSHIFT);
	bool bControlPressed = g_pControl->IsPressed(DIK_RCONTROL) || g_pControl->IsPressed(DIK_LCONTROL);

	switch (iMEvent) {
	case EVT_MOUSE_MOVE:
		if (!g_pEditor->IsLoaded()) break;
		g_iMouseX = iX;
		g_iMouseY = iY;
		if (g_bLButton) {
			g_pEditor->SetTile(g_iMouseX, g_iMouseY);
			bRedraw = true;
		}
		break;
	case EVT_MOUSE_LEFT_DOWN:
		if (!g_pEditor->IsLoaded()) break;
		g_bLButton = true;
		g_pEditor->MouseClick(iX, iY);
		bRedraw = true;
		break;
	case EVT_MOUSE_LEFT_UP:
		g_bLButton = false;
		break;
	case EVT_MOUSE_RIGHT_DOWN:
		if (!g_pEditor->IsLoaded()) break;
		g_bRButton = true;
		g_pEditor->MouseClickR(iX, iY);
		bRedraw = true;
		break;
	case EVT_MOUSE_RIGHT_UP:
		g_bRButton = false;
		break;
	}

	if (!g_pEditor->IsLoaded()) return;
	int iTileSize = g_pEditor->GetMap()->GetTileSize();

	if (g_pControl->IsPressedOnce(DIK_TAB)) {
		g_pEditor->CycleSelectedParam();
		bRedraw = true;
	}

	g_pEditor->Update();

	if (!g_pEditor->InEditText()) {
		bool bAdd = false;
		bool bSub = false;
		if (g_pControl->IsPressedRepeat(DIK_ADD) || g_pControl->IsPressedRepeat(DIK_BACKSLASH)) {
			bAdd = true;
			bRedraw = true;
		}
		if (g_pControl->IsPressedRepeat(DIK_SUBTRACT) || g_pControl->IsPressedRepeat(DIK_MINUS)) {
			bSub = true;
			bRedraw = true;
		}
		if (bAdd || bSub) {
			double t = g_pAccTimer->GetInterval();
			if (t > 500.0) {
				g_pAccTimer->Reset();
				g_dAccTime = 0;
			}
			else g_dAccTime += t;

			int iNum = 1;
			if (g_dAccTime > 3500) iNum = 10;

			if (bAdd) g_pEditor->AddToSelectedParam(iNum);
			if (bSub) g_pEditor->AddToSelectedParam(-iNum);
		}

		if (g_pControl->IsPressedOnce(DIK_S)) {
			g_pEditor->Save();
		}
		if (g_pControl->IsPressedOnce(DIK_L)) {
			bRedraw = true;
			g_pEditor->Load();
		}

		if (g_pControl->IsPressedOnce(DIK_F1)) {
			bRedraw = true;
			g_pEditor->SetDrawingToolSize(1);
		}
		if (g_pControl->IsPressedOnce(DIK_F2)) {
			bRedraw = true;
			g_pEditor->SetDrawingToolSize(2);
		}
		if (g_pControl->IsPressedOnce(DIK_F3)) {
			bRedraw = true;
			g_pEditor->SetDrawingToolSize(3);
		}
		if (g_pControl->IsPressedOnce(DIK_F4)) {
			bRedraw = true;
			g_pEditor->SetDrawingToolSize(4);
		}
		if (g_pControl->IsPressedOnce(DIK_F5)) {
			bRedraw = true;
			g_pEditor->SetDrawingToolSize(5);
		}

		if (g_pControl->IsPressedRepeat(DIK_DOWN)) {
			bRedraw = true;
			g_pEditor->GetMap()->MoveRelative(0, iTileSize / 2);
			if (g_bLButton) g_pEditor->SetTile(g_iMouseX, g_iMouseY);
		}
		if (g_pControl->IsPressedRepeat(DIK_UP)) {
			bRedraw = true;
			g_pEditor->GetMap()->MoveRelative(0, -iTileSize / 2);
			if (g_bLButton) g_pEditor->SetTile(g_iMouseX, g_iMouseY);
		}
		if (g_pControl->IsPressedRepeat(DIK_RIGHT)) {
			bRedraw = true;
			g_pEditor->GetMap()->MoveRelative(iTileSize / 2, 0);
			if (g_bLButton) g_pEditor->SetTile(g_iMouseX, g_iMouseY);
		}
		if (g_pControl->IsPressedRepeat(DIK_LEFT)) {
			bRedraw = true;
			g_pEditor->GetMap()->MoveRelative(-iTileSize / 2, 0);
			if (g_bLButton) g_pEditor->SetTile(g_iMouseX, g_iMouseY);
		}

		if (g_pControl->IsPressedRepeat(DIK_HOME) || g_pControl->IsPressedRepeat(DIK_F)) {
			bRedraw = true;
			if (bShiftPressed) g_pEditor->CropMap(0);
			else if (bControlPressed) g_pEditor->ExtendMap(0);
			else g_pEditor->GetMap()->MoveRelative(-(g_pEditor->GetMap()->GetScreenRect()->width / iTileSize) * iTileSize, 0);
		}
		if (g_pControl->IsPressedRepeat(DIK_END) || g_pControl->IsPressedRepeat(DIK_H)) {
			bRedraw = true;
			if (bShiftPressed) g_pEditor->CropMap(1);
			else if (bControlPressed) g_pEditor->ExtendMap(1);
			else g_pEditor->GetMap()->MoveRelative((g_pEditor->GetMap()->GetScreenRect()->width / iTileSize) * iTileSize, 0);
		}
		if (g_pControl->IsPressedRepeat(DIK_PRIOR) || g_pControl->IsPressedRepeat(DIK_T)) { //page up
			bRedraw = true;
			if (bShiftPressed) g_pEditor->CropMap(2);
			else if (bControlPressed) g_pEditor->ExtendMap(2);
			else g_pEditor->GetMap()->MoveRelative(0, -(g_pEditor->GetMap()->GetScreenRect()->height / iTileSize) * iTileSize);
		}
		if (g_pControl->IsPressedRepeat(DIK_NEXT) || g_pControl->IsPressedRepeat(DIK_G)) { //page down
			bRedraw = true;
			if (bShiftPressed) g_pEditor->CropMap(3);
			else if (bControlPressed) g_pEditor->ExtendMap(3);
			else g_pEditor->GetMap()->MoveRelative(0, (g_pEditor->GetMap()->GetScreenRect()->height / iTileSize) * iTileSize);
		}
	} else bRedraw = true;

	//draw if needed
	if (g_bRedraw || bRedraw) g_pEditor->RedrawScreen();
	g_bRedraw = false;
}

bool HandleEvents() //returns true on exit
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_KEYUP:
		case SDL_KEYDOWN:
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (g_pControl) g_pControl->PollInput(&e);
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
				g_pEditor->SetSize(e.window.data1, e.window.data2);
			if (e.window.event == SDL_WINDOWEVENT_EXPOSED)
				g_pEditor->RedrawScreen();
			break;
		case SDL_QUIT:
			return true;
		}
	}
	return false;
}

//main
#ifdef WIN32
int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
	bool bShowDialog = false;
	char szOpenName[2048];
	szOpenName[0] = 0;

	C_Global::Init("tiledesc.txt", "Boulder");

#if defined WIN32
	if (lpCmdLine[0] == 0) {
		bShowDialog = true;
	}
	else {
		strcpy(szOpenName, lpCmdLine);
		if (szOpenName[0] == '"') {
			C_Global::GetWithin(szOpenName, '"'); //remove ""
		}
	}
#else
	if (argc != 2) {
		//ERROR_MSG("Usage: mapeditor [file.map]");
		//goto out;
		bShowDialog = true;
	} else strcpy(szOpenName, argv[1]);
#endif

	if (bShowDialog) {
		szOpenName[0] = 0;
		C_Global::OpenDialog(szOpenName, sizeof(szOpenName),
			C_Global::szAppDataPath,
			(char*)"map\0\0", (char*)"MapFiles (.map)\0\0");
	}

#ifdef __APPLE__
	if (szOpenName[0] == 0 || strstr(szOpenName, "*")) {
		//on mac you cannot choose a new file that does not exist in the open dialog
		if (bShowDialog) {
			szOpenName[0] = 0;
			C_Global::SaveDialog(szOpenName, sizeof(szOpenName),
				C_Global::szAppDataPath,
				(char*)"map\0\0", (char*)"MapFiles (.map)\0\0");
		}
	}
#endif
	if (szOpenName[0] == 0 || strstr(szOpenName, "*")) {
		//no file to open, set default
		sprintf(szOpenName, "%s/%s", C_Global::szAppDataPath, "newlevel.map");
	}

	g_pControl = new C_Control();

	g_pEditor = new C_Editor();
	if (!g_pEditor->IsInited()) goto out;

	if (!g_pEditor->Open(szOpenName)) goto out;

	//main loop
	while (1) {
		if (HandleEvents()) break;
		Update(); //updates input and does redraw on editor events
		mssleep(10);
	}
out:
	delete g_pEditor;
	delete g_pControl;
	delete g_pAccTimer;
#ifdef WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
#endif
	return 0;
}
