
#include <math.h>

#include "global.h"

#include "game_object.h"
#include "game_world.h"

/////////////////////////////////////////////////////////////////////////////

static const S_Point s_astP[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
static int s_aiAnimMove[4] = { ANIM_MOVE_RIGHT, ANIM_MOVE_DOWN, ANIM_MOVE_LEFT, ANIM_MOVE_UP };

static const float HEARING_RANGE = 600.0f; //maxdistance sound is played at all
static const float PANNING_RANGE = 370.0f; //maxdistance sound is played in both speakers

C_Control* C_Object::s_pControl = NULL;
C_Sound* C_Object::s_pSound = NULL;

int C_Object::s_aiObjProperties[TILENUMBERLIMIT];

S_Point C_Object::s_stListenerPos;
double C_Object::s_dTimeSinceDiamondHitSound;
double C_Object::s_dTimeSinceExplosionSound;
double C_Object::s_dTimeSinceStoneHitSound;
bool C_Object::s_bPlayEnemyBlueSound;
double C_Object::s_dTimeSinceEnemyBlueSound;
S_Point C_Object::s_stNearestEnemyBluePos;
bool C_Object::s_bPlayEnemyRobotSound;
double C_Object::s_dTimeSinceEnemyRobotSound;
S_Point C_Object::s_stNearestEnemyRobotPos;
bool C_Object::s_bPlayEnemyEatingSound;
double C_Object::s_dTimeSinceEnemyEatingSound;
S_Point C_Object::s_stNearestEnemyEatingPos;
double C_Object::s_dTimeSinceEnemyJumpingSound;

float C_Object::Distance(int x, int y, int x2, int y2)
{
	float fDistX = (float)((x * 32 + 16) - (x2 * 32 + 16));
	float fDistY = (float)((y * 32 + 16) - (y2 * 32 + 16));
	float fDist = (float)sqrt(((double)fDistX * (double)fDistX) + ((double)fDistY * (double)fDistY));
	return fDist;
}

void C_Object::InitObjPropertiesOnce(C_Sound* pSound, C_Control* pControl)
{
	s_pSound = pSound;
	s_pControl = pControl;

	s_aiObjProperties[OBJ_EMPTY] = OBJ_EMPTY_TYPE;
	s_aiObjProperties[OBJ_WALL] = OBJ_WALL_TYPE;
	s_aiObjProperties[OBJ_EARTH] = OBJ_EARTH_TYPE;
	s_aiObjProperties[OBJ_DIAM_RED] = OBJ_DIAM_RED_TYPE;
	s_aiObjProperties[OBJ_DIAM_BLUE] = OBJ_DIAM_BLUE_TYPE;
	s_aiObjProperties[OBJ_STONE] = OBJ_STONE_TYPE;
	s_aiObjProperties[OBJ_WALL_D] = OBJ_WALL_D_TYPE;
	s_aiObjProperties[OBJ_WALL_DS] = OBJ_WALL_DS_TYPE;
	s_aiObjProperties[OBJ_NUT] = OBJ_NUT_TYPE;
	s_aiObjProperties[OBJ_BOMB] = OBJ_BOMB_TYPE;
	s_aiObjProperties[OBJ_KEY] = OBJ_KEY_TYPE;
	s_aiObjProperties[OBJ_DOOR] = OBJ_DOOR_TYPE;
	s_aiObjProperties[OBJ_DOOR_GREY] = OBJ_DOOR_TYPE;
	s_aiObjProperties[OBJ_MUD] = OBJ_MUD_TYPE;
	s_aiObjProperties[OBJ_DIAM_MAKER] = OBJ_DIAM_MAKER_TYPE;
	s_aiObjProperties[OBJ_GREEN_WALL_LEFT] = OBJ_GREEN_WALL_TYPE;
	s_aiObjProperties[OBJ_GREEN_WALL_RIGHT] = OBJ_GREEN_WALL_TYPE;
	s_aiObjProperties[OBJ_GREEN_BOTTOM] = OBJ_GREEN_WALL_TYPE;
	s_aiObjProperties[OBJ_GREEN_SLIP_LEFT] = OBJ_GREEN_SLIP_TYPE;
	s_aiObjProperties[OBJ_GREEN_SLIP_RIGHT] = OBJ_GREEN_SLIP_TYPE;
	s_aiObjProperties[OBJ_LAVA] = OBJ_LAVA_TYPE;
	s_aiObjProperties[OBJ_LAVA_SPLASH] = OBJ_LAVA_SPLASH_TYPE;
	s_aiObjProperties[OBJ_MINE] = OBJ_MINE_TYPE;
	s_aiObjProperties[OBJ_PLAYER] = OBJ_PLAYER_TYPE;
	s_aiObjProperties[OBJ_EXIT_CLOSED] = OBJ_EXIT_CLOSED_TYPE;
	s_aiObjProperties[OBJ_EXIT_OPEN] = OBJ_EXIT_OPEN_TYPE;
	s_aiObjProperties[OBJ_EXPLOSION] = OBJ_EXPLOSION_TYPE;
	s_aiObjProperties[OBJ_ENEMY0] = OBJ_ENEMY0_TYPE;
	s_aiObjProperties[OBJ_ENEMY1] = OBJ_ENEMY1_TYPE;
	s_aiObjProperties[OBJ_ENEMY2] = OBJ_ENEMY2_TYPE;
	s_aiObjProperties[OBJ_ENEMY3] = OBJ_ENEMY3_TYPE;
	s_aiObjProperties[OBJ_ENEMY4] = OBJ_ENEMY4_TYPE;
	s_aiObjProperties[OBJ_SLIME] = OBJ_SLIME_TYPE;
	s_aiObjProperties[OBJ_OBJECT_MACHINE] = OBJ_OBJECT_MACHINE_TYPE;
}

void C_Object::SetWorldAndPos(C_World* pParent, int iTileX, int iTileY)
{
	m_pParent = pParent;
	m_iWorldPosX = iTileX;
	m_iWorldPosY = iTileY;
}

C_Object::C_Object(C_TileHandler* pTileHandler, int iObjNumber, int iObjParam, int iObjParam2)
{
	m_pTileHandler = pTileHandler;
	m_iObjNumber = iObjNumber;
	m_iObjParam = iObjParam;
	m_iObjProperties = s_aiObjProperties[m_iObjNumber];

	m_pstTileInfo = m_pTileHandler->GetTileInfo(m_iObjNumber);

	m_dTimeIntoAnim = 0.0;
	m_stAnimOffset.x = m_stAnimOffset.y = 0;

	m_bToBeDeleted = false;
	m_bMoving = m_bFalling = false;
	m_pPrevObj = NULL;
	m_pNextObj = NULL;
	InitPos(-1, -1);

	m_iDefaultAnim = ANIM_IDLE;
	m_iCurFrame = 0;
	m_dTimer = -1.0;
	switch (m_iObjNumber) { //handle objects with params
	case OBJ_KEY:
		m_iDefaultAnim = ANIM_IDLE + m_iObjParam;
		break;
	case OBJ_DOOR:
		m_iCurFrame = m_iObjParam;
		break;
	case OBJ_EXPLOSION:
		m_iDefaultAnim = ANIM_EXPLOSION_FIRE;
		m_pNextObj = new C_Object(m_pTileHandler, m_iObjParam, iObjParam2, 0);
		break;
	case OBJ_SLIME:
		m_iDefaultAnim = rand() % 4;
		break;
	case OBJ_LAVA_SPLASH:
		m_pNextObj = new C_Object(m_pTileHandler, OBJ_EMPTY, 0, 0);
		break;
	case OBJ_MINE:
		if (m_iObjParam != 0) m_pNextObj = new C_Object(m_pTileHandler, OBJ_BOMB, 0, 0);
		break;
	}
	m_iCurAnim = m_iDefaultAnim;
	SetAnim(m_iCurAnim);
}

C_Object::~C_Object()
{
	delete m_pPrevObj;
	m_pPrevObj = NULL;
}

void C_Object::Draw(int iScreenX, int iScreenY, S_Rect* pstPartOfTile)
{
	m_pTileHandler->Draw(m_iObjNumber, m_iCurFrame, iScreenX, iScreenY, pstPartOfTile);
}

void C_Object::DrawCliped(int iScreenX, int iScreenY, S_Rect* pstClipRect)
{
	int iTileSizeX = m_pTileHandler->m_iTileSize;
	int iTileSizeY = m_pTileHandler->m_iTileSize;
	int iOffsetX = 0;
	int iOffsetY = 0;
	iScreenX += m_stAnimOffset.x; //add offset
	iScreenY += m_stAnimOffset.y;

	if (m_bAnimClip) {
		if (m_stAnimOffset.x < 0) {
			iOffsetX -= m_stAnimOffset.x;
			iScreenX -= m_stAnimOffset.x;
		}
		if (m_stAnimOffset.y < 0) {
			iOffsetY -= m_stAnimOffset.y;
			iScreenY -= m_stAnimOffset.y;
		}
		iTileSizeX -= abs(m_stAnimOffset.x);
		iTileSizeY -= abs(m_stAnimOffset.y);
	}

	if (iScreenX < pstClipRect->x) {
		iOffsetX += pstClipRect->x - iScreenX;
		iScreenX = pstClipRect->x;
		iTileSizeX -= iOffsetX;
		//if(iTileSizeX<0) return; //out of limits
	}
	if ((iScreenX + iTileSizeX) > (pstClipRect->x + pstClipRect->width)) {
		iTileSizeX -= ((iScreenX + iTileSizeX) - (pstClipRect->x + pstClipRect->width));
		//if(iTileSizeX<0) return; //out of limits
	}
	if (iScreenY < pstClipRect->y) {
		iOffsetY += pstClipRect->y - iScreenY;
		iScreenY = pstClipRect->y;
		iTileSizeY -= iOffsetY;
		//if(iTileSizeY<0) return; //out of limits
	}
	if ((iScreenY + iTileSizeY) > (pstClipRect->y + pstClipRect->height)) {
		iTileSizeY -= ((iScreenY + iTileSizeY) - (pstClipRect->y + pstClipRect->height));
		//if(iTileSizeY<0) return; //out of limits
	}

	S_Rect stPartOfTile = { iOffsetX, iOffsetY, iTileSizeX, iTileSizeY };
	m_pTileHandler->Draw(m_iObjNumber, m_iCurFrame, iScreenX, iScreenY, &stPartOfTile);
}

void C_Object::PlaySoundVolPan(int iSoundIndex, int x, int y)
{
	float fDistX = (float)((x * 32 + 16) - (s_stListenerPos.x * 32 + 16));
	float fDistY = (float)((y * 32 + 16) - (s_stListenerPos.y * 32 + 16));
	float fDist = (float)sqrt(((double)fDistX * (double)fDistX) + ((double)fDistY * (double)fDistY));

	//volume
	float fVol = 1.0f - fDist / HEARING_RANGE;
	if (fVol < 0.0f) fVol = 0.0f;
	//panning
	float fPan = fDistX / PANNING_RANGE;

	if (fVol > 0) {
		switch (iSoundIndex)
		{
		case SOUND_EXPLOSION_FIRE:
			if (s_dTimeSinceExplosionSound > 250) s_dTimeSinceExplosionSound = 0;
			else iSoundIndex = -1;
			break;
		case SOUND_HIT_DIAM:
			if (s_dTimeSinceDiamondHitSound > 250) s_dTimeSinceDiamondHitSound = 0;
			else iSoundIndex = -1;
			break;
		case SOUND_HIT_STONE:
			if (s_dTimeSinceStoneHitSound > 250) s_dTimeSinceStoneHitSound = 0;
			else iSoundIndex = -1;
			break;
		case SOUND_ENEMY_JUMPING:
			if(s_dTimeSinceEnemyJumpingSound > 100) s_dTimeSinceEnemyJumpingSound = 0;
			else iSoundIndex = -1;
		}
	}
	else return;

	if (iSoundIndex!= -1 && s_pSound) s_pSound->Play(iSoundIndex, fVol, fPan, false);
}

void C_Object::InitPos(int iTileX, int iTileY)
{
	m_iWorldPosX = iTileX;
	m_iWorldPosY = iTileY;
}

S_Point C_Object::GetPos()
{
	S_Point p = { m_iWorldPosX, m_iWorldPosY };
	return p;
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
	S_AnimInfo* pstNewAnim = GetAnimInfo(iAnimNumber);
	if (pstNewAnim != NULL) {
		m_bPreDraw = pstNewAnim->bPreDraw;
		m_iCurAnim = iAnimNumber;
		m_dTimeIntoAnim = 0.0;
		UpdateAnimation(0.0); //init animation
	}
}

//update pass 1
bool C_Object::UpdateAnimation(double dTime)
{ //returns true when a sequental anim is past its end
	int iCurFrameOrder;

	bool bPrevFinished = false; //assume animation is playing and still playing when this func returns
	if (m_pPrevObj != NULL) bPrevFinished = m_pPrevObj->UpdateAnimation(dTime);
	m_bAnimFinished = bPrevFinished;

	//always update distances
	if (m_iObjNumber == OBJ_ENEMY1) {
		float fCurDist = Distance(s_stNearestEnemyBluePos.x, s_stNearestEnemyBluePos.y, s_stListenerPos.x, s_stListenerPos.y);
		float fThisDist = Distance(m_iWorldPosX, m_iWorldPosY, s_stListenerPos.x, s_stListenerPos.y);
		if (fThisDist < fCurDist) { s_stNearestEnemyBluePos.x = m_iWorldPosX; s_stNearestEnemyBluePos.y = m_iWorldPosY; }
		s_bPlayEnemyBlueSound = true;
	}
	if (m_iObjNumber == OBJ_ENEMY2) {
		float fCurDist = Distance(s_stNearestEnemyRobotPos.x, s_stNearestEnemyRobotPos.y, s_stListenerPos.x, s_stListenerPos.y);
		float fThisDist = Distance(m_iWorldPosX, m_iWorldPosY, s_stListenerPos.x, s_stListenerPos.y);
		if (fThisDist < fCurDist) { s_stNearestEnemyRobotPos.x = m_iWorldPosX; s_stNearestEnemyRobotPos.y = m_iWorldPosY; }
		s_bPlayEnemyRobotSound = true;
	}
	if (m_iObjNumber == OBJ_ENEMY3) {
		float fCurDist = Distance(s_stNearestEnemyEatingPos.x, s_stNearestEnemyEatingPos.y, s_stListenerPos.x, s_stListenerPos.y);
		float fThisDist = Distance(m_iWorldPosX, m_iWorldPosY, s_stListenerPos.x, s_stListenerPos.y);
		if (fThisDist < fCurDist) { s_stNearestEnemyEatingPos.x = m_iWorldPosX; s_stNearestEnemyEatingPos.y = m_iWorldPosY; }
		s_bPlayEnemyEatingSound = true;
	}

	if (m_dTimeIntoAnim == 0 && dTime != 0) { //anim start, play sound
		int iSoundNumber = -1;

		if (m_iObjNumber == OBJ_DIAM_BLUE && m_iCurAnim == ANIM_SQUEESE) iSoundNumber = SOUND_SQUEESE;
		else if (m_iObjNumber == OBJ_DIAM_RED && m_iCurAnim == ANIM_CRACKFROMNUT) iSoundNumber = SOUND_CRACK;
		else if (m_iObjNumber == OBJ_PLAYER && (m_iCurAnim >= ANIM_MOVE_2XDOWN && m_iCurAnim <= ANIM_MOVE_2XUP)) iSoundNumber = SOUND_DOOR;
		else if (m_iObjNumber == OBJ_PLAYER && (m_iCurAnim == ANIM_PUSH_LEFT || m_iCurAnim == ANIM_PUSH_RIGHT)) iSoundNumber = SOUND_PUSH;
		else if (m_iObjNumber == OBJ_SLIME && (m_iCurAnim == ANIM_SLIME_GROW)) iSoundNumber = SOUND_SLIME_GROW;
		else if (m_iObjNumber == OBJ_ENEMY4 && (m_iCurAnim > 0)) iSoundNumber = SOUND_ENEMY_JUMPING;

		if(iSoundNumber!= -1) PlaySoundVolPan(iSoundNumber, m_iWorldPosX, m_iWorldPosY);
	}

	S_AnimInfo* pstAnim = GetAnimInfo(m_iCurAnim);
	if (pstAnim != NULL) {
		m_dTimeIntoAnim += dTime;
		m_stAnimOffset.x = 0; m_stAnimOffset.y = 0; //set default offset
		m_bAnimClip = pstAnim->bClipInclusive;
		if (pstAnim->iOrder == 1) { //sequental
			iCurFrameOrder = (int)(m_dTimeIntoAnim / pstAnim->iFrameTime);
			if (iCurFrameOrder >= pstAnim->iNumFramesInSequence) {
				m_bAnimFinished = true;
			}
			else {
				m_iCurFrame = pstAnim->aiFrameSequence[iCurFrameOrder];
				if (pstAnim->bUseOffset) { m_stAnimOffset.x = pstAnim->astFrameOffset[iCurFrameOrder].x; m_stAnimOffset.y = pstAnim->astFrameOffset[iCurFrameOrder].y; }
				m_bAnimFinished = false;
			}
		}
		else { //random (animation can never be finnished)
			if (m_dTimeIntoAnim >= pstAnim->iFrameTime || dTime == 0) {
				m_dTimeIntoAnim = 0.00000001; //to be sure m_dTimeIntoAnim!=0.0 next time (even if the update time is wery small)
				iCurFrameOrder = rand() % pstAnim->iNumFramesInSequence;
				m_iCurFrame = pstAnim->aiFrameSequence[iCurFrameOrder];
				//if(stAnim.bUseOffset) m_stAnimOffset = new Point(stAnim.astFrameOffset[iCurFrameOrder]); //radom anims cant have offset!
			}
		}
	}

	if (m_bAnimFinished) m_bMoving = false; //movement finnished
	//added this m_bAnimFinished variable to be able to do
	//a multipass. first update animations on ALL objects
	//then update the objects actions, that way objects in
	//the map are all at the same timestate in the actionupdate

	return m_bAnimFinished;
}

//update earth frame, do this with earth objects
//in the beginning and whenever the player has walked near
void C_Object::UpdateEarth()
{
	//if(m_iObjNumber==OBJ_EARTH) {
	int iNeigbour;
	m_iCurFrame = 0; //0 borders 
	iNeigbour = m_pParent->GetFirstObject(m_iWorldPosX - 1, m_iWorldPosY)->m_iObjNumber; //left
	m_iCurFrame |= (iNeigbour != OBJ_WALL && iNeigbour != OBJ_EARTH) ? 1 : 0;
	iNeigbour = m_pParent->GetFirstObject(m_iWorldPosX + 1, m_iWorldPosY)->m_iObjNumber; //right
	m_iCurFrame |= (iNeigbour != OBJ_WALL && iNeigbour != OBJ_EARTH) ? 2 : 0;
	iNeigbour = m_pParent->GetFirstObject(m_iWorldPosX, m_iWorldPosY - 1)->m_iObjNumber; //up
	m_iCurFrame |= (iNeigbour != OBJ_WALL && iNeigbour != OBJ_EARTH) ? 4 : 0;
	iNeigbour = m_pParent->GetFirstObject(m_iWorldPosX, m_iWorldPosY + 1)->m_iObjNumber; //down
	m_iCurFrame |= (iNeigbour != OBJ_WALL && iNeigbour != OBJ_EARTH) ? 8 : 0;
	//}
}

void C_Object::UpdateObjectMachine()
{
	//if(m_iObjNumber==OBJ_OBJECT_MACHINE) {
	{ //let stone/blue/red/bomb out
		C_Object* pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
		if (pObject->m_iObjNumber == OBJ_EMPTY) {
			if (m_iObjParam <= 0) m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY + 1, OBJ_STONE, ANIM_MOVE_DOWN_FROMMUD, ANIM_IDLE);
			else if (m_iObjParam == 1) m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY + 1, OBJ_DIAM_RED, ANIM_MOVE_DOWN_FROMMUD, ANIM_IDLE);
			else if (m_iObjParam == 2) m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY + 1, OBJ_DIAM_BLUE, ANIM_MOVE_DOWN_FROMMUD, ANIM_IDLE);
			else if (m_iObjParam >= 3) m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY + 1, OBJ_BOMB, ANIM_MOVE_DOWN_FROMMUD, ANIM_IDLE);
			pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
			pObject->m_bMoving = true;
			SetAnim(1);
		}
	}
	//}
}

