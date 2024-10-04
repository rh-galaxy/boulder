#pragma once

#include <stdio.h>
#include <vector>

#include "common.h"
#include "image.h"
#include "graph.h"

//possible object property flags
static const int TYPE_STATIC = 0x0001;
static const int TYPE_REPLACABLE = 0x0002;  //only OBJ_EMPTY
static const int TYPE_ENEMY_WALKABLE = 0x0002;  //only OBJ_EMPTY (therefore same as TYPE_REPLACABLE)
static const int TYPE_DESTRUCTABLE = 0x0004;
static const int TYPE_FALLING = 0x0008;
static const int TYPE_SLIPPING = 0x0010;
static const int TYPE_EXPLODING = 0x0020;
static const int TYPE_MOVABLE = 0x0040;
static const int TYPE_SLIPABLE = 0x0080;
static const int TYPE_HITABLE = 0x0100;
static const int TYPE_PLAYER_PUSHABLE = 0x0200;
static const int TYPE_PLAYER_WALKABLE = 0x0400;
static const int TYPE_PLAYER_TAKABLE = 0x0800;
static const int TYPE_ENEMY = 0x1000;
static const int TYPE_WALL = 0x2000;

//objects name
static const int OBJ_EMPTY = 0x0001;   //TYPE_STATIC | TYPE_REPLACABLE | TYPE_ENEMY_WALKABLE | TYPE_DESTRUCTABLE
static const int OBJ_WALL = 0x0002;   //TYPE_STATIC | TYPE_WALL
static const int OBJ_EARTH = 0x0003;   //TYPE_DESTRUCTABLE | TYPE_PLAYER_WALKABLE
static const int OBJ_DIAM_RED = 0x0004;   //TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_TAKABLE
static const int OBJ_DIAM_BLUE = 0x0005;   //TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_HITABLE | TYPE_PLAYER_TAKABLE
static const int OBJ_STONE = 0x0006;   //TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_PUSHABLE
static const int OBJ_WALL_D = 0x0007;   //TYPE_STATIC | TYPE_DESTRUCTABLE | TYPE_WALL
static const int OBJ_WALL_DS = 0x0008;   //TYPE_STATIC | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_WALL
static const int OBJ_NUT = 0x0009;   //TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_PUSHABLE | TYPE_HITABLE
static const int OBJ_BOMB = 0x000A;   //TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE? | TYPE_SLIPABLE | TYPE_PLAYER_PUSHABLE | TYPE_HITABLE
static const int OBJ_KEY = 0x000B;   //TYPE_DESTRUCTABLE | TYPE_PLAYER_TAKABLE
static const int OBJ_DOOR = 0x000C;   //TYPE_STATIC | TYPE_DESTRUCTABLE | TYPE_WALL
static const int OBJ_DOOR_GREY = 0x000D;   //TYPE_STATIC
static const int OBJ_MUD = 0x000E;   //TYPE_HITABLE
static const int OBJ_DIAM_MAKER = 0x000F;   //TYPE_HITABLE
static const int OBJ_GREEN_WALL_LEFT = 0x0010;   //TYPE_STATIC | TYPE_WALL
static const int OBJ_GREEN_WALL_RIGHT = 0x0011;   //TYPE_STATIC | TYPE_WALL
static const int OBJ_GREEN_SLIP_LEFT = 0x0012;   //TYPE_STATIC | TYPE_SLIPABLE | TYPE_WALL
static const int OBJ_GREEN_SLIP_RIGHT = 0x0013;   //TYPE_STATIC | TYPE_SLIPABLE | TYPE_WALL
static const int OBJ_GREEN_BOTTOM = 0x0014;   //TYPE_STATIC
static const int OBJ_LAVA = 0x0015;   //TYPE_HITABLE
static const int OBJ_LAVA_SPLASH = 0x0016;   //TYPE_DESTRUCTABLE | TYPE_PLAYER_WALKABLE
static const int OBJ_MINE = 0x0020;   //TYPE_DESTRUCTABLE | TYPE_PLAYER_TAKABLE
static const int OBJ_PLAYER = 0x00A0;   //TYPE_HITABLE  ?TYPE_DESTRUCTABLE?
static const int OBJ_EXIT_CLOSED = 0x00A1;   //TYPE_DESTRUCTABLE | TYPE_WALL
static const int OBJ_EXIT_OPEN = 0x00A2;   //TYPE_DESTRUCTABLE | TYPE_WALL
static const int OBJ_EXPLOSION = 0x00B0;   //TYPE_DESTRUCTABLE
static const int OBJ_ENEMY0 = 0x00C0;   //TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY
static const int OBJ_ENEMY1 = 0x00C1;   //TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY
static const int OBJ_ENEMY2 = 0x00C2;   //TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY
static const int OBJ_ENEMY3 = 0x00C3;   //TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY
static const int OBJ_ENEMY4 = 0x00C4;   //TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY
static const int OBJ_SLIME = 0x00C8;   //TYPE_DESTRUCTABLE
static const int OBJ_OBJECT_MACHINE = 0x00D0;   //TYPE_DESTRUCTABLE | TYPE_WALL

