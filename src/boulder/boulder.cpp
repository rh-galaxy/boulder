
#include <vector>

#include "tileset.h"
#include "game_world.h"

#include "graph.h"
#include "font.h"
#include "input.h"
#include "soundfx.h"

#include "global.h"
#include "fileresource.h"

#include "gui.h"
#include "hiscorelist.h"

#include <stdio.h>

#define DEFAULT_WINDOW_W 1024
#define DEFAULT_WINDOW_H 768

#define PROGRAM_TITLE ("Boulder v1.01")

#ifdef WIN32
#define ERROR_MSG(szMsg) \
	MessageBox(NULL, szMsg, "Error", MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_SERVICE_NOTIFICATION);
#else
#define ERROR_MSG(szMsg) \
	printf("%s\n", szMsg);
#endif

#define STARTXY 8 //must be >= BORDER
#define BORDER  8
#define SPACING 8

class C_Game
{
public:
	C_Game();
	~C_Game();

	bool IsInited() { return m_bInited; };

	void OpenSelectedHiscore(int iMarkedPlace = -1);
	bool OpenSelectedMap();

	void SetSize(int w, int h);
	bool Update();
	void DrawScreen();

	//dialog
	void LoadLastName();
	void SaveLastName();

private:
	static void ResizeCallback();

	bool InitInstance();
	void LoadSound();
	void FreeAll();
	bool ReOpen();

	bool HandleEvents();
	void DrawInfoBar(int x, int y, int w, int h);
	void PrintStatus(int x, int y);

	//input and sound
	C_Control* m_pControl;
	C_Sound* m_pSound;
	void* m_pMusicChannel1;
	void* m_pMusicChannel2;
	double m_dMusicTimer;

	//graph data
	C_TileHandler* m_pTileHandler;
	C_GraphWrapper* m_pGraph;
	C_Image* m_pBKImg;
	C_Image* m_pBKImg2;
	C_Image* m_pInfoImg;
	C_Image* m_pCredits;
	C_Font* m_pFont;
	C_Font* m_pFont2;
	bool m_bInited;

	//mapdata
	C_Image* m_pMiniMap;
	C_World* m_pMap;
	int m_iScore;
	int m_iTime;
	int m_iDiamonds;

	C_Timer* m_pTimer;
	double m_dFadeOutTimer;

	C_GUI* m_pDlg0;
	char m_szPlayerName[MAXPLAYERNAMELENGTH];

	C_HighScoreList* m_pCurHighscore;

	int m_iUserLevelsListPos;
	int m_iLevelId;
	int m_iState;
};

C_Game* g_pGame = NULL;

////////////////////////////////////////////////////////////////////////////////
//game