void C_Object::UpdateMud()
{
	//if(m_iObjNumber==OBJ_MUD) {
	if (m_iObjParam > 0) { //let stone out
		C_Object* pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
		if (pObject->m_iObjNumber == OBJ_EMPTY) {
			m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY + 1, OBJ_STONE, ANIM_MOVE_DOWN_FROMMUD, ANIM_IDLE);
			pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
			pObject->m_bMoving = true;
			m_iObjParam = 0;
			SetAnim(1);
		}
		else if (pObject->m_iObjNumber == OBJ_MUD && pObject->m_iObjParam < 1) {
			pObject->m_iObjParam = 1;
			pObject->SetAnim(1);
			m_iObjParam = 0;
			SetAnim(1);
		}
	}
	else if (m_iObjParam < 1) { //check if stone above and take it into the mud
		C_Object* pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY - 1);
		if (pObject->m_iObjNumber == OBJ_STONE && !pObject->m_bMoving) {
			m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY - 1, OBJ_EMPTY, ANIM_IDLE, ANIM_MOVE_DOWN_INMUD);
			SetAnim(1);
			m_iObjParam = 1;
		}
	}
	//}
}

bool C_Object::StartMaker()
{
	//if(m_iObjNumber==OBJ_DIAM_MAKER) {
	C_Object* pObject;
	int i;

	m_dTimer2 = 0.0;
	if (m_dTimer < 0) { //unstarted
		i = m_iWorldPosX;
		while (m_pParent->GetObject(i - 1, m_iWorldPosY)->m_iObjNumber == OBJ_DIAM_MAKER) i--;

		pObject = m_pParent->GetObject(i, m_iWorldPosY);
		while (pObject->m_iObjNumber == OBJ_DIAM_MAKER) {
			pObject->m_dTimer = 0.0;
			pObject->m_iDefaultAnim = 1;
			pObject->SetAnim(1);
			pObject = m_pParent->GetObject(i, m_iWorldPosY);
			i++;
		}
	}

	return (m_iObjParam != -1); //started?
	//}
}

