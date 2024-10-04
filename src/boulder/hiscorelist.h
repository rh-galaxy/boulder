
#ifndef _HISCORE_H
#define _HISCORE_H

#define MAXSCORESINLIST     15
#define MAXPLAYERNAMELENGTH 14   //13+null

#define BAD_SCORE -500000000

#include "common.h"

#pragma pack(2)
struct S_HighScore
{
	char szMagic[4]; //"B2HI"
	uint16_t iNumInList;
	char szLevelName[40];

	int iScore[MAXSCORESINLIST];
	int iTime[MAXSCORESINLIST];
	char szName[MAXSCORESINLIST][MAXPLAYERNAMELENGTH];
};
#pragma pack()

class C_HighScoreList
{
public:
	C_HighScoreList(char *szFileName, bool bAccending = true);
	~C_HighScoreList();

	void SetLevelName(char *szLevelName); //sets the levelname field of the list

	int Add(char* szName, int iScore, int iTime); //put a new hiscore to the list, saves the list
	bool GetAsData(S_HighScore *o_pstHighScore);
private:
	//"crypto"
	void Invert(S_HighScore *io_pstHighScore);

	//disk operations
	bool Load();
	void Save();
	char *m_szFileName;

	//list operations
	S_HighScore m_stList;

	//sort order
	bool m_bAccending;
};

#endif
