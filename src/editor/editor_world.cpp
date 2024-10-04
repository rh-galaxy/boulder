
#include "editor_world.h"
#include "math.h"

C_World::C_World(const char* szFilename, int iWidth, int iHeight, C_TileHandler* pTileHandler)
{
	int i, j;

	m_pTileHandler = pTileHandler;
	m_iTileSize = m_pTileHandler->m_iTileSize;
	strcpy(m_szFilename, szFilename);
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	//make grid of possible objects
	m_pGrid = new C_Object * *[m_iHeight];
	for (i = 0; i < m_iHeight; i++) {
		m_pGrid[i] = new C_Object * [m_iWidth];
		for (j = 0; j < m_iWidth; j++)
			m_pGrid[i][j] = NULL; //initialize to NULL
	}
	//make indestructable borders
	for (i = 0; i < m_iWidth; i++) {
		SetObject(i, 0, OBJ_WALL);
		SetObject(i, m_iHeight - 1, OBJ_WALL);
	}
	for (i = 0; i < m_iHeight; i++) {
		SetObject(0, i, OBJ_WALL);
		SetObject(m_iWidth - 1, i, OBJ_WALL);
	}
	//make interior
	for (i = 1; i < m_iHeight - 1; i++) {
		for (j = 1; j < m_iWidth - 1; j++) SetObject(j, i, OBJ_EARTH);
	}

	m_iViewPosY = m_iViewPosX = 0;
}

C_World::C_World(const char* szFilename, C_TileHandler* pTileHandler)
{
	m_pTileHandler = pTileHandler;
	m_iTileSize = m_pTileHandler->m_iTileSize;
	strcpy(m_szFilename, szFilename);
	m_pGrid = NULL;
	Load();

	m_iViewPosY = m_iViewPosX = 0;
}

bool C_World::Load()
{
	int i, j, iObjNum, iObjParam;

	FILE* pFile = fopen(m_szFilename, "rb");
	if (pFile == NULL) return false;

	Clear();

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
		strcpy(m_szDescription, "Description");
	}
	fread(&r, sizeof(r), 1, pFile);
	m_iDiamondsLeft = r;
	fread(&r, sizeof(r), 1, pFile);
	m_iTimeLimit = r;
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