void C_Object::UpdateMaker(double dTime)
{
	if (m_dTimer >= 0) { //only need updating when started
		m_dTimer += dTime;
		m_dTimer2 += dTime;
		if (m_iObjParam > 0 && m_dTimer2 >= 220.0) { //something to let out
			int iNewObject = OBJ_EMPTY;
			C_Object* pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
			if (pObject->m_iObjNumber == OBJ_EMPTY) {
				switch (m_iObjParam) {
				case OBJ_STONE: iNewObject = OBJ_DIAM_RED; break;
				case OBJ_DIAM_BLUE: iNewObject = OBJ_STONE; break;
				case OBJ_DIAM_RED: iNewObject = OBJ_DIAM_BLUE; break;
				}
				m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY + 1, iNewObject, ANIM_MOVE_DOWN_FROMMAKER, ANIM_IDLE);
				pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
				pObject->m_bMoving = true;
			}
			m_iObjParam = 0; //object is removed anyway
		}

		if (m_iObjParam != -1 && m_dTimer > DIAM_MAKER_LIFETIME) {
			m_iDefaultAnim = ANIM_IDLE;
			m_iObjParam = -1; //the maker is dead
		}
	}
}

void C_Object::SetLavaSplash()
{
	//set splashing object on sides
	C_Object* pObject = m_pParent->GetObject(m_iWorldPosX - 1, m_iWorldPosY - 1);
	if (pObject->m_iObjNumber == OBJ_EMPTY) m_pParent->ActionReplace(m_iWorldPosX - 1, m_iWorldPosY - 1, OBJ_LAVA_SPLASH, 0/*LEFT*/, ANIM_IDLE);
	pObject = m_pParent->GetObject(m_iWorldPosX + 1, m_iWorldPosY - 1);
	if (pObject->m_iObjNumber == OBJ_EMPTY) m_pParent->ActionReplace(m_iWorldPosX + 1, m_iWorldPosY - 1, OBJ_LAVA_SPLASH, 1/*RIGHT*/, ANIM_IDLE);
	PlaySoundVolPan(SOUND_LAVA_SPLASH, m_iWorldPosX, m_iWorldPosY);
}

