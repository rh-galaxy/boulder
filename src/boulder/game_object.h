#pragma once

#include <stdio.h>
#include <vector>

#include "common.h"
#include "image.h"
#include "graph.h"

#include "input.h"
#include "soundfx.h"

#include "tileset.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

//fwd decl
class C_World;

class C_Object
{
public:
	C_Object(C_TileHandler* pTileHandler, int iObjNumber, int iObjParam, int iObjParam2); //param2 only used for OBJ_EXPLOSION

	static C_Control* s_pControl;
	static C_Sound* s_pSound;

	static void InitObjPropertiesOnce(C_Sound* pSound, C_Control* pControl);

	void SetWorldAndPos(C_World* pParent, int iTileX, int iTileY);
	bool m_bToBeDeleted;

	C_Object* m_pPrevObj;
	C_Object* m_pNextObj;
	S_Point GetPos();
	void SetAnim(int iAnimNumber);
	void Hit(C_Object* pHitBy, bool bMoveHitBy, int iMoveAnim);

	bool UpdateAnimation(double dTime); //update pass 1
	int Update(double dTime); //update pass 2

	void Draw(int iScreenX, int iScreenY, S_Rect* pstPartOfTile);
	void DrawCliped(int iScreenX, int iScreenY, S_Rect* pstClipRect);

	//object identification
	int m_iObjNumber;
	int m_iObjProperties;
	int m_iObjParam; //key/door number and so on
	int m_iWorldPosX, m_iWorldPosY;
	//specific for some objects
	double m_dTimer, m_dTimer2; //used in OBJ_DIAMOND_MAKER

	//movement
	S_Point m_stAnimOffset; //animation offset
	bool m_bAnimClip;
	S_TileInfo* m_pstTileInfo;

	bool m_bAnimFinished; //true after current animation has played its last frame
	bool m_bFalling;      //true while the object has started falling down and has not stoped yet
	bool m_bMoving;       //true while the object is playing an animation with animationoffset (ANIM_MOVE_xxxx and so on)

	//animation state info
	double m_dTimeIntoAnim;
	int m_iCurAnim, m_iCurFrame;
	int m_iDefaultAnim;
	bool m_bPreDraw;
	double m_dMoveDelayTime;

	static void ResetSoundTimeouts() {
		s_dTimeSinceExplosionSound = 1000;
		s_dTimeSinceDiamondHitSound = 1000;
		s_dTimeSinceStoneHitSound = 1000;
		s_bPlayEnemyBlueSound = false;
		s_dTimeSinceEnemyBlueSound = 1000;
		s_stNearestEnemyBluePos.x = s_stNearestEnemyBluePos.y = 10000;
		s_dTimeSinceEnemyRobotSound = 1000;
		s_stNearestEnemyRobotPos.x = s_stNearestEnemyRobotPos.y = 10000;
		s_dTimeSinceEnemyEatingSound = 0;
		s_stNearestEnemyEatingPos.x = s_stNearestEnemyEatingPos.y = 10000;
		s_dTimeSinceEnemyJumpingSound = 0;
	}
	static void UpdateSoundTimeouts(double dTime) {
		s_dTimeSinceExplosionSound += dTime;
		s_dTimeSinceDiamondHitSound += dTime;
		s_dTimeSinceStoneHitSound += dTime;
		s_bPlayEnemyBlueSound = false;
		s_dTimeSinceEnemyBlueSound += dTime;
		s_stNearestEnemyBluePos.x = s_stNearestEnemyBluePos.y = 10000;
		s_dTimeSinceEnemyRobotSound += dTime;
		s_stNearestEnemyRobotPos.x = s_stNearestEnemyRobotPos.y = 10000;
		s_dTimeSinceEnemyEatingSound += dTime;
		s_stNearestEnemyEatingPos.x = s_stNearestEnemyEatingPos.y = 10000;
		s_dTimeSinceEnemyJumpingSound += dTime;
	};
	static void PlayNearestSounds() {
		if (s_dTimeSinceEnemyBlueSound > 225 && s_bPlayEnemyBlueSound) {
			PlaySoundVolPan(SOUND_ENEMY_BLUE, s_stNearestEnemyBluePos.x, s_stNearestEnemyBluePos.y);
			s_dTimeSinceEnemyBlueSound = 0;
		}
		if (s_dTimeSinceEnemyRobotSound > 225 && s_bPlayEnemyRobotSound) {
			PlaySoundVolPan(SOUND_ENEMY_ROBOT, s_stNearestEnemyRobotPos.x, s_stNearestEnemyRobotPos.y);
			s_dTimeSinceEnemyRobotSound = 0;
		}
		if (s_dTimeSinceEnemyEatingSound > 710 && s_bPlayEnemyEatingSound) {
			PlaySoundVolPan(SOUND_ENEMY_EATING, s_stNearestEnemyEatingPos.x, s_stNearestEnemyEatingPos.y);
			s_dTimeSinceEnemyEatingSound = 0;
		}
	};

	static void PlaySoundVolPan(int iSoundIndex, int x, int y);
private:
	//sound
	static S_Point s_stListenerPos;
	static S_Point s_stNearestEnemyBluePos;
	static bool s_bPlayEnemyBlueSound;
	static double s_dTimeSinceEnemyBlueSound;
	static S_Point s_stNearestEnemyRobotPos;
	static bool s_bPlayEnemyRobotSound;
	static double s_dTimeSinceEnemyRobotSound;
	static S_Point s_stNearestEnemyEatingPos;
	static bool s_bPlayEnemyEatingSound;
	static double s_dTimeSinceEnemyEatingSound;
	static double s_dTimeSinceEnemyJumpingSound;
	static double s_dTimeSinceDiamondHitSound;
	static double s_dTimeSinceExplosionSound;
	static double s_dTimeSinceStoneHitSound;
	static float Distance(int x, int y, int x2, int y2);

	void InitPos(int iTileX, int iTileY);

	S_AnimInfo* GetAnimInfo(int iAnimNumber);

	void UpdateEarth();
	void UpdateMud();
	bool StartMaker();
	void UpdateMaker(double dTime);
	void SetLavaSplash();
	void UpdateLava();
	void UpdateSlime();
	void UpdateObjectMachine();

	void ReplaceByNext();
	void MoveEnemy(int iMove, int x, int y, int iMoveAnim);
	void MoveEnemy0(int iLastAnim);
	void MoveEnemy1(int iLastAnim);
	void MoveEnemy3(int iLastAnim);
	void MoveEnemy4(int iLastAnim);

	static int s_aiObjProperties[TILENUMBERLIMIT];
	C_TileHandler* m_pTileHandler;

	C_World* m_pParent;
};