#define MAXLEVELS 200
C_Game::C_Game()
{
	char szLevels[MAXLEVELS][24];
	char szPath[MAX_PATH];

	m_pGraph = NULL;
	m_pBKImg = NULL;
	m_pBKImg2 = NULL;
	m_pCredits = NULL;
	m_pFont = NULL;
	m_pFont2 = NULL;
	m_pMiniMap = NULL;
	m_pCurHighscore = NULL;

	m_dMusicTimer = 49000;
	m_pMusicChannel2 = m_pMusicChannel1 = NULL;
	m_pSound = new C_Sound();
	LoadSound();
	m_pControl = new C_Control();
	C_World::InitObjProperties(m_pSound, m_pControl);

	m_bInited = InitInstance();

	m_iState = 0;

	if (m_bInited) {
		bool bResult;
		sprintf(szPath, "%s/title.jpg", C_Global::szDataPath);
		m_pBKImg = new C_Image(szPath, NULL, &bResult);
		sprintf(szPath, "%s/infobar_frame.jpg", C_Global::szDataPath);
		m_pBKImg2 = new C_Image(szPath, NULL, &bResult);
		sprintf(szPath, "%s/infobar.tga", C_Global::szDataPath);
		m_pInfoImg = new C_Image(szPath, NULL);
		sprintf(szPath, "%s/credits.tga", C_Global::szDataPath);
		m_pCredits = new C_Image(szPath, NULL);
		m_bInited = bResult;
	}

	if (m_bInited) {
		char szPath[MAX_PATH];
		S_FColor stColor = { 0.6f, 0.6f, 0.6f, 1.0f };
		m_pFont = new C_Font(C_Image::GetDefaultFormat());
		S_FColor stColor2 = { 0.3f, 0.3f, 0.3f, 1.0f };
		m_pFont2 = new C_Font(C_Image::GetDefaultFormat());

		sprintf(szPath, "%s/arial-16", C_Global::szDataPath);
		m_bInited = m_pFont->Load(szPath, NULL);
		m_pFont->SetColor(stColor);
		sprintf(szPath, "%s/arial-34", C_Global::szDataPath);
		m_pFont2->Load(szPath, NULL);
		m_pFont2->SetColor(stColor2);
	}

	sprintf(szPath, "%s/%s", C_Global::szDataPath, "gui_desc0.txt");
	m_pDlg0 = new C_GUI();
	m_pDlg0->LoadTemplate(szPath, NULL, 100, 100);

	//find official levels
	int i = 0;
	C_Resource* pRes = new C_Resource();
	void* pSearchH = pRes->FileSearchOpen(C_Global::szDataPath, "*.map");
	char szFile[MAX_PATH];
	while (pRes->FileSearchNext(pSearchH, szFile, sizeof(szFile))) {
		char* pDot = strrchr(szFile, '.');
		if (pDot) *pDot = 0;
		if (strlen(szFile) < 24) strcpy(szLevels[i++], szFile);
		if (i >= MAXLEVELS) break;
	}
	pRes->FileSearchClose(pSearchH);
	delete pRes;

	//sorting of the files and put to the GUI list
	bool abMarked[MAXLEVELS];
	memset(abMarked, 0, sizeof(abMarked));
	char szTemp[128];
	int iPos = 0, iSortedPos = 0;
	int iNumLevels = i;
	while (iSortedPos != iNumLevels) {
		bool bFirstFound = false;
		for (i = 0; i < iNumLevels; i++)
		{
			if (!bFirstFound && !abMarked[i]) {
				strcpy(szTemp, szLevels[i]);
				bFirstFound = true;
				iPos = i;
			}
			else if (bFirstFound && !abMarked[i] && strcmp(szLevels[i], szTemp) < 0) {
				strcpy(szTemp, szLevels[i]); //save the lowest
				iPos = i;
			}
		}
		abMarked[iPos] = true;

		//save the lowest
		S_GUIListEl* pstEl = new S_GUIListEl();
		memset(pstEl, 0, sizeof(S_GUIListEl));
		pstEl->iID = iSortedPos++;
		pstEl->pIcon = 0;
		strcpy(pstEl->szText[0], szTemp);
		m_pDlg0->ListAddElement(2, pstEl);
		delete pstEl; //it is copied
	}

	//find unofficial levels and add
	m_iUserLevelsListPos = iSortedPos;
	pRes = new C_Resource();
	pSearchH = pRes->FileSearchOpen(C_Global::szAppDataPath, "*.map");
	i = 0;
	while (pRes->FileSearchNext(pSearchH, szFile, sizeof(szFile))) {
		char* pDot = strrchr(szFile, '.');
		if (pDot) *pDot = 0;

		S_GUIListEl* pstEl = new S_GUIListEl();
		memset(pstEl, 0, sizeof(S_GUIListEl));
		pstEl->iID = iSortedPos++;
		pstEl->pIcon = 0;
		strcpy(pstEl->szText[0], szFile);
		m_pDlg0->ListAddElement(2, pstEl);
		delete pstEl; //it is copied

		if (iSortedPos >= MAXLEVELS) break;
	}
	pRes->FileSearchClose(pSearchH);
	delete pRes;

	LoadLastName();
	m_pDlg0->SetText(4, m_szPlayerName);

	m_pMap = NULL;
	m_pTimer = new C_Timer();

	m_iTime = 0;
	m_iDiamonds = 0;
	m_iScore = 0;

	m_iLevelId = 0;
	m_pDlg0->SetFocused(2);
	m_pDlg0->ListSetSelectedElement(2, m_iLevelId);
	OpenSelectedHiscore();
	OpenSelectedMap();
}