void C_Object::UpdateLava()
{
	//check if fallable object is above and take it into the lava
	C_Object* pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY - 1);
	if ((pObject->m_iObjProperties & TYPE_FALLING) != 0 && !pObject->m_bMoving) {
		m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY - 1, OBJ_EMPTY, ANIM_IDLE, ANIM_MOVE_DOWN_INLAVA);
		SetLavaSplash();
	}
}

void C_Object::UpdateSlime()
{
	//randomly spawn another slime square
	if (m_iObjParam == 0) return; //stable slime (no grow)

	int iChance = rand() % 25;
	if (iChance != 0) return;
	//4% chance to spawn every time anim finnished (every 100 ms)

	int i, x, y;
	i = rand() % 4;
	x = m_iWorldPosX + s_astP[i].x;
	y = m_iWorldPosY + s_astP[i].y;
	C_Object* pObject = m_pParent->GetObject(x, y);

	if ((pObject->m_iObjProperties & TYPE_PLAYER_WALKABLE) != 0 && pObject->m_pPrevObj == NULL) {
		m_pParent->ActionReplace(x, y, OBJ_SLIME, ANIM_SLIME_GROW, ANIM_IDLE);
	}
}

void C_Object::ReplaceByNext()
{
	C_Object* pToDelete = m_pNextObj;
	m_iObjNumber = m_pNextObj->m_iObjNumber;
	m_iObjProperties = m_pNextObj->m_iObjProperties;
	m_iObjParam = m_pNextObj->m_iObjParam;

	//same as before
	//m_iWorldPosX, m_iWorldPosY;

	//animation state info
	m_dTimeIntoAnim = 0.0;
	m_iCurFrame = 0;
	m_iDefaultAnim = m_pNextObj->m_iDefaultAnim;
	m_iCurAnim = m_iDefaultAnim;

	m_pTileHandler = m_pNextObj->m_pTileHandler;
	m_pstTileInfo = m_pNextObj->m_pstTileInfo;

	m_bMoving = m_bFalling = false;
	delete m_pPrevObj;
	m_pPrevObj = m_pNextObj->m_pPrevObj; //NULL
	m_pNextObj->m_pPrevObj = NULL;
	m_pNextObj = m_pNextObj->m_pNextObj; //can have more next objects...
	delete pToDelete;

	SetAnim(m_iDefaultAnim); //back to default anim (0)
}

//not for enemies
void C_Object::Hit(C_Object* pHitBy, bool bMoveHitBy, int iMoveAnim)
{
	int iObjNum, iObjProp;
	int x, y, x2, y2;

	//set default actionparams here
	if (bMoveHitBy) {
		x = pHitBy->m_iWorldPosX; y = pHitBy->m_iWorldPosY;
		x2 = m_iWorldPosX; y2 = m_iWorldPosY;
	}
	else {
		x = m_iWorldPosX; y = m_iWorldPosY;
		x2 = pHitBy->m_iWorldPosX; y2 = pHitBy->m_iWorldPosY;
	}

	iObjNum = pHitBy->m_iObjNumber;
	iObjProp = pHitBy->m_iObjProperties;

	switch (m_iObjNumber) { //"what object am i?"
	case OBJ_DIAM_RED:
		if ((iObjNum == OBJ_DIAM_RED || iObjNum == OBJ_DIAM_BLUE) && s_dTimeSinceDiamondHitSound > 250) {
			PlaySoundVolPan(SOUND_HIT_DIAM, m_iWorldPosX, m_iWorldPosY);
		}
		break;
	case OBJ_DIAM_BLUE:
		if ((iObjNum == OBJ_DIAM_RED || iObjNum == OBJ_DIAM_BLUE) && s_dTimeSinceDiamondHitSound > 250) {
			PlaySoundVolPan(SOUND_HIT_DIAM, m_iWorldPosX, m_iWorldPosY);
		}
		if (iObjNum == OBJ_STONE) { //get squesed by stones
			m_pParent->ActionMove(x, y, x2, y2, ANIM_MOVE_DOWN, ANIM_SQUEESE);
			pHitBy->m_bFalling = true; //let the stone continue falling, TODO decide
		}
		break;
	case OBJ_NUT:
		if (iObjNum == OBJ_STONE) {
			m_pParent->ActionReplace(x2, y2, OBJ_DIAM_RED, ANIM_CRACKFROMNUT, -1);
		}
		break;
	case OBJ_BOMB: //trig bomb by anything falling on it
		m_pParent->ActionExplode3x3(m_iWorldPosX, m_iWorldPosY);
		break;
	case OBJ_PLAYER: {
		//if (!(pHitBy->m_bFalling && m_iWorldPosY < pHitBy->m_iWorldPosY)) {
		m_pParent->m_bAlive = false;
		if (iObjNum == OBJ_LAVA) {
			m_pParent->ActionReplace(x, y, OBJ_EMPTY, ANIM_IDLE, ANIM_MOVE_DOWN_INLAVA);
		}
		else if (iObjNum == OBJ_BOMB) {
			m_pParent->ActionExplode3x3(x, y);
		}
		else if (iObjNum != OBJ_EXPLOSION) {
			m_pParent->ActionMoveAndDestroyBoth(x, y, x2, y2, iMoveAnim, ANIM_CURRENTANIMNORESET, ANIM_EXPLOSION_FIRE);
		}
		//}
	}
				   break;
	case OBJ_MUD: {
		if (iObjNum == OBJ_STONE && m_iObjParam < 1) {
			m_pParent->ActionReplace(x, y, OBJ_EMPTY, ANIM_IDLE, ANIM_MOVE_DOWN_INMUD);
			SetAnim(1);
			m_iObjParam = 1;
		}
	}
				break;
	case OBJ_LAVA: {
		m_pParent->ActionReplace(x, y, OBJ_EMPTY, ANIM_IDLE, ANIM_MOVE_DOWN_INLAVA);
		SetLavaSplash();
	}
				 break;
	case OBJ_DIAM_MAKER: {
		if (m_iObjParam == 0) { //can take objects (unstarted or empty)
			if (iObjNum == OBJ_STONE || iObjNum == OBJ_DIAM_RED || iObjNum == OBJ_DIAM_BLUE) {
				if (StartMaker()) {
					m_pParent->ActionReplace(x, y, OBJ_EMPTY, ANIM_IDLE, ANIM_MOVE_DOWN_INMAKER);
					m_iObjParam = iObjNum;
				}
			}
		}
	}
					   break;
	}

	if ((m_iObjProperties & TYPE_ENEMY) != 0) {
		//not handled here, should not come here
		iObjNum = iObjNum;
	}
}

