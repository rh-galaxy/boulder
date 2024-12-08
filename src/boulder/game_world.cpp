
#include "game_world.h"
#include "math.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

C_World::C_World(const char* szFilename, C_TileHandler* pTileHandler)
{
	m_pTileHandler = pTileHandler;
	m_iTileSize = m_pTileHandler->m_iTileSize;
	strcpy(m_szFilename, szFilename);
	m_pGrid = NULL;
	Load();
}

void C_World::InitObjProperties(C_Sound* pSound, C_Control* pControl)
{
	C_Object::InitObjPropertiesOnce(pSound, pControl);
}

bool C_World::Load()
{
	int i, j, iObjNum, iObjParam;

	FILE* pFile = fopen(m_szFilename, "rb");
	if (pFile == NULL) return false;

	Clear();

	//set state
	C_Object::ResetSoundTimeouts();
	m_dDebugSpeed = 1.0;
	m_bUseLevelTime = true;
	m_bAlive = true;
	m_iScore = 0;
	m_iNumMines = 0;
	m_bSnapUsed = false;
	m_iDoorKeys = 0;
	m_bFinished = false;
	m_iKeyState = 0;
	m_bPlayedShortTimeSound = false;
	m_dDelayEndLevel = 3500.0;

	m_iLastPlayerPosX = m_iLastPlayerPosY = 0;

	//read map conditions
	char szMagic[4];
	int32_t r;
	size_t iNum = fread(szMagic, 4, 1, pFile);
	if (iNum == 1 && szMagic[0] == 'B' && szMagic[1] == '2' && szMagic[2] == 'L' && szMagic[3] == 'V') {
		//new file format
		fread(m_szDescription, sizeof(m_szDescription), 1, pFile);
	}
	else {
		//old file format
		fseek(pFile, 0, SEEK_SET);
		strcpy(m_szDescription, "");
	}
	fread(&r, sizeof(r), 1, pFile);
	m_iDiamondsLeft = r;
	fread(&r, sizeof(r), 1, pFile);
	m_dLevelTime = r * 1000;
	//read dimensions
	fread(&r, sizeof(r), 1, pFile);
	m_iWidth = r;
	fread(&r, sizeof(r), 1, pFile);
	m_iHeight = r;

	//make grid of possible objects
	m_pGrid = new C_Object * *[m_iHeight];
	for (i = 0; i < m_iHeight; i++) {
		m_pGrid[i] = new C_Object * [m_iWidth];
		for (j = 0; j < m_iWidth; j++) {
			fread(&r, sizeof(r), 1, pFile);
			iObjNum = r;
			fread(&r, sizeof(r), 1, pFile);
			iObjParam = r;
			m_pGrid[i][j] = NULL; //init to NULL
			SetObject(j, i, iObjNum, iObjParam);
		}
	}
	fclose(pFile);

	return true;
}

void C_World::Clear()
{
	if (!m_pGrid) return;

	int i, j;
	//delete grid of objects
	for (i = 0; i < m_iHeight; i++) {
		for (j = 0; j < m_iWidth; j++) {
			delete m_pGrid[i][j];
		}
		delete[] m_pGrid[i];
	}
	delete[] m_pGrid;

	m_pGrid = NULL;
	m_iHeight = m_iWidth = 0;
}

void C_World::SetLevelRules(bool bUseLevelTime)
{
	m_bUseLevelTime = bUseLevelTime;
}

C_World::~C_World()
{
	Clear();
}

//set the draw area
void C_World::SetScreenRect(S_Rect* pstViewRect)
{
	m_stViewRect = *pstViewRect;
	m_iViewRectWidthSave = m_stViewRect.width;
	m_iViewRectHeightSave = m_stViewRect.height;
	FixLimits();
	MoveRelative(0, 0);
}

//moves the showpoint of the map relative to the last
// position and draws the result
void C_World::MoveRelative(int x, int y)
{
	MoveAbsolute(m_iViewPosX + x, m_iViewPosY + y);
}