C_Game::~C_Game()
{
	m_pDlg0->GetText(4, m_szPlayerName);
	SaveLastName();

	FreeAll();
}

void C_Game::LoadSound()
{
	char szFile[MAX_PATH];
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "bell2.wav");
	m_pSound->Load(szFile, NULL, SOUND_SHORTTIME);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "explosion.wav");
	m_pSound->Load(szFile, NULL, SOUND_EXPLOSION_FIRE);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "snap.wav");
	m_pSound->Load(szFile, NULL, SOUND_EXPLOSION_SNAP);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "squeese.wav");
	m_pSound->Load(szFile, NULL, SOUND_SQUEESE);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "cracknut.wav");
	m_pSound->Load(szFile, NULL, SOUND_CRACK);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "takediam.wav");
	m_pSound->Load(szFile, NULL, SOUND_TAKE_DIAM);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "takekey.wav");
	m_pSound->Load(szFile, NULL, SOUND_TAKE_KEY);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "door.wav");
	m_pSound->Load(szFile, NULL, SOUND_DOOR);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "push.wav");
	m_pSound->Load(szFile, NULL, SOUND_PUSH);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "hit_diam.wav");
	m_pSound->Load(szFile, NULL, SOUND_HIT_DIAM);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "hit_stone.wav");
	m_pSound->Load(szFile, NULL, SOUND_HIT_STONE);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "enemy_insect.wav");
	m_pSound->Load(szFile, NULL, SOUND_ENEMY_BLUE);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "enemy_grey.wav");
	m_pSound->Load(szFile, NULL, SOUND_ENEMY_ROBOT);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "enemy_jumping.wav");
	m_pSound->Load(szFile, NULL, SOUND_ENEMY_JUMPING);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "enemy_slime.wav");
	m_pSound->Load(szFile, NULL, SOUND_SLIME_GROW);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "arm_mine.wav");
	m_pSound->Load(szFile, NULL, SOUND_ARM_MINE);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "splash.wav");
	m_pSound->Load(szFile, NULL, SOUND_LAVA_SPLASH);
	sprintf(szFile, "%s/%s", C_Global::szDataPath, "enemy_eating.wav");
	m_pSound->Load(szFile, NULL, SOUND_ENEMY_EATING);

	sprintf(szFile, "%s/%s", C_Global::szDataPath, "title.ogg");
	m_pSound->Load(szFile, NULL, MUSIC_TITLE);

	m_pSound->Init();
}

void C_Game::LoadLastName()
{
	char szFile[MAX_PATH];
	m_szPlayerName[0] = 0;
	sprintf(szFile, "%s/%s", C_Global::szAppDataPath, "last_name.txt");
	FILE* pFile = fopen(szFile, "rt");
	if (pFile) {
		size_t iLen = fread(m_szPlayerName, 1, sizeof(m_szPlayerName) - 1, pFile);
		m_szPlayerName[iLen + 1] = 0;
		fclose(pFile);
	}
	if (m_szPlayerName[0] == 0) {
		strcpy(m_szPlayerName, "Incognito");
	}
}
void C_Game::SaveLastName()
{
	char szFile[MAX_PATH];
	sprintf(szFile, "%s/%s", C_Global::szAppDataPath, "last_name.txt");
	FILE* pFile = fopen(szFile, "wt");
	if (pFile) {
		size_t iLen = fwrite(m_szPlayerName, 1, strlen(m_szPlayerName), pFile);
		fclose(pFile);
	}
}

void C_Game::ResizeCallback()
{
	g_pGame->DrawScreen();
}