//update pass 2 (time param only needed for special objects like OBJ_DIAM_MAKER whos timig is not controlled by the animation playing)
int C_Object::Update(double dTime)
{
	m_dMoveDelayTime -= dTime;

	if ((m_iObjProperties & TYPE_STATIC) != 0 && m_pPrevObj == NULL) return 0;

	int iAnimation = ANIM_IDLE;
	int iAnimation2 = ANIM_IDLE;
	int iLastAnim = m_iCurAnim; //save lastanim (last movement or action)

	//done for all movable objects (or objects with a prevobject)
	bool bWasAnimFinished = m_bAnimFinished;
	if (m_bAnimFinished) {
		delete m_pPrevObj;
		m_pPrevObj = NULL; //remove prevobject
		if (m_pNextObj != NULL) {
			ReplaceByNext();
			if (m_iObjNumber == OBJ_BOMB) { //replaced by a bomb, explode immediatly
				m_pParent->ActionExplode3x3(m_iWorldPosX, m_iWorldPosY);
				return 0;
			}
		}
		else SetAnim(m_iDefaultAnim); //back to default anim (0)
	}

	//update objects by group/properties
	if (!m_bMoving && (m_iObjProperties & TYPE_FALLING) != 0) { //falling objects, no move in progress
		C_Object* pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
		if (pObject->m_pPrevObj == NULL || (pObject->m_iObjProperties & TYPE_ENEMY) != 0 || (pObject->m_iObjNumber == OBJ_PLAYER)) { //can only move to object which are finnished with 2ndary animations

			//fix so objects never stop in the middle of a fall, but fall start is delayed at least one frame time since the last object moved out
			if (pObject->m_dMoveDelayTime <= 0 || m_bFalling) {

				int iNeigbourDownProp = pObject->m_iObjProperties;
				if ((iNeigbourDownProp & TYPE_REPLACABLE) != 0) {
					iAnimation = (iLastAnim == ANIM_IDLE) ? ANIM_MOVE_DOWN_DELAY : ANIM_MOVE_DOWN;
					m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, m_iWorldPosX, m_iWorldPosY + 1, iAnimation, -1/*ANIM_IDLE*/, true);
					m_bFalling = true;
				}
				else if (m_bFalling) {
					m_bFalling = false;
					if ((m_iObjNumber == OBJ_STONE || m_iObjNumber == OBJ_NUT) &&
						(pObject->m_iObjNumber == OBJ_EARTH || pObject->m_iObjNumber == OBJ_STONE || (iNeigbourDownProp & TYPE_WALL) != 0))
					{
						PlaySoundVolPan(SOUND_HIT_STONE, m_iWorldPosX, m_iWorldPosY);
					}
					//stoped (hit something important?)
					if (m_iObjNumber == OBJ_BOMB) { //explode bombs when stoped, no matter what they have hit (except OBJ_EMPTY)
						m_pParent->ActionExplode3x3(m_iWorldPosX, m_iWorldPosY);
						return 0;
					}
					else if ((iNeigbourDownProp & TYPE_HITABLE) != 0) {
						//print("something hit by falling obj. ");
						if (iNeigbourDownProp & TYPE_ENEMY) {
							m_pParent->m_iScore += 25; //all enemies are worth 25 points
							if (pObject->m_iObjParam == 0) m_pParent->ActionMoveAndDestroyBoth(m_iWorldPosX, m_iWorldPosY, m_iWorldPosX, m_iWorldPosY + 1, ANIM_MOVE_DOWN, ANIM_CURRENTANIMNORESET, ANIM_EXPLOSION_FIRE);
							else m_pParent->ActionExplodeInto(m_iWorldPosX, m_iWorldPosY + 1, pObject->m_iObjParam, ANIM_EXPLOSION_SHORT);
							return 0;
						}
						else pObject->Hit(this, true, ANIM_MOVE_DOWN);
					}
				}
			}
		}
	}

	if (!m_bMoving && (m_iObjProperties & TYPE_SLIPPING) != 0) { //sideways slipping objects, no move in progress
		C_Object* pObject = m_pParent->GetObject(m_iWorldPosX, m_iWorldPosY + 1);
		int iNeigbourProp = pObject->m_iObjProperties;
		if ((iNeigbourProp & TYPE_SLIPABLE) != 0 && pObject->m_pPrevObj == NULL) {

			//fix so objects never stop in the middle of a fall, but fall start is delayed at least one frame time since the last object moved out
			if (pObject->m_dMoveDelayTime <= 0 && !m_bFalling) {

				iNeigbourProp = m_pParent->GetObject(m_iWorldPosX - 1, m_iWorldPosY)->m_iObjProperties;
				pObject = m_pParent->GetObject(m_iWorldPosX - 1, m_iWorldPosY + 1);
				if ((iNeigbourProp & TYPE_REPLACABLE) != 0 && (pObject->m_iObjProperties & TYPE_REPLACABLE) != 0 && pObject->m_pPrevObj == NULL) {
					m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, m_iWorldPosX - 1, m_iWorldPosY + 1, ANIM_SLIP_LEFT, -1/*ANIM_IDLE*/, true);
					m_bFalling = true;
				}
				else {
					iNeigbourProp = m_pParent->GetObject(m_iWorldPosX + 1, m_iWorldPosY)->m_iObjProperties;
					pObject = m_pParent->GetObject(m_iWorldPosX + 1, m_iWorldPosY + 1);
					if ((iNeigbourProp & TYPE_REPLACABLE) != 0 && (pObject->m_iObjProperties & TYPE_REPLACABLE) != 0 && pObject->m_pPrevObj == NULL) {
						m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, m_iWorldPosX + 1, m_iWorldPosY + 1, ANIM_SLIP_RIGHT, -1/*ANIM_IDLE*/, true);
						m_bFalling = true;
					}
				}

			}
		}
	}

	//update by object number
	switch (m_iObjNumber) {
	case OBJ_EXIT_CLOSED:
		if (m_pParent->m_iDiamondsLeft <= 0) { //change to an open exit?
			m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY, OBJ_EXIT_OPEN, ANIM_IDLE, -1);
		}
		break;
	case OBJ_EARTH:
		UpdateEarth();
		break;
	case OBJ_MUD:
		if (bWasAnimFinished) UpdateMud();
		break;
	case OBJ_OBJECT_MACHINE:
		if (bWasAnimFinished) UpdateObjectMachine();
		break;
	case OBJ_DIAM_MAKER:
		UpdateMaker(dTime);
		break;
	case OBJ_LAVA:
		UpdateLava();
		break;
	case OBJ_SLIME:
		if (bWasAnimFinished) UpdateSlime();
		break;
	case OBJ_ENEMY0:
		if (bWasAnimFinished) MoveEnemy0(iLastAnim);
		break;
	case OBJ_ENEMY1:
	case OBJ_ENEMY2:
		if (bWasAnimFinished) MoveEnemy1(iLastAnim);
		break;
	case OBJ_ENEMY3:
		if (bWasAnimFinished) MoveEnemy3(iLastAnim);
		break;
	case OBJ_ENEMY4:
		if (bWasAnimFinished) MoveEnemy4(iLastAnim);
		break;
	case OBJ_PLAYER: { //if object is main player: update position (keyboardinput), move view
		int iX, iY;
		if (!m_bMoving) {
			if (m_pParent->m_bAlive) {
				S_GamepadState stGPState;
				s_pControl->GetGamepadState(&stGPState);

				if (s_pControl->IsPressed(DIK_K) || (stGPState.iBack)) { //suicide
					m_pParent->m_bAlive = false; //kill player
					break;
				}
				int iDeltaX = 0, iDeltaY = 0;

				int iCurKeyState =
					((s_pControl->IsPressed(DIK_LEFT) || s_pControl->IsPressed(DIK_A) || (stGPState.iDpad & 4)) ? KEY_LEFT : 0) |
					((s_pControl->IsPressed(DIK_RIGHT) || s_pControl->IsPressed(DIK_D) || (stGPState.iDpad & 8)) ? KEY_RIGHT : 0) |
					((s_pControl->IsPressed(DIK_UP) || s_pControl->IsPressed(DIK_W) || (stGPState.iDpad & 1)) ? KEY_UP : 0) |
					((s_pControl->IsPressed(DIK_DOWN) || s_pControl->IsPressed(DIK_S) || (stGPState.iDpad & 2)) ? KEY_DOWN : 0) |
					((s_pControl->IsPressed(DIK_RSHIFT) || s_pControl->IsPressed(DIK_NUMPAD0) || (stGPState.iButton & 1)) ? KEY_RSHIFT : 0);

				if (m_pParent->m_iKeyState != iCurKeyState) {
					m_dTimer = 0.0;
					m_pParent->m_bSnapUsed = false;
					m_pParent->m_iKeyState = iCurKeyState;
				}
				bool bSnapPressed = (iCurKeyState & KEY_RSHIFT) != 0;
				if (!m_pParent->m_bSnapUsed) {
					if (bSnapPressed) m_dTimer += dTime;
					if ((iCurKeyState & KEY_LEFT) != 0) { iDeltaX -= 1; iAnimation = ANIM_MOVE_LEFT; }
					else if ((iCurKeyState & KEY_RIGHT) != 0) { iDeltaX += 1; iAnimation = ANIM_MOVE_RIGHT; }
					else if ((iCurKeyState & KEY_UP) != 0) { iDeltaY -= 1; iAnimation = ANIM_MOVE_UP; }
					else if ((iCurKeyState & KEY_DOWN) != 0) { iDeltaY += 1; iAnimation = ANIM_MOVE_DOWN; }
					else m_dTimer = 0.0;
				}
				iX = m_iWorldPosX + iDeltaX;
				iY = m_iWorldPosY + iDeltaY;

				if (iX != m_iWorldPosX || iY != m_iWorldPosY) { //player wants to move
					C_Object* pDestObj = m_pParent->GetObject(iX, iY);
					int iDestProp = pDestObj->m_iObjProperties;
					int iDestObjNum = pDestObj->m_iObjNumber;

					if (!bSnapPressed && (pDestObj->m_bFalling || ((iDestProp & TYPE_ENEMY) != 0 && iDestObjNum != OBJ_ENEMY3) || iDestObjNum == OBJ_LAVA || (iDestObjNum == OBJ_EXPLOSION && pDestObj->m_iCurAnim != ANIM_EXPLOSION_SNAP))) {
						//kill player when hit a dangerous object
						Hit(pDestObj, false, iAnimation);
					}
					else if ((iDestProp & TYPE_PLAYER_WALKABLE) != 0) {
						if (bSnapPressed) { //snap walk
							if (m_dTimer >= 1100 && m_pParent->m_iNumMines > 0) { //lay mine
								m_pParent->m_iNumMines--;
								m_pParent->ActionReplace(iX, iY, OBJ_MINE, ANIM_ARMED_MINE, ANIM_IDLE);
								m_dTimer = 0.0;
								m_pParent->m_bSnapUsed = true; //set snaptake (player must change keystate to be able to do it again)
								PlaySoundVolPan(SOUND_ARM_MINE, m_iWorldPosX, m_iWorldPosY);
							}
							else { //snap dig
								if (pDestObj->m_iObjNumber == OBJ_EARTH) {
									m_pParent->ActionReplace(iX, iY, OBJ_EXPLOSION, ANIM_EXPLOSION_SNAP, -1);
									m_pParent->m_bSnapUsed = true; //set snaptake (player must change keystate to be able to do it again)
									PlaySoundVolPan(SOUND_EXPLOSION_SNAP, iX, iY);
								}
							}
						}
						else { //normal walk
							m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, iX, iY, iAnimation, ANIM_IDLE);
						}
					}
					else if ((iDestProp & TYPE_PLAYER_TAKABLE) != 0 && !pDestObj->m_bFalling && pDestObj->m_iCurAnim != ANIM_ARMED_MINE) {
						//count for taking of object
						switch (pDestObj->m_iObjNumber) {
						case OBJ_DIAM_BLUE:
							iAnimation2 = ANIM_TAKE_DIAMOND;
							m_pParent->m_iDiamondsLeft -= 3;
							m_pParent->m_iScore += 25;
							PlaySoundVolPan(SOUND_TAKE_DIAM, iX, iY);
							break;
						case OBJ_DIAM_RED:
							iAnimation2 = ANIM_TAKE_DIAMOND;
							m_pParent->m_iDiamondsLeft -= 1;
							m_pParent->m_iScore += 10;
							PlaySoundVolPan(SOUND_TAKE_DIAM, iX, iY);
							break;
						case OBJ_KEY:
							iAnimation2 = ANIM_TAKE_KEY + pDestObj->m_iObjParam;
							m_pParent->m_iDoorKeys |= (1 << pDestObj->m_iObjParam);
							PlaySoundVolPan(SOUND_TAKE_KEY, iX, iY);
							break;
						case OBJ_MINE:
							iAnimation2 = ANIM_TAKE_MINE;
							m_pParent->m_iNumMines++;
							PlaySoundVolPan(SOUND_TAKE_KEY, iX, iY);
							break;
						}

						if (bSnapPressed) { //snap take
							m_pParent->ActionReplace(iX, iY, OBJ_EMPTY, ANIM_IDLE, iAnimation2);
							m_pParent->m_bSnapUsed = true; //set snaptake (player must change keystate to be able to do it again)
						}
						else { //normal walk and take
							m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, iX, iY, iAnimation, iAnimation2);
						}
					}
					else if (!bSnapPressed) { //no snap walking here

						if ((iDestProp & TYPE_PLAYER_PUSHABLE) != 0) {
							if (iX != m_iWorldPosX && !pDestObj->m_bMoving) { //can only push horizontaly, and nonmoving objects
								int i2X = m_iWorldPosX + (2 * iDeltaX);
								if ((m_pParent->GetObject(i2X, m_iWorldPosY)->m_iObjProperties & TYPE_REPLACABLE) != 0
									&& ((m_pParent->GetObject(iX, m_iWorldPosY + 1)->m_iObjProperties & TYPE_REPLACABLE) == 0)) { //can be pushed?
									m_pParent->ActionMove(iX, m_iWorldPosY, i2X, m_iWorldPosY, iAnimation + 4, -1); //move pushable
									m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, iX, m_iWorldPosY, iAnimation + 4, -1); //move player
								}
							}
						}
						else if (iDestObjNum == OBJ_EXIT_OPEN) { //player finnished game
							m_pParent->m_bFinished = true;
							m_pParent->m_iScore += 500; //finnished bonus
							//move into exit, player will be deleted when the anim is finnished
							m_pParent->ActionMoveAndDestroyBoth(m_iWorldPosX, m_iWorldPosY, iX, iY, iAnimation, ANIM_CURRENTANIMNORESET, ANIM_EXPLOSION_SNAP);
							/**/return 0;
						}
						else if (iDestObjNum == OBJ_DOOR || iDestObjNum == OBJ_DOOR_GREY) {
							if ((m_pParent->m_iDoorKeys & (1 << pDestObj->m_iObjParam)) != 0) { //player has the right key and can move through the door if the way is free
								int i2X = m_iWorldPosX + (2 * iDeltaX);
								int i2Y = m_iWorldPosY + (2 * iDeltaY);
								if ((m_pParent->GetObject(i2X, i2Y)->m_iObjProperties & TYPE_REPLACABLE) != 0) { //will move there
									m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, i2X, i2Y, iAnimation + 40, ANIM_IDLE);
								}
							}
						}
					}
				}
			}
			else { //player is commanded to die externaly (suicide or time is up)
				if (!m_pParent->m_bFinished) {
					PlaySoundVolPan(SOUND_EXPLOSION_FIRE, m_iWorldPosX, m_iWorldPosY);
					m_pParent->ActionReplace(m_iWorldPosX, m_iWorldPosY, OBJ_EXPLOSION, ANIM_EXPLOSION_FIRE, -1);
					return 0;
				}
			}
		}
		//always move viewpos
		iX = (int)(((m_iWorldPosX + 0.5) * m_pParent->m_iTileSize + m_stAnimOffset.x) - (m_pParent->m_stViewRect.width / 2));
		iY = (int)(((m_iWorldPosY + 0.5) * m_pParent->m_iTileSize + m_stAnimOffset.y) - (m_pParent->m_stViewRect.height / 2));
		m_pParent->MoveAbsolute(iX, iY);
		C_Object::s_stListenerPos.x = m_iWorldPosX; C_Object::s_stListenerPos.y = m_iWorldPosY;
	}
	break;
	}

	return 0;
}