//moves the showpoint of the map to a new position,
// given in world pixel coordinates
void C_World::MoveAbsolute(int x, int y)
{
	TestLimits(&x, &y, m_iScrollMaxX, m_iScrollMaxY);

	m_iViewPosX = x; m_iViewPosY = y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
C_Object* C_World::GetObject(int x, int y)
{
	bool bInRange = TestLimits(&x, &y, m_iWidth, m_iHeight);
	if (bInRange) return m_pGrid[y][x];
	else return NULL;
}

C_Object* C_World::GetFirstObject(int x, int y)
{
	C_Object* pObject = m_pGrid[y][x];
	C_Object* pPrevObject = pObject->m_pPrevObj;
	return (pPrevObject != NULL) ? pPrevObject : pObject;
}

C_Object* C_World::SetObject(int x, int y, C_Object* pNewObject)
{
	C_Object* pObject = m_pGrid[y][x];
	if (pObject) pObject->m_bToBeDeleted = true;
	m_pGrid[y][x] = pNewObject;
	pNewObject->SetWorldAndPos(this, x, y);
	return pObject;
}
C_Object* C_World::SetObject(int x, int y, int iObjNumber, int iObjParam)
{
	C_Object* pObject = new C_Object(m_pTileHandler, iObjNumber, iObjParam, 0);
	return SetObject(x, y, pObject);
}

C_Object* C_World::MoveObject(int x, int y, int x2, int y2, bool bDelayNextMove) //empty space is inserted at old pos, object at new pos is removed from grid and returned
{
	C_Object* pObject = m_pGrid[y2][x2];
	pObject->m_bToBeDeleted = true; //SetWorldAndPos(NULL, x2, y2); //does not belong to grid anymore
	m_pGrid[y2][x2] = m_pGrid[y][x];
	m_pGrid[y2][x2]->SetWorldAndPos(this, x2, y2);
	m_pGrid[y][x] = new C_Object(m_pTileHandler, OBJ_EMPTY, 0, 0);
	m_pGrid[y][x]->SetWorldAndPos(this, x, y);

	if (bDelayNextMove) m_pGrid[y][x]->m_dMoveDelayTime = 40.0;

	return pObject;
}

C_Object* C_World::ReplaceObject(int x, int y, C_Object* pNewObject)
{
	C_Object* pObject = m_pGrid[y][x];
	pObject->m_bToBeDeleted = true; //SetWorldAndPos(NULL, x, y); //does not belong to grid anymore
	m_pGrid[y][x] = pNewObject;
	pNewObject->SetWorldAndPos(this, x, y);
	return pObject;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

int C_World::Update(double dTime)
{
	int i, j;
	bool bAnimFinnished, bPlayerExist = false;
	C_Object* oObject = NULL;
	C_Object::UpdateSoundTimeouts(dTime);

	//debug - variable gamespeed hack
	if (C_Object::s_pControl->IsPressedOnce(DIK_SUBTRACT)) m_dDebugSpeed *= 0.8;
	if (C_Object::s_pControl->IsPressedOnce(DIK_ADD)) m_dDebugSpeed *= 1.25;
	if (m_dDebugSpeed <= 0) m_dDebugSpeed = 0.01;
	dTime *= m_dDebugSpeed;

	//pass 1:
	//update all objects animations to get them to the same timestate
	for (i = 0; i < m_iHeight; i++) {
		for (j = 0; j < m_iWidth; j++) {
			oObject = m_pGrid[i][j];
			bAnimFinnished = oObject->UpdateAnimation(dTime);
		}
	}
	//pass 2:
	//update all objects but enemies
	for (i = 0; i < m_iHeight; i++) {
		for (j = 0; j < m_iWidth; j++) {
			oObject = m_pGrid[i][j];
			if ((oObject->m_iObjProperties & TYPE_ENEMY) == 0) oObject->Update(dTime);
			if (oObject->m_iObjNumber == OBJ_PLAYER) {
				m_iLastPlayerPosX = j;
				m_iLastPlayerPosY = i;
				bPlayerExist = true;
			}
		}
	}
	//pass 3:
	//update all enemies
	for (i = 0; i < m_iHeight; i++) {
		for (j = 0; j < m_iWidth; j++) {
			oObject = m_pGrid[i][j];
			if ((oObject->m_iObjProperties & TYPE_ENEMY) != 0) oObject->Update(dTime);
		}
	}

	//play 'collected' sounds
	C_Object::PlayNearestSounds();

	//update and check end level conditions
	if (!m_bFinished) m_dLevelTime -= dTime;
	if (m_dLevelTime <= 0 && m_bUseLevelTime) {
		m_dLevelTime = 0;
		m_bAlive = false; //kill player
	}
	{ //play "short time" sound at time 15
		int iLevelTime = (int)(m_dLevelTime * 0.001);
		if (iLevelTime <= 15 && !m_bPlayedShortTimeSound) { //once only
			m_bPlayedShortTimeSound = true;
			C_Object::s_pSound->Play(SOUND_SHORTTIME);
		}
	}
	int iResult = 0;
	if (!bPlayerExist || m_bFinished) {
		m_dDelayEndLevel -= dTime;
		if (m_dDelayEndLevel <= 0) { //exit level
			iResult = bPlayerExist ? 0 : 1;
			iResult = m_bFinished ? 2 : iResult;
		}
	}
	return iResult; //0 game running, 1 game over, 2 level finnished
}

// checks if the values goes outside the limits, and take actions
bool C_World::TestLimits(int* o_iX, int* o_iY, int w, int h)
{
	bool bResult = true;

	if (*o_iX >= w) {
		*o_iX = w;
		bResult = false;
	}
	if (*o_iY >= h) {
		*o_iY = h;
		bResult = false;
	}
	if (*o_iX < 0) {
		*o_iX = 0;
		bResult = false;
	}
	if (*o_iY < 0) {
		*o_iY = 0;
		bResult = false;
	}
	return bResult;
}

void C_World::FixLimits()
{
	m_stViewRect.width = m_iViewRectWidthSave;
	m_stViewRect.height = m_iViewRectHeightSave;
	// if the show area is larger than the map, we need to limit the showarea
	if (m_iWidth * m_iTileSize < m_iViewRectWidthSave) m_stViewRect.width = m_iWidth * m_iTileSize;
	if (m_iHeight * m_iTileSize < m_iViewRectHeightSave) m_stViewRect.height = m_iHeight * m_iTileSize;

	// calc maximum upper left coordinate we can scroll to, without getting outside the map
	m_iPixMaxX = m_stViewRect.x + m_stViewRect.width;
	m_iPixMaxY = m_stViewRect.y + m_stViewRect.height;
	m_iScrollMaxX = m_iWidth * m_iTileSize - m_stViewRect.width;
	m_iScrollMaxY = m_iHeight * m_iTileSize - m_stViewRect.height;
	if (m_iScrollMaxX < 0) m_iScrollMaxX = 0;
	if (m_iScrollMaxY < 0) m_iScrollMaxY = 0;
}

C_Image* C_World::GetMiniMap(int iTileSize)
{
	C_Image* oResult = new C_Image(m_iWidth * iTileSize, m_iHeight * iTileSize, _RGBA8888);

	S_Color stColorWhite = { 255, 255, 255, 255 };
	S_Color stColorBlack = { 0, 0, 0, 255 };
	S_Color stColorGrey = { 158, 158, 158, 255 };
	S_Color stColorDarkGrey = { 90, 90, 90, 255 };
	S_Color stColorDiamondBlue = { 152, 186, 230, 255 };
	//S_Color stColorDiamondRed = { 248, 118, 140, 255 };
	S_Color stColorEnemyBlue = { 0, 25, 200, 255 };
	S_Color stColorEnemyGreen = { 0, 200, 25, 255 };
	//S_Color stColorEarth = { 100, 60, 12, 255 };
	S_Color stColorEarth = { 11, 60, 70, 255 };
	S_Color stColorBomb = { 253, 62, 17, 255 };
	S_Color stColorLava = { 193, 30, 0, 255 };
	S_Rect stRectMapArea = { 0, 0, m_iWidth * iTileSize, m_iHeight * iTileSize };

	oResult->FillWithRGBColor(&stRectMapArea, &stColorBlack);

	for (int i = 0; i < m_iHeight; i++) {
		for (int j = 0; j < m_iWidth; j++) {
			S_Rect stRectTileArea = { j * iTileSize, i * iTileSize, iTileSize, iTileSize };
			C_Object* oObject = m_pGrid[i][j];
			switch (oObject->m_iObjNumber) {
			case OBJ_MINE:
			case OBJ_EMPTY: break; //no draw
			case OBJ_OBJECT_MACHINE:
			case OBJ_DOOR:
			case OBJ_DOOR_GREY:
			case OBJ_WALL_D:
			case OBJ_WALL_DS:
			case OBJ_GREEN_WALL_LEFT:
			case OBJ_GREEN_WALL_RIGHT:
			case OBJ_GREEN_SLIP_LEFT:
			case OBJ_GREEN_SLIP_RIGHT:
			case OBJ_GREEN_BOTTOM:
			case OBJ_WALL: oResult->FillWithRGBColor(&stRectTileArea, &stColorGrey); break;
			case OBJ_BOMB: oResult->FillWithRGBColor(&stRectTileArea, &stColorBomb); break;
			case OBJ_MUD:
			case OBJ_EARTH: oResult->FillWithRGBColor(&stRectTileArea, &stColorEarth); break;
			case OBJ_NUT:
			case OBJ_KEY:
			case OBJ_DIAM_RED:
			case OBJ_DIAM_MAKER:
			case OBJ_DIAM_BLUE: oResult->FillWithRGBColor(&stRectTileArea, &stColorDiamondBlue); break;
			case OBJ_STONE: oResult->FillWithRGBColor(&stRectTileArea, &stColorDarkGrey); break;
			case OBJ_LAVA: oResult->FillWithRGBColor(&stRectTileArea, &stColorLava); break;
			case OBJ_PLAYER:
			case OBJ_EXIT_CLOSED: oResult->FillWithRGBColor(&stRectTileArea, &stColorWhite); break;
			case OBJ_ENEMY0:
			case OBJ_ENEMY1:
			case OBJ_ENEMY2:
			case OBJ_ENEMY3: oResult->FillWithRGBColor(&stRectTileArea, &stColorEnemyBlue); break;
			case OBJ_SLIME:
			case OBJ_ENEMY4: oResult->FillWithRGBColor(&stRectTileArea, &stColorEnemyGreen); break;
			}
		}
	}

	return oResult;
}

void C_World::Draw()
{
	int iTempX, iTempY;
	int iBaseTileX, iBaseOffsetX;
	int iTileX, iTileY;
	int iOffsetX, iOffsetY;
	int iScreenX = m_stViewRect.x, iScreenY = m_stViewRect.y;
	S_Rect stRect = S_RECTMAKE(0, 0, 0, 0);
	int iNumInList = 0;
	C_Object* aObjList[1024];
	C_Object* pObject, * pPrevObject;
	C_Object* pPlayer = NULL;

	//clear to black to be able to draw with transparency
	S_FColor stC = S_FCOLORMAKE(0.0f, 0.0f, 0.0f, 1.0f);
	m_pTileHandler->m_pGraph->SetColor(&stC);
	m_pTileHandler->m_pGraph->Rect(&m_stViewRect);

	iBaseTileX = iTileX = m_iViewPosX / m_iTileSize;
	iBaseOffsetX = iOffsetX = m_iViewPosX % m_iTileSize;
	iTileY = m_iViewPosY / m_iTileSize;
	iOffsetY = m_iViewPosY % m_iTileSize;

	//pass1, handle offset objects
	int i, j;
	int iStartX = iBaseTileX - 1;
	int iStartY = iTileY - 1;
	int iEndX = (m_stViewRect.width / m_iTileSize + 1 + 1 + 2) + iStartX;
	int iEndY = (m_stViewRect.height / m_iTileSize + 1 + 1 + 2) + iStartY;
	if (iStartX < 0) iStartX = 0; if (iStartY < 0) iStartY = 0;
	if (iEndX > m_iWidth) iEndX = m_iWidth; if (iEndY > m_iHeight) iEndY = m_iHeight;
	for (j = iStartY; j < iEndY; j++) {
		for (i = iStartX; i < iEndX; i++) {
			pObject = m_pGrid[j][i]; //must not be NULL!
			while (pObject != NULL) { //will be max 2 iterations
				if ((pObject->m_stAnimOffset.x != 0 || pObject->m_stAnimOffset.y != 0) && pObject->m_iObjNumber != OBJ_EMPTY) {
					if (pObject->m_bPreDraw) {
						S_Point stPos = pObject->GetPos();
						stPos.x = ((stPos.x * m_iTileSize) - m_iViewPosX) + m_stViewRect.x;
						stPos.y = ((stPos.y * m_iTileSize) - m_iViewPosY) + m_stViewRect.y;
						pObject->DrawCliped(stPos.x, stPos.y, &m_stViewRect);
					}
					else {
						if (pObject->m_iObjNumber == OBJ_PLAYER) pPlayer = pObject;
						else {
							aObjList[iNumInList] = pObject;
							iNumInList++;
						}
					}
				}
				pObject = pObject->m_pPrevObj;
			}
		}
	}

	//pass 2, rest of map
	do {
		do {
			pObject = m_pGrid[iTileY][iTileX]; //must not be NULL!
			pPrevObject = pObject->m_pPrevObj;
			if (pPrevObject != NULL) {
				//put to list to draw later
				if (pObject->m_iObjNumber != OBJ_EMPTY) {
					if (pObject->m_iObjNumber == OBJ_PLAYER) pPlayer = pObject;
					else {
						aObjList[iNumInList] = pObject;
						iNumInList++;
					}
				}
				pObject = pPrevObject;
			}
			iTempX = m_iPixMaxX - iScreenX;
			iTempY = m_iPixMaxY - iScreenY;
			stRect.width = (m_iTileSize - iOffsetX) < iTempX ? (m_iTileSize - iOffsetX) : iTempX;
			stRect.height = (m_iTileSize - iOffsetY) < iTempY ? (m_iTileSize - iOffsetY) : iTempY;
			if (pObject->m_stAnimOffset.x == 0 && pObject->m_stAnimOffset.y == 0 && pObject->m_iObjNumber != OBJ_EMPTY) {
				stRect.x = iOffsetX;
				stRect.y = iOffsetY;
				pObject->Draw(iScreenX, iScreenY, &stRect);
			}

			iTileX++;
			iScreenX += stRect.width;
			iOffsetX = 0;
		} while (iScreenX < m_iPixMaxX);
		iOffsetX = iBaseOffsetX;
		iOffsetY = 0;
		iTileX = iBaseTileX;
		iTileY++;

		iScreenX = m_stViewRect.x;
		iScreenY += stRect.height;
	} while (iScreenY < m_iPixMaxY);

	//draw player here, special case since it is showing blackness around while moving (used for digging)
	if (pPlayer && !pPlayer->m_bPreDraw) {
		S_Point stPos = pPlayer->GetPos();
		stPos.x = ((stPos.x * m_iTileSize) - m_iViewPosX) + m_stViewRect.x;
		stPos.y = ((stPos.y * m_iTileSize) - m_iViewPosY) + m_stViewRect.y;
		pPlayer->DrawCliped(stPos.x, stPos.y, &m_stViewRect);
	}

	//draw objects in list
	for (i = 0; i < iNumInList; i++) {
		pObject = aObjList[i];
		S_Point stPos = pObject->GetPos();
		stPos.x = ((stPos.x * m_iTileSize) - m_iViewPosX) + m_stViewRect.x;
		stPos.y = ((stPos.y * m_iTileSize) - m_iViewPosY) + m_stViewRect.y;
		pObject->DrawCliped(stPos.x, stPos.y, &m_stViewRect);
	}
}

///////////////////////////////////////////////////////////////	
//NEW DOACTION FUNCTIONS
///////////////////////////////////////////////////////////////	
void C_World::ActionMove(int x, int y, int x2, int y2, int iAnim, int iAnimForPrevObject, bool bDelayNextMove)
{
	C_Object* oObj, * oPrevObj;

	oObj = m_pGrid[y][x];
	//delete any existing prevobject
	delete oObj->m_pPrevObj;
	oObj->m_pPrevObj = NULL;

	oPrevObj = MoveObject(x, y, x2, y2, bDelayNextMove);
	oObj->SetAnim(iAnim);
	oObj->m_bMoving = true;
	if (iAnimForPrevObject != -1) {
		oObj->m_pPrevObj = oPrevObj;
		if (iAnimForPrevObject != ANIM_CURRENTANIMNORESET) oPrevObj->SetAnim(iAnimForPrevObject);
	} else {
		delete oPrevObj;
	}
}

void C_World::ActionReplace(int x, int y, int iNewObjNum, int iNewAnim, int iAnimForPrevObject)
{
	C_Object* oObj, * oPrevObj;

	int iNextParam = 0;
	if (iNewObjNum == OBJ_EXPLOSION) iNextParam = OBJ_EMPTY; //must specify nextobject to explosion
	else if (iNewObjNum == OBJ_SLIME) iNextParam = 1;
	else if (iNewObjNum == OBJ_MINE) iNextParam = 1;
	oObj = new C_Object(m_pTileHandler, iNewObjNum, iNextParam, 0);
	oPrevObj = ReplaceObject(x, y, oObj);
	oObj->SetAnim(iNewAnim);
	//oObj->m_bMoving = true; //not actually moving...
	if (iAnimForPrevObject != -1) {
		oObj->m_pPrevObj = oPrevObj;
		if (iAnimForPrevObject != ANIM_CURRENTANIMNORESET) oPrevObj->SetAnim(iAnimForPrevObject);
	} else {
		delete oPrevObj;
	}
}

void C_World::ActionMoveAndDestroyBoth(int x, int y, int x2, int y2, int iAnim, int iAnimForPrevObject, int iExplosionAnim)
{
	C_Object* oObj, * oPrevObj;

	oObj = GetObject(x, y);
	oPrevObj = MoveObject(x, y, x2, y2);
	oObj->SetAnim(iAnim);
	oObj->m_bMoving = true;
	oObj->m_pNextObj = new C_Object(m_pTileHandler, OBJ_EXPLOSION, OBJ_EMPTY, 0);
	oObj->m_pNextObj->m_iDefaultAnim = iExplosionAnim;
	if (iAnimForPrevObject != -1) {
		oObj->m_pPrevObj = oPrevObj; //replace any existing prevobject
		if (iAnimForPrevObject != ANIM_CURRENTANIMNORESET) oPrevObj->SetAnim(iAnimForPrevObject);
	} else {
		delete oPrevObj;
	}

	C_Object::PlaySoundVolPan(iExplosionAnim == ANIM_EXPLOSION_SNAP ? SOUND_EXPLOSION_SNAP : SOUND_EXPLOSION_FIRE, x2, y2);
}

void C_World::ActionExplode3x3(int x, int y)
{
	C_Object* oObj;

	int iNumEnemies = 0;
	int iEnemiesX[9];
	int iEnemiesY[9];
	int iEnemiesParam[9];

	int iAreaX, iAreaY;
	for (iAreaY = y - 1; iAreaY <= y + 1; iAreaY++) {
		for (iAreaX = x - 1; iAreaX <= x + 1; iAreaX++) {
			oObj = GetObject(iAreaX, iAreaY);
			if ((oObj->m_iObjProperties & TYPE_DESTRUCTABLE) != 0) {
				delete oObj->m_pPrevObj;
				oObj->m_pPrevObj = NULL; //delete any existing prevobject

				if ((oObj->m_iObjProperties & TYPE_ENEMY) != 0) {
					iEnemiesX[iNumEnemies] = iAreaX;
					iEnemiesY[iNumEnemies] = iAreaY;
					iEnemiesParam[iNumEnemies] = oObj->m_iObjParam;
					iNumEnemies++;
					//continue;
				}
				if (oObj->m_iObjNumber != OBJ_EXPLOSION) {
					C_Object* oObjExplosion = new C_Object(m_pTileHandler, OBJ_EXPLOSION, OBJ_EMPTY, 0);
					//delete oObjExplosion->m_pNextObj; //hack since we want to set next obj here
					if (oObj->m_pNextObj != NULL) {
						oObjExplosion->m_pNextObj = oObj->m_pNextObj;
					}
					else if (oObj->m_iObjNumber == OBJ_BOMB && (iAreaX != x || iAreaY != y)) {
						oObjExplosion->m_pNextObj = new C_Object(m_pTileHandler, OBJ_BOMB, 0, 0);
					}
					SetObject(iAreaX, iAreaY, oObjExplosion);
				}
				else {
					oObj->m_bAnimFinished = false;
					oObj->m_dTimeIntoAnim = 0;
				}
			}
		}
	}
	for (int i = 0; i < iNumEnemies; i++) {
		ActionExplodeInto(iEnemiesX[i], iEnemiesY[i], iEnemiesParam[i], ANIM_EXPLOSION_SHORT);
	}

	C_Object::PlaySoundVolPan(SOUND_EXPLOSION_FIRE, x, y);
}

void C_World::ActionExplodeInto(int x, int y, int iExplodeInto, int iExplosionAnim)
{
	C_Object* oObj;

	int iExt = 1;
	if (iExplodeInto == 0) iExt = 0;

	int iAreaX, iAreaY, iObjIndex = 0;
	for (iAreaY = y - iExt; iAreaY <= y + iExt; iAreaY++) {
		for (iAreaX = x - iExt; iAreaX <= x + iExt; iAreaX++) {
			oObj = GetObject(iAreaX, iAreaY);
			if ((oObj->m_iObjProperties & TYPE_DESTRUCTABLE) != 0) {
				int iNewObjNum = ENEMY_EXPLODE_INTO[iExplodeInto][iObjIndex];
				int iNewObjParam = ENEMY_EXPLODE_INTO_PARAM[iExplodeInto][iObjIndex];
				delete oObj->m_pPrevObj;
				oObj->m_pPrevObj = NULL; //delete any existing prevobject

				iObjIndex++;

				if (oObj->m_iObjNumber == OBJ_BOMB) {
					ActionExplode3x3(iAreaX, iAreaY);
					continue;
				}
				if (oObj->m_iObjNumber == OBJ_EXPLOSION) {
					continue;
				}
				//not continous explosions
				//all objects are destroyed and replaced within the area, including enemies
				C_Object* oObjExplosion = new C_Object(m_pTileHandler, OBJ_EXPLOSION, iNewObjNum, iNewObjParam);
				oObjExplosion->m_pNextObj->m_iDefaultAnim = iExplosionAnim;
				SetObject(iAreaX, iAreaY, oObjExplosion);
			}
			else iObjIndex++;
		}
	}

	C_Object::PlaySoundVolPan(SOUND_EXPLOSION_FIRE, x, y);
}

///////////////////////////////////////////////////////////////	
//NEW DOACTION FUNCTIONS
///////////////////////////////////////////////////////////////	