bool C_World::Save()
{
	int i, j, iDefault = OBJ_EMPTY;
	FILE* pFile = fopen(m_szFilename, "wb");
	if (pFile == NULL) return false;

	//only save in new file format
	char szMagic[4] = { 'B', '2', 'L', 'V' };
	fwrite(szMagic, 4, 1, pFile);
	fwrite(m_szDescription, sizeof(m_szDescription), 1, pFile);

	//save map conditions
	int32_t r;
	r = m_iDiamondsLeft;
	fwrite(&r, sizeof(r), 1, pFile);
	r = m_iTimeLimit;
	fwrite(&r, sizeof(r), 1, pFile);
	//save dimensions
	r = m_iWidth;
	fwrite(&r, sizeof(r), 1, pFile);
	r = m_iHeight;
	fwrite(&r, sizeof(r), 1, pFile);

	//save grid of objects
	for (i = 0; i < m_iHeight; i++) {
		for (j = 0; j < m_iWidth; j++) {
			C_Object* pObj = m_pGrid[i][j];
			if (pObj != NULL) {
				r = pObj->m_iObjNumber;
				fwrite(&r, sizeof(r), 1, pFile);
				r = pObj->m_iObjParam;
				fwrite(&r, sizeof(r), 1, pFile);
			}
			else { //error
				r = iDefault;
				fwrite(&r, sizeof(r), 1, pFile);
				r = 0;
				fwrite(&r, sizeof(r), 1, pFile);
			}
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

C_World::~C_World()
{
	Clear();
}

//crops one tile from the given side
//0=left, 1=right, 2=top, 3=bottom
void C_World::Crop(int iSide)
{
	int i, j, w, h;

	bool bFail = false;
	int xmod = 0;
	int ymod = 0;
	w = m_iWidth;
	h = m_iHeight;
	switch (iSide) {
	case 0:
	case 1: if (w <= 2) bFail = true; break;
	case 2:
	case 3: if (h <= 2) bFail = true; break;
	}
	if (bFail) return;

	switch (iSide) {
	case 0: xmod = 1; w--; for (i = 0; i < m_iHeight; i++) delete m_pGrid[i][0]; break;
	case 1: xmod = 0; w--; for (i = 0; i < m_iHeight; i++) delete m_pGrid[i][w]; break;
	case 2: ymod = 1; h--; for (i = 0; i < m_iWidth; i++) delete m_pGrid[0][i]; break;
	case 3: ymod = 0; h--; for (i = 0; i < m_iWidth; i++) delete m_pGrid[h][i]; break;
	}

	//create new grid and copy
	C_Object*** pGrid = new C_Object * *[h];
	for (i = 0; i < h; i++) {
		pGrid[i] = new C_Object * [w];
		for (j = 0; j < w; j++) {
			pGrid[i][j] = m_pGrid[i + ymod][j + xmod];
		}
	}

	//replace grid
	for (j = 0; j < m_iHeight; j++) {
		delete[] m_pGrid[j];
	}
	delete[] m_pGrid;
	m_pGrid = pGrid;
	m_iHeight = h;
	m_iWidth = w;
	MoveRelative(0, 0);
}

//extend one tile from the given side
//0=left, 1=right, 2=top, 3=bottom
void C_World::Extend(int iSide)
{
	int i, j, w, h;

	int xmod = 0;
	int ymod = 0;
	w = m_iWidth;
	h = m_iHeight;
	switch (iSide) {
	case 0: xmod = 1; w++; break;
	case 1: xmod = 0; w++; break;
	case 2: ymod = 1; h++; break;
	case 3: ymod = 0; h++; break;
	}

	//create new grid
	C_Object*** pGrid = new C_Object * *[h];
	for (i = 0; i < h; i++) {
		pGrid[i] = new C_Object * [w];
		for (j = 0; j < w; j++) {
			pGrid[i][j] = NULL;
		}
	}
	//copy
	for (i = 0; i < m_iHeight; i++) {
		for (j = 0; j < m_iWidth; j++) {
			pGrid[i + ymod][j + xmod] = m_pGrid[i][j];
		}
	}

	//replace grid
	for (j = 0; j < m_iHeight; j++) {
		delete[] m_pGrid[j];
	}
	delete[] m_pGrid;
	m_pGrid = pGrid;
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			if (m_pGrid[i][j] == NULL) SetObject(j, i, 0, 0);
		}
	}

	m_iHeight = h;
	m_iWidth = w;
	FixLimits();
	MoveRelative(0, 0);
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

//only useful in editor, because the pixelvalues are in screen coordinates, not world coordinates
//sets a mapelement to the given value
void C_World::SetElement(int iScrPixelX, int iScrPixelY, int iTileNum, int iObjParam)
{
	int iRealX = (m_iViewPosX + (iScrPixelX - m_stViewRect.x)) / m_iTileSize;
	int iRealY = (m_iViewPosY + (iScrPixelY - m_stViewRect.y)) / m_iTileSize;

	if (!TestLimits(&iRealX, &iRealY, m_iWidth, m_iHeight)) return;

	SetObject(iRealX, iRealY, iTileNum, iObjParam);
}

void C_World::GetElement(int iScrPixelX, int iScrPixelY, int* o_iTileNum, int* o_iObjParam)
{
	int iRealX = (m_iViewPosX + (iScrPixelX - m_stViewRect.x)) / m_iTileSize;
	int iRealY = (m_iViewPosY + (iScrPixelY - m_stViewRect.y)) / m_iTileSize;

	if (!TestLimits(&iRealX, &iRealY, m_iWidth, m_iHeight)) return;

	C_Object* pObj = GetObject(iRealX, iRealY);
	if (o_iTileNum) *o_iTileNum = pObj->m_iObjNumber;
	if (o_iObjParam) *o_iObjParam = pObj->m_iObjParam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
C_Object* C_World::GetObject(int x, int y)
{
	bool bInRange = TestLimits(&x, &y, m_iWidth, m_iHeight);
	if (bInRange) return m_pGrid[y][x];
	else return NULL;
}

void C_World::SetObject(int x, int y, C_Object* pNewObject)
{
	delete m_pGrid[y][x];
	m_pGrid[y][x] = pNewObject;
	pNewObject->SetWorldAndPos(this, x, y);
}
void C_World::SetObject(int x, int y, int iObjNumber, int iObjParam)
{
	C_Object* pObject = new C_Object(m_pTileHandler, iObjNumber, iObjParam);
	SetObject(x, y, pObject);
}

C_Object* C_World::MoveObject(int x, int y, int x2, int y2) //empty space is inserted at old pos, object at new pos is removed from grid and returned
{
	C_Object* pObject = m_pGrid[y2][x2];
	pObject->SetWorldAndPos(NULL, x2, y2); //does not belong to grid anymore
	m_pGrid[y2][x2] = m_pGrid[y][x];
	m_pGrid[y2][x2]->SetWorldAndPos(this, x2, y2);
	m_pGrid[y][x] = new C_Object(m_pTileHandler, OBJ_EMPTY, 0);
	m_pGrid[y][x]->SetWorldAndPos(this, x, y);
	return pObject;
}

C_Object* C_World::ReplaceObject(int x, int y, C_Object* pNewObject)
{
	C_Object* pObject = m_pGrid[y][x];
	pObject->SetWorldAndPos(NULL, x, y); //does not belong to grid anymore
	m_pGrid[y][x] = pNewObject;
	pNewObject->SetWorldAndPos(this, x, y);
	return pObject;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void C_World::GetViewPos(int* o_iWorldX, int* o_iWorldY)
{
	if (o_iWorldX) *o_iWorldX = m_iViewPosX;
	if (o_iWorldY) *o_iWorldY = m_iViewPosY;
}

void C_World::Draw()
{
	int iTempX, iTempY;
	int iBaseTileX, iBaseOffsetX;
	int iTileX, iTileY;
	int iOffsetX, iOffsetY;
	int iScreenX = m_stViewRect.x, iScreenY = m_stViewRect.y;
	S_Rect stRect;

	//fill with black first
	S_FColor stC = S_FCOLORMAKE(0.0f, 0.0f, 0.0f, 1.0f);
	m_pTileHandler->m_pGraph->SetColor(&stC);
	m_pTileHandler->m_pGraph->Rect(&m_stViewRect);

	iBaseTileX = iTileX = m_iViewPosX / m_iTileSize;
	iBaseOffsetX = iOffsetX = m_iViewPosX % m_iTileSize;
	iTileY = m_iViewPosY / m_iTileSize;
	iOffsetY = m_iViewPosY % m_iTileSize;
	do {
		do {
			C_Object* pObject = m_pGrid[iTileY][iTileX]; //must not be NULL!
			stRect.x = iOffsetX;
			stRect.y = iOffsetY;
			iTempX = m_iPixMaxX - iScreenX;
			iTempY = m_iPixMaxY - iScreenY;
			stRect.width = (m_iTileSize - iOffsetX) < iTempX ? (m_iTileSize - iOffsetX) : iTempX;
			stRect.height = (m_iTileSize - iOffsetY) < iTempY ? (m_iTileSize - iOffsetY) : iTempY;
			pObject->Draw(iScreenX, iScreenY, &stRect);

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
}