bool C_Game::InitInstance()
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

	return true;
}

void C_Game::FreeAll()
{
	delete m_pBKImg;   m_pBKImg = NULL;
	delete m_pBKImg2;  m_pBKImg2 = NULL;
	delete m_pInfoImg; m_pInfoImg = NULL;
	delete m_pCredits; m_pCredits = NULL;

	delete m_pTileHandler; m_pTileHandler = NULL;
	delete m_pMap;     m_pMap = NULL;
	delete m_pFont;    m_pFont = NULL;
	delete m_pFont2;   m_pFont2 = NULL;
	delete m_pGraph;   m_pGraph = NULL;
	delete m_pControl; m_pControl = NULL;
	delete m_pSound;   m_pSound = NULL;

	delete m_pDlg0;    m_pDlg0 = NULL;
	delete m_pMiniMap; m_pMiniMap = NULL;

	delete m_pTimer;   m_pTimer = NULL;
	delete m_pCurHighscore; m_pCurHighscore = NULL;
}

void C_Game::OpenSelectedHiscore(int iMarkedPlace)
{
	char szFile[MAX_PATH];

	int iEl = 0;
	m_pDlg0->ListGetSelectedElement(2, &iEl);
	S_GUIListEl* pEl = m_pDlg0->ListGetElementById(2, iEl);
	if (pEl) {
		sprintf(szFile, "%s/%s.hiscore", C_Global::szAppDataPath, pEl->szText[0]);
		delete m_pCurHighscore;
		m_pCurHighscore = new C_HighScoreList(szFile, true);
		S_HighScore stScoreList;
		m_pCurHighscore->GetAsData(&stScoreList);
		m_pDlg0->ListRemoveAll(1);
		for (int i = 0; i < stScoreList.iNumInList; i++) {
			S_GUIListEl* pstEl = new S_GUIListEl();
			memset(pstEl, 0, sizeof(S_GUIListEl));
			pstEl->iID = i;
			pstEl->pIcon = 0;
			strcpy(pstEl->szText[0], stScoreList.szName[i]);
			sprintf(pstEl->szText[1], "%d.%d", stScoreList.iScore[i] / 10, (abs(stScoreList.iScore[i]) % 10) ); //score 0.0
			sprintf(pstEl->szText[2], "%d.%02d s", stScoreList.iTime[i] / 1000, (abs(stScoreList.iTime[i]) % 1000) / 10); //display 0.00 s
			m_pDlg0->ListAddElement(1, pstEl);
			delete pstEl; //it is copied
		}
		//show a new score as selected, else nothing selected
		if (iMarkedPlace >= 0) {
			m_pDlg0->ListSetHasSelection(1, true);
			m_pDlg0->ListSetSelectedElement(1, iMarkedPlace);
		} else m_pDlg0->ListSetHasSelection(1, false);
	}
}

bool C_Game::OpenSelectedMap()
{
	delete m_pMap; m_pMap = NULL;
	delete m_pMiniMap; m_pMiniMap = NULL;

	char szFile[MAX_PATH];

	int iEl = 0;
	m_pDlg0->ListGetSelectedElement(2, &iEl);
	S_GUIListEl* pEl = m_pDlg0->ListGetElementById(2, iEl);
	if (pEl) {
		sprintf(szFile, "%s %s", PROGRAM_TITLE, pEl->szText[0]);
		m_pGraph->SetWindowTitle(szFile);
		if(iEl>= m_iUserLevelsListPos) sprintf(szFile, "%s/%s.map", C_Global::szAppDataPath, pEl->szText[0]);
		else sprintf(szFile, "%s/%s.map", C_Global::szDataPath, pEl->szText[0]);
		m_pMap = new C_World(szFile, m_pTileHandler);
		m_pMap->GetMapParameters(&m_iDiamonds, &m_iTime, &m_iScore);
		m_pDlg0->SetText(7, m_pMap->GetMapDescription());
		m_pTimer->Reset();

		m_pMiniMap = m_pMap->GetMiniMap(3);
	}

	return true;
}
bool C_Game::ReOpen()
{
	m_pMap->Load();
	m_pMap->GetMapParameters(&m_iDiamonds, &m_iTime, &m_iScore);

	return true;
}