//objects properties
static const int OBJ_EMPTY_TYPE = TYPE_STATIC | TYPE_REPLACABLE | TYPE_ENEMY_WALKABLE | TYPE_PLAYER_WALKABLE | TYPE_DESTRUCTABLE;
static const int OBJ_WALL_TYPE = TYPE_STATIC | TYPE_WALL;
static const int OBJ_EARTH_TYPE = TYPE_DESTRUCTABLE | TYPE_PLAYER_WALKABLE;
static const int OBJ_DIAM_RED_TYPE = TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_TAKABLE | TYPE_HITABLE;
static const int OBJ_DIAM_BLUE_TYPE = TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_TAKABLE | TYPE_HITABLE;
static const int OBJ_STONE_TYPE = TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_PUSHABLE;
static const int OBJ_WALL_D_TYPE = TYPE_STATIC | TYPE_DESTRUCTABLE | TYPE_WALL;
static const int OBJ_WALL_DS_TYPE = TYPE_STATIC | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_WALL;
static const int OBJ_NUT_TYPE = TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_PUSHABLE | TYPE_HITABLE;
static const int OBJ_BOMB_TYPE = TYPE_FALLING | TYPE_SLIPPING | TYPE_DESTRUCTABLE | TYPE_SLIPABLE | TYPE_PLAYER_PUSHABLE | TYPE_HITABLE;
static const int OBJ_KEY_TYPE = TYPE_DESTRUCTABLE | TYPE_PLAYER_TAKABLE;
static const int OBJ_DOOR_TYPE = TYPE_STATIC | TYPE_DESTRUCTABLE | TYPE_WALL;
static const int OBJ_MUD_TYPE = TYPE_HITABLE;
static const int OBJ_DIAM_MAKER_TYPE = TYPE_HITABLE | TYPE_DESTRUCTABLE;
static const int OBJ_GREEN_WALL_TYPE = TYPE_STATIC | TYPE_WALL;
static const int OBJ_GREEN_SLIP_TYPE = TYPE_STATIC | TYPE_SLIPABLE | TYPE_WALL;
static const int OBJ_LAVA_TYPE = TYPE_HITABLE;
static const int OBJ_LAVA_SPLASH_TYPE = TYPE_DESTRUCTABLE | TYPE_PLAYER_WALKABLE;
static const int OBJ_MINE_TYPE = TYPE_DESTRUCTABLE | TYPE_PLAYER_TAKABLE;
static const int OBJ_PLAYER_TYPE = TYPE_HITABLE | TYPE_DESTRUCTABLE;
static const int OBJ_EXIT_CLOSED_TYPE = TYPE_DESTRUCTABLE | TYPE_WALL;
static const int OBJ_EXIT_OPEN_TYPE = TYPE_DESTRUCTABLE | TYPE_WALL;
static const int OBJ_EXPLOSION_TYPE = TYPE_DESTRUCTABLE;
static const int OBJ_ENEMY0_TYPE = TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY;
static const int OBJ_ENEMY1_TYPE = TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY;
static const int OBJ_ENEMY2_TYPE = TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY;
static const int OBJ_ENEMY3_TYPE = TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY;
static const int OBJ_ENEMY4_TYPE = TYPE_HITABLE | TYPE_DESTRUCTABLE | TYPE_ENEMY;
static const int OBJ_SLIME_TYPE = TYPE_DESTRUCTABLE | TYPE_WALL;
static const int OBJ_OBJECT_MACHINE_TYPE = TYPE_DESTRUCTABLE | TYPE_WALL;

