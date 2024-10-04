#ifndef _WORLD_H
#define _WORLD_H

#include <stdio.h>
#include <vector>

#include "common.h"
#include "image.h"
#include "graph.h"

#include "editor_object.h"

//fwd decl
class C_World;

///////////////////////////////////////////////////////////////////////////////////////////////////

class C_World {
public:
	C_World(const char* szFilename, int iWidth, int iHeight, C_TileHandler* pTileHandler);
	//creates an empty world with borders, ex for a world-editor
	C_World(const char* szFilename, C_TileHandler* pTileHandler);
	//creates a world and inits it with the filedata
	~C_World();

	//client functions
	void SetScreenRect(S_Rect* pstViewRect);
	void Draw();
	void GetViewPos(int* o_iWorldX, int* o_iWorldY);
	void MoveRelative(int x, int y);
	void MoveAbsolute(int x, int y);

	S_Rect* GetScreenRect() { return &m_stViewRect; }
	int GetTileSize() { return m_iTileSize; }
	void GetMapSize(int* o_iWidth, int* o_iHeight) { *o_iWidth = m_iWidth * m_iTileSize; *o_iHeight = m_iHeight * m_iTileSize; };

	//object functions
	C_Object* GetObject(int x, int y);
	void SetObject(int x, int y, int iObjNumber, int iObjParam = 0);
	void SetObject(int x, int y, C_Object* pNewObject);
	C_Object* MoveObject(int x, int y, int x2, int y2); //returns prev obj at destination (inserts OBJ_EMPTY at source)
	C_Object* ReplaceObject(int x, int y, C_Object* pNewObject); //returns prev obj at destination

	//editor Functions
	void SetMapParameters(int iDiamondsLeft, int iTimeLimit) { m_iDiamondsLeft = iDiamondsLeft; m_iTimeLimit = iTimeLimit; };
	void GetMapParameters(int* o_iDiamondsLeft, int* o_iTimeLimit) { *o_iDiamondsLeft = m_iDiamondsLeft; *o_iTimeLimit = m_iTimeLimit; };
	void SetMapDescription(const char *szDescription) { strcpy(m_szDescription, szDescription); };
	char *GetMapDescription() { return m_szDescription; };
	bool Load(); //loads the current file
	bool Save(); //saves the current file
	void SetElement(int iScrPixelX, int iScrPixelY, int iTileNum, int iObjParam);
	void GetElement(int iScrPixelX, int iScrPixelY, int* o_iTileNum, int* o_iObjParam);

	void Crop(int iSide);
	void Extend(int iSide);

private:
	C_TileHandler* m_pTileHandler;
	char m_szFilename[256];
	int m_iWidth, m_iHeight;      //in tiles
	C_Object*** m_pGrid;

	void FixLimits();
	bool TestLimits(int* o_iX, int* o_iY, int w, int h);
	S_Rect m_stViewRect;
	int m_iViewPosX, m_iViewPosY; //world pixel coordinates
	int m_iTileSize;
	int m_iViewRectWidthSave, m_iViewRectHeightSave;
	int m_iPixMaxX, m_iPixMaxY;
	int m_iScrollMaxX, m_iScrollMaxY;

	int m_iDiamondsLeft;
	int m_iTimeLimit;
	char m_szDescription[32];

	void Clear();

	friend C_Object;
};

#endif
