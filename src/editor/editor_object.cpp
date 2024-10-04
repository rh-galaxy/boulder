
#include "global.h"

#include "editor_object.h"
#include "editor_world.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

C_Object::C_Object(C_TileHandler* pTileHandler, int iObjNumber, int iObjParam, int iObjParam2)
{
	m_iWorldPosX = 0;
	m_iWorldPosY = 0;

	m_pTileHandler = pTileHandler;
	m_iObjNumber = iObjNumber;
	m_iObjParam = iObjParam;

	m_pstTileInfo = m_pTileHandler->GetTileInfo(m_iObjNumber);

	m_dTimeIntoAnim = 0;
	m_stAnimOffset.x = m_stAnimOffset.y = 0;

	m_pParent = NULL; //does not belong to a world yet

	m_iDefaultAnim = ANIM_IDLE;
	m_iCurFrame = 0;
	switch (m_iObjNumber) {
	case OBJ_KEY:
	case OBJ_DOOR:
		m_iCurFrame = m_iObjParam;
		break;
	}
	m_iCurAnim = m_iDefaultAnim;
}

C_Object::~C_Object()
{
}

void C_Object::SetWorldAndPos(C_World* pParent, int iTileX, int iTileY)
{
	m_pParent = pParent;
	m_iWorldPosX = iTileX;
	m_iWorldPosY = iTileY;
}

S_AnimInfo* C_Object::GetAnimInfo(int iAnimNumber)
{
	int i, iSize = m_pstTileInfo->iNumAnims;
	for (i = 0; i < iSize; i++) {
		if (m_pstTileInfo->pstAnims[i].iAnimNumber == iAnimNumber) return &m_pstTileInfo->pstAnims[i];
	}
	return NULL; //error, not found
}

void C_Object::SetAnim(int iAnimNumber)
{
	if (iAnimNumber == -1) iAnimNumber = m_iDefaultAnim;
	S_AnimInfo* pstAnim = GetAnimInfo(iAnimNumber);
	if (pstAnim) {
		m_iCurAnim = iAnimNumber;
		m_dTimeIntoAnim = 0;
		//UpdateAnimation(0); //init animation
	}
}

void C_Object::Draw(int iScreenX, int iScreenY, S_Rect* pstPartOfTile, bool bOverrideTransparancy)
{
	m_pTileHandler->Draw(m_iObjNumber, m_iCurFrame, iScreenX, iScreenY, pstPartOfTile, bOverrideTransparancy);
}

void C_Object::DrawCliped(int iScreenX, int iScreenY, S_Rect* pstClipRect, bool bOverrideTransparancy)
{
	int iTileSize = m_pTileHandler->m_iTileSize;
	int iOffsetX = 0;
	int iOffsetY = 0;
	iScreenX += m_stAnimOffset.x; //add offset
	iScreenY += m_stAnimOffset.y;

	if (iScreenX < pstClipRect->x) {
		iOffsetX = pstClipRect->x - iScreenX;
		iScreenX = pstClipRect->x;
		iTileSize -= iOffsetX;
		//if(iTileSize<0) return; //out of limits
	}
	if ((iScreenX + iTileSize) > (pstClipRect->x + pstClipRect->width)) {
		iTileSize -= ((iScreenX + iTileSize) - (pstClipRect->x + pstClipRect->width));
		//if(iTileSize<0) return; //out of limits
	}
	if (iScreenY < pstClipRect->y) {
		iOffsetY = pstClipRect->y - iScreenY;
		iScreenY = pstClipRect->y;
		iTileSize -= iOffsetY;
		//if(iTileSize<0) return; //out of limits
	}
	if ((iScreenY + iTileSize) > (pstClipRect->y + pstClipRect->height)) {
		iTileSize -= ((iScreenY + iTileSize) - (pstClipRect->y + pstClipRect->height));
		//if(iTileSize<0) return; //out of limits
	}

	S_Rect stPartOfTile = { iOffsetX,iOffsetY,iTileSize,iTileSize };
	m_pTileHandler->Draw(m_iObjNumber, m_iCurFrame, iScreenX, iScreenY, &stPartOfTile, bOverrideTransparancy);
}

S_Point C_Object::GetPos()
{
	S_Point stPos = { m_iWorldPosX, m_iWorldPosY };
	return stPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
