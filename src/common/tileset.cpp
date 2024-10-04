
#include "global.h"
#include "tileset.h"

//////////////////////////////////////////////////////////////////////////////////////////

C_TileHandler::C_TileHandler(C_GraphWrapper* pGraph)
{
	m_pGraph = pGraph;
	m_iTileSize = 0;

	int i;
	for (i = 0; i < TILENUMBERLIMIT; i++) {
		m_stTiles[i].iNumFrames = 0;
		m_stTiles[i].iNumAnims = 0;
	}
}

C_TileHandler::~C_TileHandler()
{
	FreeAll();
}

bool C_TileHandler::Load(const char* szPath, const char* szFilename)
{
	FreeAll();

	FILE* pFile;
	char   szLine[512], * pszStr;
	char   szCommand[100];
	int    r, i, iSize, iTemp;
	int    iTileNum, iNumFrames;
	int    iStartX, iStartY;

	char szFullPath[MAX_PATH];
	sprintf(szFullPath, "%s/%s", szPath, szFilename);

	pFile = fopen(szFullPath, "rt");
	if (!pFile) return false;

	do {
		pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);

		if (strcmp(szCommand, "*SIZE") == 0) {
			r = sscanf(szLine, "%s %d", szCommand, &m_iTileSize);
			continue;
		}

		if (strcmp(szCommand, "*TILE") == 0) {
			r = sscanf(szLine, "%s %x", szCommand, &iTileNum); //tilenumber
			m_stTiles[iTileNum].iNumFrames = 0;
			m_stTiles[iTileNum].pstFrames = NULL;
			m_stTiles[iTileNum].bTransparent = false;
			m_stTiles[iTileNum].iNumAnims = 0;
			m_stTiles[iTileNum].pstAnims = NULL;

			pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
			if (strcmp(szCommand, "*FILE") == 0) {
				C_Global::GetWithin(szLine, '"');
				sprintf(szFullPath, "%s/%s", szPath, szLine);
				m_stTiles[iTileNum].pImage = GetImage(szFullPath);
			}

			pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
			if (strcmp(szCommand, "*XY") == 0) {
				r = sscanf(szLine, "%s %d %d", szCommand, &iStartX, &iStartY);
			}

			pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
			if (strcmp(szCommand, "*NFRAMES") == 0) {
				r = sscanf(szLine, "%s %d", szCommand, &iNumFrames);

				m_stTiles[iTileNum].iNumFrames = iNumFrames;
				m_stTiles[iTileNum].pstFrames = new S_Rect[iNumFrames];
				if (m_pGraph) { //ignore drawinginfo if no drawing
					S_Rect sRect = { iStartX,iStartY,m_iTileSize,m_iTileSize }; //set width and height (always same)
					int iImgWidth, iImgHeight;
					m_stTiles[iTileNum].pImage->GetInfo(&iImgWidth, &iImgHeight, NULL);
					for (i = 0; i < iNumFrames; i++) {
						m_stTiles[iTileNum].pstFrames[i] = sRect;

						sRect.x += sRect.width;
						if (sRect.x >= iImgWidth) {
							sRect.x = 0;
							sRect.y += sRect.height;
						}
					}
				}
			}

			pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
			if (strcmp(szCommand, "*TRANSP") == 0) {
				r = sscanf(szLine, "%s %d", szCommand, &i);
				m_stTiles[iTileNum].bTransparent = (i != 0);
			}

			pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
			if (strcmp(szCommand, "*ANIMATIONS") == 0) {
				r = sscanf(szLine, "%s %d", szCommand, &iSize);
				m_stTiles[iTileNum].iNumAnims = iSize;
				m_stTiles[iTileNum].pstAnims = new S_AnimInfo[iSize];

				for (i = 0; i < iSize; i++) {
					S_AnimInfo* pstAnim = &m_stTiles[iTileNum].pstAnims[i];
					pstAnim->bPreDraw = false;

					pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
					if (strcmp(szCommand, "*ANIMNUMBER") == 0) {
						r = sscanf(szLine, "%s %d", szCommand, &pstAnim->iAnimNumber);

						//hack to skip having predraw anim info in the tiledesc file
						//it will only be true for OBJ_PLAYER and ANIM_MOVE2X_... (to draw player _behind_ the door)
						if (iTileNum == OBJ_PLAYER && pstAnim->iAnimNumber >= ANIM_MOVE_2XDOWN && pstAnim->iAnimNumber <= ANIM_MOVE_2XUP) {
							pstAnim->bPreDraw = true;
						}
					}

					pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
					if (strcmp(szCommand, "*ORDER") == 0) {
						r = sscanf(szLine, "%s %d", szCommand, &pstAnim->iOrder);
					}

					pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
					if (strcmp(szCommand, "*FRAMETIME") == 0) {
						r = sscanf(szLine, "%s %d", szCommand, &pstAnim->iFrameTime);
					}

					pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
					if (strcmp(szCommand, "*FRAMESEQUENCE") == 0) {
						pstAnim->iNumFramesInSequence = 0;
						int iPos = 0;
						do {
							//advance one "word" in szLine
							while ((szLine[iPos] == '\t' || szLine[iPos] == ' ') && szLine[iPos] != '\n') iPos++;
							while ((szLine[iPos] != '\t' && szLine[iPos] != ' ') && szLine[iPos] != '\n') iPos++;
							//read it
							iTemp = sscanf(&szLine[iPos], "%d", &pstAnim->aiFrameSequence[pstAnim->iNumFramesInSequence]);
							if (iTemp == 1) pstAnim->iNumFramesInSequence++;
						} while (iTemp == 1);
					}

					pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
					if (strcmp(szCommand, "*USEFRAMEOFFSET") == 0) {
						char szInclusive[16];
						szInclusive[0] = 0;
						int iArgs = sscanf(szLine, "%s %d %s", szCommand, &iTemp, szInclusive);
						pstAnim->bUseOffset = (iTemp != 0);
						pstAnim->bClipInclusive = (iArgs == 3 && strcmp(szInclusive, "CLIPINCLUSIVE") == 0);
						if (iTemp != 0) {
							pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
							int iNum = 0, iPos = 0;
							do {
								//advance one "word" in szLine
								while ((szLine[iPos] == '\t' || szLine[iPos] == ' ') && szLine[iPos] != '\n') iPos++;
								while ((szLine[iPos] != '\t' && szLine[iPos] != ' ') && szLine[iPos] != '\n') iPos++;
								//read it
								iTemp = sscanf(&szLine[iPos], "%d", &pstAnim->astFrameOffset[iNum].x);
								iNum++;
							} while (iTemp == 1);
							pszStr = C_Global::GetNextLineAndCommand(pFile, szLine, szCommand);
							iNum = 0; iPos = 0;
							do {
								//advance one "word" in szLine
								while ((szLine[iPos] == '\t' || szLine[iPos] == ' ') && szLine[iPos] != '\n') iPos++;
								while ((szLine[iPos] != '\t' && szLine[iPos] != ' ') && szLine[iPos] != '\n') iPos++;
								//read it
								iTemp = sscanf(&szLine[iPos], "%d", &pstAnim->astFrameOffset[iNum].y);
								iNum++;
							} while (iTemp == 1);
						}
					}

				}
			}

			continue;
		}

	} while (pszStr != NULL);

	fclose(pFile);
	return true;
}

