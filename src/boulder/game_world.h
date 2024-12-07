#ifndef _WORLD_H
#define _WORLD_H

#include <stdio.h>
#include <vector>

#include "common.h"
#include "image.h"
#include "graph.h"

#include "game_object.h"
#include "game_world.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

class C_World {
public:
	//creates a world and inits it with the filedata
	C_World(const char* szFilename, C_TileHandler* pTileHandler);
	~C_World();

	static void InitObjProperties(C_Sound* pSound, C_Control* pControl);

	void Draw();
	int Update(double dTime);

	C_Image* GetMiniMap(int iTileSize);

	void GetViewPos(int* o_iWorldX, int* o_iWorldY); //control ViewPos
	void MoveRelative(int x, int y);           //control ViewPos
	void MoveAbsolute(int x, int y);           //control ViewPos

	//common control functions
	S_Rect* GetScreenRect() { return &m_stViewRect; }
	void SetScreenRect(S_Rect* pstViewRect);
	int GetTileSize() { return m_iTileSize; }
	void GetMapSize(int* o_iWidth, int* o_iHeight) { *o_iWidth = m_iWidth * m_iTileSize; *o_iHeight = m_iHeight * m_iTileSize; };

	void SetLevelRules(bool bUseLevelTime);
	void GetMapParameters(int* o_iDiamondsLeft, int* o_iTimeLimit, int* o_iScore) {
		*o_iDiamondsLeft = m_iDiamondsLeft; *o_iTimeLimit = (int)(m_dLevelTime); *o_iScore = m_iScore;
	};
	char* GetMapDescription() { return m_szDescription; };
	bool Load(); //loads the current file

private:
	double m_dDebugSpeed;

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

	char m_szDescription[32];

	bool m_bUseLevelTime;
	bool m_bPlayedShortTimeSound;
	int m_iDiamondsLeft;
	double m_dLevelTime;
	double m_dDelayEndLevel;
	int m_iScore;
	int m_iNumMines;
	int m_iDoorKeys;
	bool m_bSnapUsed;
	bool m_bAlive;
	bool m_bFinished;
	int m_iKeyState;

	int m_iLastPlayerPosX;
	int m_iLastPlayerPosY;

	void Clear();

	//object functions
	C_Object* GetObject(int x, int y);
	C_Object* GetFirstObject(int x, int y);
	C_Object* SetObject(int x, int y, int iObjNumber, int iObjParam = 0);
	C_Object* SetObject(int x, int y, C_Object* pNewObject);
	C_Object* MoveObject(int x, int y, int x2, int y2, bool bDelayNextMove = false); //returns prev obj at destination (inserts OBJ_EMPTY at source)
	C_Object* ReplaceObject(int x, int y, C_Object* pNewObject); //returns prev obj at destination

	void ActionMove(int x, int y, int x2, int y2, int iAnim, int iAnimForPrevObject, bool bDelayNextMove = false);
	void ActionReplace(int x, int y, int iNewObjNum, int iNewAnim, int iAnimForPrevObject);
	void ActionMoveAndDestroyBoth(int x, int y, int x2, int y2, int iAnim, int iAnimForPrevObject, int iExplosionAnim);
	void ActionExplode3x3(int x, int y);
	void ActionExplodeInto(int x, int y, int iExplodeInto, int iExplosionAnim);

	friend C_Object;
};

#endif