//0 normal move, 1 move and destroy 1x1, 2 move and destroy 3x3
void C_Object::MoveEnemy(int iMove, int x, int y, int iMoveAnim)
{
	if (iMove == 0) {
		m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, x, y, iMoveAnim, -1);
	}
	else if (iMove == 1) { //move and kill player and self
		m_pParent->ActionMoveAndDestroyBoth(m_iWorldPosX, m_iWorldPosY, x, y, iMoveAnim, ANIM_CURRENTANIMNORESET, ANIM_EXPLOSION_FIRE);
	}
	else if (iMove == 2) { //move and kill self
		m_pParent->m_iScore += 25; //all enemies are worth 25 points

		//not handled by by falling object hit, need to handle it here if the enemy wants to move into falling object
		bool bBomb = m_pParent->GetObject(x, y)->m_iObjNumber == OBJ_BOMB;
		if (!bBomb) {
			m_pParent->ActionMove(m_iWorldPosX, m_iWorldPosY, x, y, iMoveAnim, -1);
			m_pParent->ActionExplodeInto(x, y, m_iObjParam, ANIM_EXPLOSION_SHORT);
		}
		else m_pParent->ActionExplode3x3(x, y); //need to explode the bomb before moving
	}
}

//ENEMY AMOEBA
//always runs to the right of its last heading (will be stuck if no wall to follow)
void C_Object::MoveEnemy0(int iLastAnim)
{
	int aiOrder[4][4] = { {2, 1, 0, 3}, {3, 2, 1, 0}, {1, 0, 3, 2}, {0, 3, 2, 1} };
	int aiAnimTurnCW[] = { ANIM_TURN_RIGHT_CW, ANIM_TURN_DOWN_CW, ANIM_TURN_LEFT_CW, ANIM_TURN_UP_CW };
	int i, x, y, iIndex, iLastWasTurn = 0;
	C_Object* pCloseTo;

	int iOrder = iLastAnim - 1;
	if (iOrder >= 44) {
		iOrder = iOrder - 44;
		iLastWasTurn = 1;
	}
	if (iOrder < 0 || iOrder>3) iOrder = 0;

	for (i = 0; i < 4; i++) {
		iIndex = aiOrder[iOrder][i];
		x = m_iWorldPosX + s_astP[iIndex].x;
		y = m_iWorldPosY + s_astP[iIndex].y;
		pCloseTo = m_pParent->GetObject(x, y);
		if (iLastWasTurn || i == 1) { //last was delay, or forward
			if (pCloseTo->m_iObjNumber == OBJ_EMPTY && pCloseTo->m_pPrevObj == NULL) {
				MoveEnemy(0, x, y, s_aiAnimMove[iIndex]);
				break;
			}
			else if (pCloseTo->m_iObjNumber == OBJ_PLAYER) { //move and kill player and self
				MoveEnemy(1, x, y, s_aiAnimMove[iIndex]);
				break;
			}
			else if (pCloseTo->m_bFalling) { //move and kill self
				MoveEnemy(2, x, y, s_aiAnimMove[iIndex]);
				break;
			}

		}
		else {
			if ((pCloseTo->m_iObjNumber == OBJ_EMPTY && pCloseTo->m_pPrevObj == NULL)
				|| pCloseTo->m_iObjNumber == OBJ_PLAYER
				|| pCloseTo->m_bFalling) {
				//wants to move here, do a delay anim to ensure tile is free
				SetAnim(iOrder + 45);
				m_bMoving = true;
				break;
			}
		}
	}
}