void C_TileHandler::FreeAll()
{
	m_iTileSize = 0;

	size_t i, iSize = m_oImageList.size();
	for (i = 0; i < iSize; i++) {
		delete m_oImageList[i].pImage;
	}
	m_oImageList.clear();

	for (i = 0; i < TILENUMBERLIMIT; i++) {
		if (m_stTiles[i].iNumFrames != 0) {
			m_stTiles[i].iNumFrames = 0;
			delete[] m_stTiles[i].pstFrames;

			m_stTiles[i].iNumAnims = 0;
			delete[] m_stTiles[i].pstAnims;
		}
	}
}

C_Image* C_TileHandler::GetImage(const char* szFilename)
{
	if (!m_pGraph) return NULL; //do not load images if no draw

	S_ImageInfo sImgInfo;

	bool bResult;
	size_t i, iSize = m_oImageList.size();
	for (i = 0; i < iSize; i++) {
		if (strcmp(m_oImageList[i].szFilename, szFilename) == 0) {
			return m_oImageList[i].pImage;
		}
	}
	strcpy(sImgInfo.szFilename, szFilename);
	sImgInfo.pImage = new C_Image(szFilename, NULL, &bResult);
	if (!bResult) {
		delete sImgInfo.pImage;
		return NULL;
	}
	m_oImageList.push_back(sImgInfo);
	return sImgInfo.pImage;
}

bool C_TileHandler::Draw(int iTileNum, int iFrameNum, int x, int y, S_Rect* pPartOfTile, bool bOverrideTransparancy)
{
	//if(iTileNum>=TILENUMBERLIMIT || iTileNum<0) return false; //out of range
	S_TileInfo* pstTile = &m_stTiles[iTileNum];
	if (iFrameNum >= pstTile->iNumFrames) return false; //no image exists

	S_Rect stPartOfTile = pstTile->pstFrames[iFrameNum];
	if (pPartOfTile) {
		stPartOfTile.width = pPartOfTile->width; stPartOfTile.height = pPartOfTile->height;
		stPartOfTile.x += pPartOfTile->x; stPartOfTile.y += pPartOfTile->y;
	}

	if (pstTile->bTransparent || bOverrideTransparancy) {
		m_pGraph->Blt(pstTile->pImage, &stPartOfTile, x, y, true);
	}
	else {
		m_pGraph->Blt(pstTile->pImage, &stPartOfTile, x, y);
	}

	return true;
}