void C_Game::SetSize(int w, int h)
{
	m_pGraph->SetSize(w, h);
	DrawScreen();
}

void C_Game::DrawInfoBar(int x, int y, int w, int h)
{
	int iImgW, iImgH;
	m_pBKImg2->GetInfo(&iImgW, &iImgH, NULL);

	S_Rect stBR[9];
	stBR[0] = S_RECTMAKE(0, 0, BORDER, BORDER);              //u left corner
	stBR[1] = S_RECTMAKE(iImgW - BORDER, 0, BORDER, BORDER); //u right corner
	stBR[2] = S_RECTMAKE(0, iImgH - BORDER, BORDER, BORDER); //l left corner
	stBR[3] = S_RECTMAKE(iImgW - BORDER, iImgH - BORDER, BORDER, BORDER); //l right corner
	stBR[4] = S_RECTMAKE(BORDER, 0, 2, BORDER);              //upper
	stBR[5] = S_RECTMAKE(BORDER, iImgH - BORDER, 2, BORDER); //lower
	stBR[6] = S_RECTMAKE(0, BORDER, BORDER, 2);              //left
	stBR[7] = S_RECTMAKE(iImgW - BORDER, BORDER, BORDER, 2); //right
	stBR[8] = S_RECTMAKE(BORDER, BORDER, 4, 4);              //middle

	//dst rects
	int w2 = w - BORDER;
	int h2 = h - BORDER;

	S_Rect dstRectW1 = S_RECTMAKE(x + BORDER, y, w - 2 * BORDER, BORDER);
	S_Rect dstRectW2 = S_RECTMAKE(x + BORDER, y + h - BORDER, w - 2 * BORDER, BORDER);
	S_Rect dstRectH1 = S_RECTMAKE(x, y + BORDER, BORDER, h - 2 * BORDER);
	S_Rect dstRectH2 = S_RECTMAKE(x + w - BORDER, y + BORDER, BORDER, h - 2 * BORDER);
	S_Rect dstRectM = S_RECTMAKE(x + BORDER, y + BORDER, w - 2 * BORDER, h - 2 * BORDER);

	//draw
	m_pGraph->Blt(m_pBKImg2, &stBR[0], x, y);
	m_pGraph->Blt(m_pBKImg2, &stBR[1], x + w2, y);
	m_pGraph->Blt(m_pBKImg2, &stBR[2], x, y + h2);
	m_pGraph->Blt(m_pBKImg2, &stBR[3], x + w2, y + h2);

	m_pGraph->Blt(m_pBKImg2, &stBR[4], &dstRectW1);
	m_pGraph->Blt(m_pBKImg2, &stBR[5], &dstRectW2);
	m_pGraph->Blt(m_pBKImg2, &stBR[6], &dstRectH1);
	m_pGraph->Blt(m_pBKImg2, &stBR[7], &dstRectH2);

	m_pGraph->Blt(m_pBKImg2, &stBR[8], &dstRectM);
}

