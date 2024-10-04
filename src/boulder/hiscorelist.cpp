
//class to handle the server hiscore list
// - save to disk
// - load from disk
// - add/replace entries depending on score

#include "common.h"
#include "hiscorelist.h"


//copy filename to local, and load file
C_HighScoreList::C_HighScoreList(char *szFileName, bool bAccending)
{
	m_szFileName = new char[strlen(szFileName)+1];
	strcpy(m_szFileName, szFileName);

	m_bAccending = bAccending;

	Load();
}

//release mem
C_HighScoreList::~C_HighScoreList()
{
	delete[] m_szFileName;
}

//sets the levelname field of the list, need only be done in a new list
void C_HighScoreList::SetLevelName(char *szLevelName)
{
	strcpy(m_stList.szLevelName, szLevelName);
}

//puts a valid hiscore to the list
// also saves the file
int C_HighScoreList::Add(char *szName, int iScore, int iTime)
{
	bool bPutScore = false;

	//list is sorted in order, keep it that way
	int i = -1, j;
	for(i=0; i<m_stList.iNumInList; i++) { //search list for place of new score
		if((m_bAccending && m_stList.iScore[i]<iScore) || (!m_bAccending && iScore<m_stList.iScore[i])) {
			bPutScore = true;
			break;
		}
	}
	//value of i is the pos

	if(m_stList.iNumInList < MAXSCORESINLIST) {
		bPutScore = true;
		m_stList.iNumInList++; //resize list if not full
	}

	if(bPutScore) {
		//insert score to list
		for(j= m_stList.iNumInList-2; j>=i; j--) {
			//reorder list from place of insertion
			strcpy(m_stList.szName[j+1], m_stList.szName[j]);
			m_stList.iTime[j+1] = m_stList.iTime[j];
			m_stList.iScore[j+1] = m_stList.iScore[j];
		}

		//insert new score on position
		strcpy(m_stList.szName[i], szName);
		m_stList.iScore[i] = iScore;
		m_stList.iTime[i] = iTime;
		//save on disk
		Save();
	}

	return i;
}

//load list from file if exists
bool C_HighScoreList::Load()
{
	//assume empty list
	m_stList.iNumInList = 0;

	FILE *pstFile = fopen(m_szFileName, "rb");
	if(pstFile) {
		size_t iNum = fread(&m_stList, sizeof(S_HighScore), 1, pstFile);
		fclose(pstFile);
		if(iNum==1 && m_stList.szMagic[0]=='B' && m_stList.szMagic[1]=='2' && m_stList.szMagic[2]=='H' && m_stList.szMagic[3]=='I') {
			m_stList.iNumInList = m_stList.iNumInList;
			Invert(&m_stList);
			return true;
		}
	}
	//file did not exist, or is bad (or of a previous version)
	//begin again:
	memset(&m_stList, 0, sizeof(m_stList));
	m_stList.szMagic[0] = 'B';
	m_stList.szMagic[1] = '2';
	m_stList.szMagic[2] = 'H';
	m_stList.szMagic[3] = 'I';

	return false;
}

//save list to file if list is not empty
void C_HighScoreList::Save()
{
	//if list empty, nothing to save
	if(!m_stList.iNumInList) return;

	Invert(&m_stList);

	FILE *pstFile = fopen(m_szFileName, "wb");
	if(pstFile) {
		fwrite(&m_stList, sizeof(S_HighScore), 1, pstFile);
		fclose(pstFile);
	}

	Invert(&m_stList);
}

//a very simple crypto to make the score data file
// unreadable; just inverts all bits
void C_HighScoreList::Invert(S_HighScore *io_pstHighScore)
{
	int i, j;

	for(i=0; i<MAXSCORESINLIST; i++) {
		//io_pstHiscore->iScore[i] = ~io_pstHiscore->iScore[i];
		for(j=0; j<MAXPLAYERNAMELENGTH; j++) {
			io_pstHighScore->szName[i][j] = (char)~io_pstHighScore->szName[i][j];
		}
	}
}

//return the hiscore list contents
bool C_HighScoreList::GetAsData(S_HighScore *o_pstHighScore)
{
	memcpy(o_pstHighScore, &m_stList, sizeof(S_HighScore));
	return (m_stList.iNumInList!=0);
}