//property constants for special objects
static const int DIAM_MAKER_LIFETIME = 40000; //20 sec

//animation numbers (as used/specified in tiledesc.txt)
static const int ANIM_IDLE = 0;
static const int ANIM_MOVE_DOWN = 1;
static const int ANIM_MOVE_LEFT = 2;
static const int ANIM_MOVE_RIGHT = 3;
static const int ANIM_MOVE_UP = 4;
static const int ANIM_MOVE_DOWN_DELAY = 5;
static const int ANIM_PUSH_LEFT = 6;
static const int ANIM_PUSH_RIGHT = 7;
static const int ANIM_SLIP_LEFT = 10;
static const int ANIM_SLIP_RIGHT = 11;
static const int ANIM_MOVE_DOWN_INMUD = 14;
static const int ANIM_MOVE_DOWN_FROMMUD = 15;
static const int ANIM_MOVE_DOWN_INMAKER = 16;
static const int ANIM_MOVE_DOWN_INLAVA = 16;
static const int ANIM_MOVE_DOWN_FROMMAKER = 17;
static const int ANIM_SLIME_GROW = 20;
static const int ANIM_TAKE_DIAMOND = 20;
static const int ANIM_TAKE_KEY = 20;
static const int ANIM_TAKE_MINE = 22;
static const int ANIM_SQUEESE = 23;  //SQUEESE DIAMOND
static const int ANIM_CRACKFROMNUT = 24;  //CRACK DIAMOND FROM NUT
static const int ANIM_ARMED_MINE = 25;
static const int ANIM_EXPLOSION_FIRE = 30;
static const int ANIM_EXPLOSION_SNAP = 31;
static const int ANIM_EXPLOSION_SHORT = 32;
static const int ANIM_MOVE_2XDOWN = 41;
static const int ANIM_MOVE_2XLEFT = 42;
static const int ANIM_MOVE_2XRIGHT = 43;
static const int ANIM_MOVE_2XUP = 44;
static const int ANIM_TURN_DOWN_CW = 45;
static const int ANIM_TURN_LEFT_CW = 46;
static const int ANIM_TURN_RIGHT_CW = 47;
static const int ANIM_TURN_UP_CW = 48;
static const int ANIM_TURN_DOWN_CCW = 49;
static const int ANIM_TURN_LEFT_CCW = 50;
static const int ANIM_TURN_RIGHT_CCW = 51;
static const int ANIM_TURN_UP_CCW = 52;
static const int ANIM_CURRENTANIMNORESET = -2; //special constant... only to use with C_World::Actionxxxxx functions