void C_Game::PrintStatus(int x, int y)
{
	m_pGraph->Blt(m_pInfoImg, NULL, x, y, true);

	S_FColor stColor2 = { 0.4f, 0.4f, 0.4f, 1.0f };
	m_pFont2->SetColor(stColor2);
	m_pFont2->Print(x + 72, y + 12, "%d", m_iScore);
	if (m_iTime < 0) {
		stColor2.r = 0.7f; stColor2.g = 0.4f; stColor2.b = 0.4f; stColor2.a = 1.0f;
		m_pFont2->SetColor(stColor2);
	}
	if (m_iTime >= 0) m_pFont2->Print(x + 400, y + 12, "%d", m_iTime / 1000);
	else m_pFont2->Print(x + 400, y + 12, "%d", abs(m_iTime) / 1000 + 1);

	if (m_iDiamonds < 0) {
		stColor2.r = 0.4f; stColor2.g = 0.7f; stColor2.b = 0.4f; stColor2.a = 1.0f;
		m_pFont2->SetColor(stColor2);
	}
	else {
		stColor2.r = 0.4f; stColor2.g = 0.4f; stColor2.b = 0.4f; stColor2.a = 1.0f;
		m_pFont2->SetColor(stColor2);
	}
	m_pFont2->Print(x + 280, y + 12, "%d", abs(m_iDiamonds));

	m_pFont->Print(x + 500, y + 12, "'R' to Restart, 'K' to Kill player\nSHIFT+move to snap/lay mine", m_iScore);
}

void C_Game::DrawScreen()
{
	int iW, iH;
	int w = m_pGraph->GetModeWidth();
	int h = m_pGraph->GetModeHeight();

	//background
	S_FColor stBlack = S_FCOLORMAKE(0, 0, 0, 1.0f);
	m_pGraph->SetColor(&stBlack);
	m_pGraph->Rect(NULL, false);

	if (m_iState > 1) {
		m_pBKImg2->GetInfo(&iW, &iH, NULL);

		int iTileSize = m_pMap->GetTileSize();
		int iMapAreaW = w - (STARTXY + BORDER);
		int iMapAreaH = h - (STARTXY + iH + BORDER);

		//draw?
		if (iMapAreaH > (10 * iTileSize) && iMapAreaW > (iW)) {
			//center map
			int w2, h2;
			S_Rect stRect = { STARTXY, STARTXY + iH, iMapAreaW, iMapAreaH };
			m_pMap->GetMapSize(&w2, &h2);
			if (iMapAreaW > w2) {
				stRect.x = STARTXY + (iMapAreaW - w2) / 2;
				stRect.width = w2;
			}
			if (iMapAreaH > h2) {
				stRect.y = STARTXY + iH + (iMapAreaH - h2) / 2;
				stRect.height = h2;
			}
			m_pMap->SetScreenRect(&stRect);
			m_pMap->MoveRelative(0, 0);

			//text
			DrawInfoBar(STARTXY - BORDER, STARTXY - BORDER, w, iH);
			PrintStatus(0, 0);

			//map
			DrawInfoBar(stRect.x - BORDER, stRect.y - BORDER, stRect.width + 2 * BORDER, stRect.height + 2 * BORDER);
			m_pMap->Draw();
		}
	}

	if (m_iState == 0 || m_iState == 1) {
		m_pGraph->Blt(m_pBKImg, NULL, 0, 0);

		//dlg
		int w2, h2;
		int dlgX = 648; int dlgY = 412;
		int x = (w / 2) - (dlgX / 2);
		int y = STARTXY + SPACING + 134 + dlgY;
		if (y > h - SPACING) y -= dlgY + (y - (h - SPACING));
		else y -= dlgY;
		if (x >= STARTXY && y >= 0) {
			m_pDlg0->SetPos(x, y);
			m_pDlg0->Draw();
		}
		//credits
		m_pCredits->GetInfo(&w2, &h2, NULL);
		if (x >= STARTXY+48 && h >= y + dlgY + h2) {
			m_pGraph->Blt(m_pCredits, NULL, STARTXY, h - h2, true);
		}
		//mini map
		m_pMiniMap->GetInfo(&w2, &h2, NULL);
		x = (w / 2) - (w2 / 2);
		y = y + dlgY + SPACING;
		if (x >= 0 && y >= 0) {
			m_pGraph->Blt(m_pMiniMap, NULL, x, y);
		}

		//fade out
		if (m_iState == 1) {
			//draw black
			float fAlpha = (float)(m_dFadeOutTimer / 1000.0);
			if (fAlpha > 1.0f) fAlpha = 1.0f;
			S_FColor stCol = S_FCOLORMAKE(0, 0, 0, fAlpha);
			m_pGraph->SetColor(&stCol);
			m_pGraph->Rect(NULL, true);
		}
	}

	m_pGraph->Flip();
}

