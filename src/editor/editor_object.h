#pragma once

#include <stdio.h>
#include <vector>

#include "common.h"
#include "image.h"
#include "graph.h"

#include "input.h"

#include "tileset.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

//fwd decl
class C_World;

class C_Object {
public:
	C_Object(C_TileHandler* pTileHandler, int iObjNumber, int iObjParam = 0, int iObjParam2 = 0); //param2 only used for OBJ_EXPLOSION
	~C_Object();

	//control
	void SetWorldAndPos(C_World* pParent, int iTileX, int iTileY);
	S_Point GetPos();
	void SetAnim(int iAnimNumber);

	//draw
	void Draw(int iScreenX, int iScreenY, S_Rect* pstPartOfTile, bool bOverrideTransparancy = true);
	void DrawCliped(int iScreenX, int iScreenY, S_Rect* pstClipRect, bool bOverrideTransparancy = true);

	int m_iObjNumber;
	int m_iObjParam; //player Id and so on
protected:
	C_World* m_pParent;
	int m_iWorldPosX, m_iWorldPosY;

	//animation state info
	S_AnimInfo* GetAnimInfo(int iAnimNumber);
	double m_dTimeIntoAnim;
	int m_iCurAnim, m_iCurFrame;
	int m_iDefaultAnim;

	//movement
	POINT m_stAnimOffset; //animation offset
	C_TileHandler* m_pTileHandler;
	S_TileInfo* m_pstTileInfo;
};
typedef std::vector<C_Object*> ObjectList;