//sound indeces
static const int SOUND_SHORTTIME = 0;
static const int SOUND_EXPLOSION_FIRE = 1;
static const int SOUND_EXPLOSION_SNAP = 2;
static const int SOUND_SQUEESE = 3;
static const int SOUND_CRACK = 4;
static const int SOUND_TAKE_DIAM = 5;
static const int SOUND_TAKE_KEY = 6;
static const int SOUND_PUSH = 7;
static const int SOUND_DOOR = 10;
static const int SOUND_HIT_DIAM = 11;
static const int SOUND_HIT_STONE = 12;
static const int SOUND_ENEMY_BLUE = 13;
static const int SOUND_ENEMY_ROBOT = 14;
static const int SOUND_ENEMY_JUMPING = 15;
static const int SOUND_SLIME_GROW = 16;
static const int SOUND_ARM_MINE = 17;
static const int SOUND_LAVA_SPLASH = 18;
static const int SOUND_ENEMY_EATING = 19;
static const int MUSIC_TITLE = 32;

//keystate flags
static const int KEY_UP = 0x01;
static const int KEY_LEFT = 0x02;
static const int KEY_RIGHT = 0x04;
static const int KEY_DOWN = 0x08;
static const int KEY_RSHIFT = 0x10;
static const int KEY_S = 0x20;

//tileinfo constants
static const int ENEMY_EXPLODE_INTO[9][9] = {
	{OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY},
	{OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY,OBJ_EMPTY},
	{OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_BLUE,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED},
	{OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE},
	{OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_DIAM_RED,OBJ_STONE,OBJ_STONE,OBJ_STONE},
	{OBJ_STONE,OBJ_DIAM_RED,OBJ_STONE,OBJ_STONE,OBJ_KEY,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE},
	{OBJ_STONE,OBJ_DIAM_RED,OBJ_STONE,OBJ_STONE,OBJ_KEY,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE},
	{OBJ_STONE,OBJ_DIAM_RED,OBJ_STONE,OBJ_STONE,OBJ_KEY,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE},
	{OBJ_STONE,OBJ_DIAM_RED,OBJ_STONE,OBJ_STONE,OBJ_KEY,OBJ_STONE,OBJ_STONE,OBJ_STONE,OBJ_STONE}
};
static const int ENEMY_EXPLODE_INTO_PARAM[9][9] = {
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,0,0},
	{0,0,0,0,2,0,0,0,0},
	{0,0,0,0,3,0,0,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#define FRAMESEQUENCENUMBERLIMIT 192
struct S_AnimInfo
{
	int iAnimNumber;
	int iOrder;     //sequential=1/random=2
	int iFrameTime; //frame speed

	int iNumFramesInSequence;
	int aiFrameSequence[FRAMESEQUENCENUMBERLIMIT];
	bool bUseOffset;
	bool bClipInclusive;
	bool bPreDraw;
	POINT astFrameOffset[FRAMESEQUENCENUMBERLIMIT];
};

#define TILENUMBERLIMIT 2048 //limits to 11 bits tilenumbersize
struct S_TileInfo
{
	int iNumFrames; //if numframes == 0 then the record is not valid
	S_Rect* pstFrames;

	int iNumAnims;
	S_AnimInfo* pstAnims;

	C_Image* pImage;
	bool bTransparent;
};

struct S_ImageInfo
{
	char szFilename[256];
	C_Image* pImage;
};
typedef std::vector<struct S_ImageInfo> C_ImageInfoList;

class C_TileHandler
{
public:
	C_TileHandler(C_GraphWrapper* pGraph);
	~C_TileHandler();

	bool Load(const char* szPath, const char* szFilename);
	void FreeAll();

	bool Draw(int iTileNum, int iFrameNum, int x, int y, S_Rect* pPartOfTile, bool bOverrideTransparancy = true);
	S_TileInfo* GetTileInfo(int iTileNum) { return &m_stTiles[iTileNum]; };

	int m_iTileSize;

	C_GraphWrapper* m_pGraph;
private:
	S_TileInfo m_stTiles[TILENUMBERLIMIT];
	C_ImageInfoList m_oImageList;

	//helpers
	C_Image* GetImage(const char* szFilename);
};

struct S_Point
{
	int x;
	int y;
};