//ENEMY BLUE (INSECT)
//always runs to the right of its last heading (will be stuck if no wall to follow)
void C_Object::MoveEnemy1(int iLastAnim)
{
	int aiOrder[4][3] = { {2, 1, 0},{3, 2, 1},{1, 0, 3},{0, 3, 2} }; //right, forward, left
	int aiAnimTurnCW[] = { ANIM_TURN_DOWN_CW, ANIM_TURN_LEFT_CW, ANIM_TURN_UP_CW, ANIM_TURN_RIGHT_CW };
	int aiAnimTurnCCW[] = { ANIM_TURN_UP_CCW, ANIM_TURN_RIGHT_CCW, ANIM_TURN_DOWN_CCW, ANIM_TURN_LEFT_CCW };
	int i, x, y, iIndex;
	int iLastWasTurn = 0; //0=not turn, 1=CW, 2=CCW
	C_Object* pCloseTo;

	int iOrder = iLastAnim - 1;
	if (iOrder >= 44) {
		iOrder = iOrder - 44;
		iLastWasTurn = 1;
		if (iOrder >= 4) {
			iOrder = iOrder - 4;
			iLastWasTurn = 2;
		}
	}
	if (iOrder < 0 || iOrder>3) iOrder = 0;

	//check close to player or slime first
	for (i = 0; i <= 2; i++) {
		iIndex = aiOrder[iOrder][i];
		x = m_iWorldPosX + s_astP[iIndex].x;
		y = m_iWorldPosY + s_astP[iIndex].y;
		pCloseTo = m_pParent->GetObject(x, y);
		if ((pCloseTo->m_iObjNumber == OBJ_PLAYER) || (pCloseTo->m_iObjNumber == OBJ_SLIME)) {
			m_pParent->ActionExplodeInto(m_iWorldPosX, m_iWorldPosY, m_iObjParam, ANIM_EXPLOSION_SHORT);
			/**/return;
		}
	}

	//right free (or turn right)
	int iFwdHeading = aiOrder[iOrder][1];
	iIndex = aiOrder[iOrder][0];
	x = m_iWorldPosX + s_astP[iIndex].x;
	y = m_iWorldPosY + s_astP[iIndex].y;
	pCloseTo = m_pParent->GetObject(x, y);

	if (iLastWasTurn == 0 && (
		(pCloseTo->m_iObjNumber == OBJ_EMPTY && pCloseTo->m_pPrevObj == NULL)
		|| (pCloseTo->m_bFalling))) {
		SetAnim(aiAnimTurnCW[iFwdHeading]); //turn right if right square free
		m_bMoving = true;
	}
	else {

		//fwd free?
		x = m_iWorldPosX + s_astP[iFwdHeading].x;
		y = m_iWorldPosY + s_astP[iFwdHeading].y;
		pCloseTo = m_pParent->GetObject(x, y);

		if (pCloseTo->m_iObjNumber == OBJ_EMPTY && pCloseTo->m_pPrevObj == NULL) {
			MoveEnemy(0, x, y, s_aiAnimMove[iFwdHeading]);
		}
		else if (pCloseTo->m_bFalling) { //move and kill self
			MoveEnemy(2, x, y, s_aiAnimMove[iFwdHeading]);
		}
		else { //turn right if not forward free

			//left free (or turn left)		
			iIndex = aiOrder[iOrder][2];
			x = m_iWorldPosX + s_astP[iIndex].x;
			y = m_iWorldPosY + s_astP[iIndex].y;
			pCloseTo = m_pParent->GetObject(x, y);

			if (iLastWasTurn == 0 && (
				(pCloseTo->m_iObjNumber == OBJ_EMPTY && pCloseTo->m_pPrevObj == NULL)
				|| (pCloseTo->m_bFalling))) {
				SetAnim(aiAnimTurnCCW[iFwdHeading]); //turn left if left square free
				m_bMoving = true;
			}
			else { //nothing free, just turn left or right...
				if (iLastWasTurn == 1) SetAnim(aiAnimTurnCW[iFwdHeading]);
				else SetAnim(aiAnimTurnCCW[iFwdHeading]);
				m_bMoving = true;
			}
		}
	}
}