bool C_Game::HandleEvents() //returns true on exit
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
		case SDL_MOUSEWHEEL:
			if (m_pControl) m_pControl->PollInput(&e);
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
				SetSize(e.window.data1, e.window.data2);
			break;
		case SDL_QUIT:
			return true;
		}
	}
	return false;
}

bool C_Game::Update()
{
	if (HandleEvents()) return true;
	m_pControl->Update();
	double t = m_pTimer->GetInterval();

	//statemachine
	switch (m_iState)
	{
	case 0: //menu
		if (m_pDlg0) {
			int iLevelId;
			m_pDlg0->UpdateInput();
			m_pDlg0->ListGetSelectedElement(2, &iLevelId);
			if (m_iLevelId != iLevelId) {
				OpenSelectedHiscore();
				OpenSelectedMap();
				m_iLevelId = iLevelId;
			}
			S_GamepadState stPad;
			m_pControl->GetGamepadState(&stPad);
			if (m_pDlg0->WasClicked(3) || stPad.iButton & 0x1 || m_pControl->IsPressed(DIK_RETURN)) {
				OpenSelectedMap();
				m_pDlg0->GetText(4, m_szPlayerName);
				if (m_szPlayerName[0] == 0) strcpy(m_szPlayerName, "Incognito"); //do not allow 0 name length
				m_pMap->SetLevelRules(!m_pDlg0->IsChecked(5));
				m_iState++;
				m_dFadeOutTimer = 0.0;
			}
		}
		m_dMusicTimer += t;
		if (m_dMusicTimer > 48000) {
			m_pMusicChannel2 = m_pMusicChannel1;
			m_pMusicChannel1 = m_pSound->Play(MUSIC_TITLE, 0.85f, 0.0f, false, 0);
			m_dMusicTimer = 0;
		}
		break;
	case 1: //fade out
		m_dFadeOutTimer += t;
		//music vol -> 0
		{
			float fVol = 0.85f - (float)(m_dFadeOutTimer / 1000.0);
			m_pSound->SetVolume(m_pMusicChannel1, fVol);
			m_pSound->SetVolume(m_pMusicChannel2, fVol);
		}
		if (m_dFadeOutTimer > 1050) {
			m_iState++;
			m_pSound->Stop(m_pMusicChannel1);
			m_pSound->Stop(m_pMusicChannel2);
			m_pMusicChannel2 = m_pMusicChannel1 = NULL;
		}
		break;
	case 2: //game
		if (m_pControl->IsPressedOnce(DIK_R)) {
			ReOpen();
		}

		//0 game running, 1 game over, 2 level finnished
		int iStatus = m_pMap->Update(t);
		m_pMap->GetMapParameters(&m_iDiamonds, &m_iTime, &m_iScore);
		if (iStatus > 0) {
			if (iStatus == 2 && m_iTime >= 0) {
				int iPlace = m_pCurHighscore->Add(m_szPlayerName, m_iScore*10 + (m_iTime / 5) / 10, m_iTime); //saves file
				OpenSelectedHiscore(iPlace);
			}
			m_pDlg0->SetFocused(2);
			m_iState = 0;
			m_dMusicTimer = 49000;
		}
		break;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//os

int iLevel = 0;
int iNumLevels = 16;

//main
#ifdef WIN32
int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
	bool bShowDialog = false;
	char szOpenName[1024];
	szOpenName[0] = 0;

	C_Global::Init("gui_desc0.txt", "Boulder");

	g_pGame = new C_Game();
	if (!g_pGame->IsInited()) goto out;

	//main loop
	while (1) {
		if (g_pGame->Update()) break;
		g_pGame->DrawScreen();
		mssleep(1); //50 debug
	}
out:
	delete g_pGame;
#ifdef WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
#endif
	return 0;
}