//ENEMY EATING
//keeps current heading, then random right or left or no movement, or back or no movement if stuck
void C_Object::MoveEnemy3(int iLastAnim)
{
	int aiOrder[4][4] = { {1, 2, 0, 3},{2, 3, 1, 0},{0, 1, 3, 2},{3, 0, 2, 1} };
	int i, x, y, iIndex;
	C_Object* pCloseTo;

	int iOrder = iLastAnim - 1;
	if (iOrder < 0 || iOrder>3) iOrder = 0;

	for (i = 0; i < 4; i++) {
		iIndex = aiOrder[iOrder][i];
		x = m_iWorldPosX + s_astP[iIndex].x;
		y = m_iWorldPosY + s_astP[iIndex].y;
		pCloseTo = m_pParent->GetObject(x, y);

		if (i > 0) {
			int iChance = rand() % 4 * i;
			if (iChance == 0) break;
			if (iChance == 1 || iChance == 2) continue;
		}

		if (pCloseTo->m_iObjNumber == OBJ_EMPTY && pCloseTo->m_pPrevObj == NULL) {
			MoveEnemy(0, x, y, s_aiAnimMove[iIndex]);
			break;
		}
		else if (pCloseTo->m_iObjNumber == OBJ_DIAM_BLUE && !pCloseTo->m_bMoving) {
			m_pParent->ActionReplace(x, y, OBJ_EMPTY, ANIM_IDLE, ANIM_TAKE_DIAMOND);
			PlaySoundVolPan(SOUND_SQUEESE, x, y);
			break;
		}
		else if (pCloseTo->m_iObjNumber == OBJ_PLAYER) { //move and kill player and self
			MoveEnemy(1, x, y, s_aiAnimMove[iIndex]);
			break;
		}
		else if (pCloseTo->m_bFalling && pCloseTo->m_iObjNumber != OBJ_DIAM_RED && pCloseTo->m_iObjNumber != OBJ_DIAM_BLUE) { //move and kill self
			MoveEnemy(2, x, y, s_aiAnimMove[iIndex]);
			break;
		}
	}
}

//ENEMY JUMPING
//seeks out player in a straight line, meaning it will stop at walls
void C_Object::MoveEnemy4(int iLastAnim)
{
	int x, y;
	C_Object* pCloseTo = NULL;

	if (iLastAnim != ANIM_IDLE) return;

	int iChance = rand() % 4;
	if (iChance > 1) return;

	int seekX = m_pParent->m_iLastPlayerPosX;
	int seekY = m_pParent->m_iLastPlayerPosY;

	int iWayX = 0, iWayY = 0;
	int iAnim = 0, iAnimX, iAnimY;
	if (m_iWorldPosX > seekX) { iWayX = -1; iAnimX = ANIM_MOVE_LEFT; }
	if (m_iWorldPosX < seekX) { iWayX = 1; iAnimX = ANIM_MOVE_RIGHT; }
	if (m_iWorldPosY > seekY) { iWayY = -1; iAnimY = ANIM_MOVE_UP; }
	if (m_iWorldPosY < seekY) { iWayY = 1; iAnimY = ANIM_MOVE_DOWN; }

	if (iChance == 0 && iWayX != 0) {
		x = m_iWorldPosX + iWayX;
		y = m_iWorldPosY + 0;
		pCloseTo = m_pParent->GetObject(x, y);
		iAnim = iAnimX;
	}
	if (iChance == 1 && iWayY != 0) {
		x = m_iWorldPosX + 0;
		y = m_iWorldPosY + iWayY;
		pCloseTo = m_pParent->GetObject(x, y);
		iAnim = iAnimY;
	}

	if (pCloseTo) {
		if (pCloseTo->m_iObjNumber == OBJ_EMPTY && pCloseTo->m_pPrevObj == NULL) {
			MoveEnemy(0, x, y, iAnim);
			return;
		}
		else if (pCloseTo->m_iObjNumber == OBJ_PLAYER) { //move and kill player and self
			MoveEnemy(1, x, y, iAnim);
			return;
		}
		else if (pCloseTo->m_bFalling) { //move and kill self
			MoveEnemy(2, x, y, iAnim);
		}
	}
}
