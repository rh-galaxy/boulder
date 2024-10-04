// C_GUI

//class to implement gui elements.
// - elements for a set (one dialog)
//   are read from a definition text file
// - handles input and draw
// - the elements are
//   - dialog background
//   - text
//   - button
//   - edit box
//   - check box
//   - list

#include "gui.h"
#include "fileresource.h"
#include "global.h"

#define BBORDER_SIZE 6
#define EBORDER_SIZE 4

S_FColor C_GUI::s_stStateCol[3] = {
	{0.50f,0.50f,0.50f,0.8f}, //disabled
	{0.85f,0.85f,0.85f,0.8f}, //normal
	{1.00f,1.00f,1.00f,0.8f}  //highlighted
};

S_FColor C_GUI::s_stSelectCol =
	{0.30f,0.60f,1.00f,0.28f};

S_FColor C_GUI::s_stTextColor = 
	{0.8f,0.8f,0.8f ,1};

int C_GUI::s_iObjCount = 0;

C_Font    *C_GUI::s_pFont;
C_Image   *C_GUI::s_pButton[2];
S_Rect     C_GUI::s_stButtonR[9];
C_Image   *C_GUI::s_pEdit;
S_Rect     C_GUI::s_stEditR[9];
C_Image   *C_GUI::s_pCheck[2];
S_Rect     C_GUI::s_stCheckR[9];
C_Image   *C_GUI::s_pListVBar;
S_Rect     C_GUI::s_stListVBarR[6];

//initialization and deinit
C_GUI::C_GUI()
{
	int w, h;
	char szFile[MAX_PATH];

	m_pGraph = C_GraphWrapper::GetGraphWrapperObject();
	m_pInput = C_Control::GetControlObject();

	if(s_iObjCount==0) { //init graphics
		sprintf(szFile, "%s/%s", C_Global::szDataPath, "gui_button.tga");
		s_pButton[0] = new C_Image(szFile, NULL);
		sprintf(szFile, "%s/%s", C_Global::szDataPath, "gui_button_p.tga");
		s_pButton[1] = new C_Image(szFile, NULL); //must be same dimensions as 0
		s_pButton[0]->GetInfo(&w, &h, NULL);
		s_stButtonR[0] = S_RECTMAKE(0,0,                          BBORDER_SIZE,BBORDER_SIZE); //u left corner
		s_stButtonR[1] = S_RECTMAKE(w-BBORDER_SIZE,0,             BBORDER_SIZE,BBORDER_SIZE); //u right corner
		s_stButtonR[2] = S_RECTMAKE(0,h-BBORDER_SIZE,             BBORDER_SIZE,BBORDER_SIZE); //l left corner
		s_stButtonR[3] = S_RECTMAKE(w-BBORDER_SIZE,h-BBORDER_SIZE,BBORDER_SIZE,BBORDER_SIZE); //l right corner
		s_stButtonR[4] = S_RECTMAKE(BBORDER_SIZE,0,2,BBORDER_SIZE);              //upper
		s_stButtonR[5] = S_RECTMAKE(BBORDER_SIZE,h-BBORDER_SIZE,2,BBORDER_SIZE); //lower
		s_stButtonR[6] = S_RECTMAKE(0,BBORDER_SIZE,BBORDER_SIZE,2);              //left
		s_stButtonR[7] = S_RECTMAKE(w-BBORDER_SIZE,BBORDER_SIZE,BBORDER_SIZE,2); //right
		s_stButtonR[8] = S_RECTMAKE(BBORDER_SIZE,BBORDER_SIZE,4,4);              //middle


		sprintf(szFile, "%s/%s", C_Global::szDataPath, "gui_edit.tga");
		s_pEdit      = new C_Image(szFile, NULL);
		s_pEdit->GetInfo(&w, &h, NULL);
		s_stEditR[0] = S_RECTMAKE(0,0,                          EBORDER_SIZE,EBORDER_SIZE); //u left corner
		s_stEditR[1] = S_RECTMAKE(w-EBORDER_SIZE,0,             EBORDER_SIZE,EBORDER_SIZE); //u right corner
		s_stEditR[2] = S_RECTMAKE(0,h-EBORDER_SIZE,             EBORDER_SIZE,EBORDER_SIZE); //l left corner
		s_stEditR[3] = S_RECTMAKE(w-EBORDER_SIZE,h-EBORDER_SIZE,EBORDER_SIZE,EBORDER_SIZE); //l right corner
		s_stEditR[4] = S_RECTMAKE(EBORDER_SIZE,0,2,EBORDER_SIZE);              //upper
		s_stEditR[5] = S_RECTMAKE(EBORDER_SIZE,h-EBORDER_SIZE,2,EBORDER_SIZE); //lower
		s_stEditR[6] = S_RECTMAKE(0,EBORDER_SIZE,EBORDER_SIZE,2);              //left
		s_stEditR[7] = S_RECTMAKE(w-EBORDER_SIZE,EBORDER_SIZE,EBORDER_SIZE,2); //right
		s_stEditR[8] = S_RECTMAKE(EBORDER_SIZE,EBORDER_SIZE,4,4);              //middle

		sprintf(szFile, "%s/%s", C_Global::szDataPath, "gui_edit.tga");
		s_pCheck[0]  = new C_Image(szFile, NULL);
		sprintf(szFile, "%s/%s", C_Global::szDataPath, "gui_checked.tga");
		s_pCheck[1]  = new C_Image(szFile, NULL); //must be same dimensions as 0
		s_pCheck[0]->GetInfo(&w, &h, NULL);
		s_stCheckR[0] = S_RECTMAKE(0,0,                          EBORDER_SIZE,EBORDER_SIZE); //u left corner
		s_stCheckR[1] = S_RECTMAKE(w-EBORDER_SIZE,0,             EBORDER_SIZE,EBORDER_SIZE); //u right corner
		s_stCheckR[2] = S_RECTMAKE(0,h-EBORDER_SIZE,             EBORDER_SIZE,EBORDER_SIZE); //l left corner
		s_stCheckR[3] = S_RECTMAKE(w-EBORDER_SIZE,h-EBORDER_SIZE,EBORDER_SIZE,EBORDER_SIZE); //l right corner
		s_stCheckR[4] = S_RECTMAKE(EBORDER_SIZE,0,2,EBORDER_SIZE);              //upper
		s_stCheckR[5] = S_RECTMAKE(EBORDER_SIZE,h-EBORDER_SIZE,2,EBORDER_SIZE); //lower
		s_stCheckR[6] = S_RECTMAKE(0,EBORDER_SIZE,EBORDER_SIZE,2);              //left
		s_stCheckR[7] = S_RECTMAKE(w-EBORDER_SIZE,EBORDER_SIZE,EBORDER_SIZE,2); //right
		s_stCheckR[8] = S_RECTMAKE(EBORDER_SIZE,EBORDER_SIZE,4,4);              //middle

		sprintf(szFile, "%s/%s", C_Global::szDataPath, "gui_list_scroll.tga");
		s_pListVBar  = new C_Image(szFile, NULL);
		s_stListVBarR[0] = S_RECTMAKE(0,0,  16,14); //up button
		s_stListVBarR[1] = S_RECTMAKE(0,30, 16,14); //down button
		s_stListVBarR[2] = S_RECTMAKE(0,14, 16,4);  //area
		s_stListVBarR[3] = S_RECTMAKE(0,20, 16,4);  //bar upper part
		s_stListVBarR[4] = S_RECTMAKE(0,24, 16,2);  //bar middle part
		s_stListVBarR[5] = S_RECTMAKE(0,26, 16,4);  //bar lower part

		s_pFont = new C_Font(_RGBA8888);
		sprintf(szFile, "%s/%s", C_Global::szDataPath, "arial-16");
		s_pFont->Load(szFile, NULL);
		s_iObjCount++;
	}

	m_pObj = new C_GUIObjects();
	m_pTimer = new C_Timer();

	m_dTime = m_dKeyTimer = 0.0;

	m_bLastMouseLeft = false;
	m_iLastMouseX = 0;
	m_iLastMouseY = 0;
}

C_GUI::~C_GUI()
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		delete (*m_pObj)[i].pListEl; //will be null if not used
	}
	delete m_pObj;
	delete m_pTimer;

	s_iObjCount--;
	if(s_iObjCount==0) {
		delete s_pFont;
		delete s_pButton[0];
		delete s_pButton[1];
		delete s_pCheck[0];
		delete s_pCheck[1];
		delete s_pEdit;
		delete s_pListVBar;
	}
}

bool C_GUI::LoadTemplate(char *szFile, char *szResource, int iScrOffsX, int iScrOffsY)
{
	char szLine[512], szCommand[128];

	bool bObjBegun = false;
	bool bColorSet[3] = {false,false,false};
	bool bTextColorSet = false;
	bool bSelectionColorSet = false;
	bool bMaxTextSet = false;
	S_GUIObject stObject;

	bool bResult = false;
	C_Resource *pResource = new C_Resource();
	if(szResource) {
		bResult = pResource->SetFilename(szFile, szResource);
	}
	if(!bResult) { //not in resource, try normal file
		char szFilename[MAX_PATH];
		sprintf(szFilename, "%s/%s", C_Global::szDataPath, szFile);
		bResult = pResource->SetFilename(szFile);
	}
	if(bResult) {
		int iSize = pResource->GetSize();
		char *szFile = new char[iSize+1];
		bResult = pResource->Read(szFile, iSize);
		szFile[iSize] = 0; //zero terminate
		if(bResult) {

			int iLineIndex = 0;
			while(C_Global::GetNextLineAndCommand(szFile+iLineIndex, szLine, szCommand, &iLineIndex)) {
				if(szCommand[0]!='*') continue;

				if(strcmp(szCommand, "*BACKGROUND") == 0) {
					memset(&stObject, 0, sizeof(stObject)); //reset to default
					stObject.iType = TYPE_BK;
					sscanf(szLine, "%s %d", szCommand, &stObject.iID);
					bObjBegun = true;
					continue;
				}

				if(strcmp(szCommand, "*BUTTON") == 0) {
					memset(&stObject, 0, sizeof(stObject)); //reset to default
					stObject.iType = TYPE_BUTTON;
					sscanf(szLine, "%s %d", szCommand, &stObject.iID);
					bObjBegun = true;
					continue;
				}

				if(strcmp(szCommand, "*EDIT") == 0) {
					memset(&stObject, 0, sizeof(stObject)); //reset to default
					stObject.iType = TYPE_EDIT;
					sscanf(szLine, "%s %d", szCommand, &stObject.iID);
					bObjBegun = true;
					continue;
				}

				if(strcmp(szCommand, "*CHECK") == 0) {
					memset(&stObject, 0, sizeof(stObject)); //reset to default
					stObject.iType = TYPE_CHECK;
					sscanf(szLine, "%s %d", szCommand, &stObject.iID);
					bObjBegun = true;
					continue;
				}

				if(strcmp(szCommand, "*LIST") == 0) {
					memset(&stObject, 0, sizeof(stObject)); //reset to default
					stObject.iType = TYPE_LIST;
					sscanf(szLine, "%s %d", szCommand, &stObject.iID);
					bObjBegun = true;
					continue;
				}

				if(strcmp(szCommand, "*STATIC") == 0) {
					memset(&stObject, 0, sizeof(stObject)); //reset to default
					stObject.iType = TYPE_STATIC;
					sscanf(szLine, "%s %d", szCommand, &stObject.iID);
					bObjBegun = true;
					continue;
				}

				//... more obj types here

				if(strcmp(szCommand, "*COLORDISABLED") == 0) {
					sscanf(szLine, "%s %f %f %f %f", szCommand, &stObject.stCol[STATE_DISABLED].r,
						&stObject.stCol[STATE_DISABLED].g, &stObject.stCol[STATE_DISABLED].b, &stObject.stCol[STATE_DISABLED].a);
					bColorSet[STATE_DISABLED] = true;
					continue;
				}
				if(strcmp(szCommand, "*COLORENABLED") == 0) {
					sscanf(szLine, "%s %f %f %f %f", szCommand, &stObject.stCol[STATE_ENABLED].r,
					&stObject.stCol[STATE_ENABLED].g, &stObject.stCol[STATE_ENABLED].b, &stObject.stCol[STATE_ENABLED].a);
					bColorSet[STATE_ENABLED] = true;
					continue;
				}
				if(strcmp(szCommand, "*COLORHIGHLIGHT") == 0) {
					sscanf(szLine, "%s %f %f %f %f", szCommand, &stObject.stCol[STATE_ACTIVE].r,
					&stObject.stCol[STATE_ACTIVE].g, &stObject.stCol[STATE_ACTIVE].b, &stObject.stCol[STATE_ACTIVE].a);
					bColorSet[STATE_ACTIVE] = true;
					continue;
				}
				if(strcmp(szCommand, "*COLORTEXT") == 0) {
					sscanf(szLine, "%s %f %f %f %f", szCommand, &stObject.stTextCol.r, &stObject.stTextCol.g,
					&stObject.stTextCol.b, &stObject.stTextCol.a);
					bTextColorSet = true;
					continue;
				}
				if(strcmp(szCommand, "*COLORSELECTION") == 0) {
					sscanf(szLine, "%s %f %f %f %f", szCommand, &stObject.stSelectCol.r, &stObject.stSelectCol.g,
					&stObject.stSelectCol.b, &stObject.stSelectCol.a);
					bSelectionColorSet = true;
					continue;
				}

				if(strcmp(szCommand, "*RECT") == 0) {
					sscanf(szLine, "%s %d %d %d %d", szCommand, &stObject.stRect.x, &stObject.stRect.y,
					&stObject.stRect.width, &stObject.stRect.height);
					stObject.stRect.x += iScrOffsX;
					stObject.stRect.y += iScrOffsY;
					continue;
				}

				if(strcmp(szCommand, "*TEXT") == 0) {
					C_Global::GetWithin(szLine, '"');
					strcpy(stObject.szText, szLine);
					continue;
					}
				if(strcmp(szCommand, "*MAXTEXT") == 0) {
					sscanf(szLine, "%s %d", szCommand, &stObject.iMaxText);
					bMaxTextSet = true;
					continue;
				}

				if(strcmp(szCommand, "*ONLYNUMBERS") == 0) {
					sscanf(szLine, "%s %d", szCommand, &stObject.bOnlyNumbers);
					continue;
				}
				if(strcmp(szCommand, "*ONLYEMAIL") == 0) {
					sscanf(szLine, "%s %d", szCommand, &stObject.bOnlyEmail);
					continue;
				}
				if (strcmp(szCommand, "*PWMASK") == 0) {
					sscanf(szLine, "%s %d", szCommand, &stObject.bIsPassWord);
					continue;
				}

				if(strcmp(szCommand, "*STICKY") == 0) {
					sscanf(szLine, "%s %d", szCommand, &stObject.bSticky);
					continue;
				}

				if(strcmp(szCommand, "*CHECKED") == 0) {
					sscanf(szLine, "%s %d", szCommand, &stObject.bChecked);
					continue;
				}

				if(strcmp(szCommand, "*COLUMNS") == 0) {
					sscanf(szLine, "%s %d %d %d %d %d", szCommand, &stObject.iListColumns,
					&stObject.iListColumnW[0], &stObject.iListColumnW[1], &stObject.iListColumnW[2], &stObject.iListColumnW[3]);
					stObject.iLastColumnWOriginal = stObject.iListColumnW[stObject.iListColumns-1]; //saved for vertical scrollbar mod
					continue;
				}

				if(strcmp(szCommand, "*SELECTION") == 0) {
					int iValue;
					sscanf(szLine, "%s %d", szCommand, &iValue);
					stObject.bListHasSelection = (iValue!=0);
					continue;
				}

				//must be last info in current object
				if(strcmp(szCommand, "*STATE") == 0) {
					sscanf(szLine, "%s %d", szCommand, &stObject.iState);

					for(int i=0; i<3; i++) {
						if(!bColorSet[i]) {
							stObject.stCol[i] = s_stStateCol[i];
						}
					}
					if(!bTextColorSet) {
						stObject.stTextCol = s_stTextColor;
					}
					if(!bSelectionColorSet) {
						stObject.stSelectCol = s_stSelectCol;
					}
					if(!bMaxTextSet) {
						stObject.iMaxText = MAX_TEXT;
					}

					//whole object read
					if(bObjBegun) {
						m_pObj->push_back(stObject);
					}
					bObjBegun = false;
					bColorSet[0] = bColorSet[1] = bColorSet[2] = false;
					bTextColorSet = bSelectionColorSet = false;
					bMaxTextSet = false;
					continue;
				}

			}
		}
		delete[] szFile;
	}
	delete pResource;
	m_iScrOffsX = iScrOffsX;
	m_iScrOffsY = iScrOffsY;

	return bResult;
}

bool C_GUI::SetPos(int iScrOffsX, int iScrOffsY)
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		pstO->stRect.x = (pstO->stRect.x-m_iScrOffsX) + iScrOffsX;
		pstO->stRect.y = (pstO->stRect.y-m_iScrOffsY) + iScrOffsY;
	}

	m_iScrOffsX = iScrOffsX;
	m_iScrOffsY = iScrOffsY;
	return true;
}

//modifications through user input
void C_GUI::UpdateInput()
{
	m_dTime = m_pTimer->GetInterval();
	m_dKeyTimer += m_dTime;

	int iCurMouseX, iCurMouseY;
	m_pInput->GetMousePos(&iCurMouseX, &iCurMouseY);
	if (m_iLastMouseX != iCurMouseX || m_iLastMouseY != iCurMouseY) {
		m_iLastMouseX = iCurMouseX;
		m_iLastMouseY = iCurMouseY;
		MouseMove(iCurMouseX, iCurMouseY);
	}
	bool bCurMouseLeft = m_pInput->MouseIsPressed(0);
	if (bCurMouseLeft != m_bLastMouseLeft) {
		if(bCurMouseLeft) MouseDown(iCurMouseX, iCurMouseY);
		else MouseUp(iCurMouseX, iCurMouseY);
		m_bLastMouseLeft = bCurMouseLeft;
	}
	int iCurWheelEvent = m_pInput->GetLastWheelEvent();
	if (iCurWheelEvent == EVT_MOUSE_WHEEL_UP) MouseWheel(-1);
	if (iCurWheelEvent == EVT_MOUSE_WHEEL_DOWN) MouseWheel(1);

	int iDpadMod = 0;
	if(m_dKeyTimer>85) {
		m_dKeyTimer = 0;

		//check all list objects for buttons pressed
		int i, iSize = (int)m_pObj->size();
		for(i=0; i<iSize; i++) {
			S_GUIObject *pstO = &(*m_pObj)[i];

			if(pstO->iType == TYPE_LIST && pstO->bListHasSelection) {
				if(pstO->iListVBarStatus & 0x08) ListModSelection(-1, pstO->iID);
				if(pstO->iListVBarStatus & 0x10) ListModSelection(1, pstO->iID);
			}
		}

		//
		S_GamepadState stPad;
		m_pInput->GetGamepadState(&stPad);
		iDpadMod = (stPad.iDpad & 0x001) ? 1 : 0;
		iDpadMod = (stPad.iDpad & 0x002) ? -1 : iDpadMod;
	}

	if((m_pInput->IsPressedRepeat(DIK_UP) && !m_pInput->IsPressed(DIK_DOWN)) || iDpadMod==1) {
		ListModSelection(-1);
	}
	if((m_pInput->IsPressedRepeat(DIK_DOWN) && !m_pInput->IsPressed(DIK_UP)) || iDpadMod==-1) {
		ListModSelection(1);
	}
	if(m_pInput->IsPressedRepeat(DIK_PRIOR) && !m_pInput->IsPressed(DIK_NEXT)) {
		ListModSelection(-4);
	}
	if(m_pInput->IsPressedRepeat(DIK_NEXT) && !m_pInput->IsPressed(DIK_PRIOR)) {
		ListModSelection(4);
	}

	if(m_pInput->IsPressedRepeat(DIK_DELETE)) {
		ListDelSelection();
	}

	if(((m_pInput->IsPressed(DIK_LCONTROL) || m_pInput->IsPressed(DIK_LWIN) || m_pInput->IsPressed(DIK_LMENU)) && m_pInput->IsPressedOnce(DIK_V)) ||
		((m_pInput->IsPressed(DIK_LSHIFT) || m_pInput->IsPressed(DIK_RSHIFT)) && m_pInput->IsPressedOnce(DIK_INSERT))	)
	{
		//paste
		char szText[128];
		char *szTextPos = szText;
		if(C_Global::GetClipboardBuffer(szText, sizeof(szText))) {
			while(*szTextPos!=0) {
				EnterChar(*szTextPos++);
			}
		}
		return;
	}

	int c = GetInput();
	if(c==-1 || c==9) return;
	EnterChar(c);
}

int C_GUI::GetInput()
{
	for(uint8_t i=1; i<220; i++) {
		if(m_pInput->IsPressedRepeat(i)) {
			int iAscii = m_pInput->GetAscii(i);
			if(iAscii!=-1) {
				return (unsigned char)iAscii;
			}
		}
	}
	return -1;
}

void C_GUI::MouseMove(int x, int y)
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(x>=pstO->stRect.x && x<=(pstO->stRect.x+pstO->stRect.width)
			&& y>=pstO->stRect.y && y<=(pstO->stRect.y+pstO->stRect.height))
		{
			//within rect
			switch(pstO->iType) {
				case TYPE_BUTTON:
				case TYPE_CHECK:
				case TYPE_EDIT:
					if(pstO->iState!=STATE_DISABLED) {
						pstO->iState = STATE_ACTIVE;
					}
					break;
				case TYPE_LIST:
					if(pstO->iState!=STATE_DISABLED) {
						pstO->iState = STATE_ACTIVE;
					}

					pstO->iListVBarStatus &= 0x01+0x08+0x10+0x40; //assume reset
					if(pstO->iListVBarStatus&0x01) {
						//list shows scrollbar
               
						if(x>=pstO->stRect.x+pstO->stRect.width-18 && x<=pstO->stRect.x+pstO->stRect.width-4) {
							//within bar
							pstO->iListVBarStatus |= (y<=pstO->stRect.y+18) ? 0x02 : 0; //up
							pstO->iListVBarStatus |= (y>=pstO->stRect.y+pstO->stRect.height-18) ? 0x04 : 0; //down

							pstO->iListVBarStatus |= (y>=pstO->iBarPosY && y<pstO->iBarPosY+pstO->iBarHeight) ? 0x20 : 0; //scroller
						}

						if(pstO->iListVBarStatus & 0x40) { //scroller moved
							int iAllRows = (int)pstO->pListEl->size(); //int iAllRows = ListGetNumRowsUntil(pstO->iID, 1000000);
							int iAreaHeight = pstO->stRect.height-36;
							double dNumPixPerRow = (double)(iAreaHeight-pstO->iBarHeight)/(double)iAllRows;

							int iNewSel = pstO->iListClickedSelection + (int)((y-pstO->iListClickedY)/dNumPixPerRow);
							ListModSelection(iNewSel - pstO->iListSelection, pstO->iID);
						}
					}
					break;
			}
		} else {
			//not within rect
			switch(pstO->iType) {
				case TYPE_BUTTON:
				case TYPE_CHECK:
					if(pstO->iState == STATE_ACTIVE) {
						pstO->iState = STATE_ENABLED;
					}
					if(!pstO->bSticky) pstO->bPressed = false;
					break;
				case TYPE_EDIT:
				case TYPE_LIST:
					if(pstO->iState == STATE_ACTIVE && !pstO->bPressed) {
						pstO->iState = STATE_ENABLED;
					}
					pstO->iListVBarStatus &= 0x1;
					break;
			}
		}
	}
}

void C_GUI::MouseDown(int x, int y)
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(x>=pstO->stRect.x && x<=(pstO->stRect.x+pstO->stRect.width)
			&& y>=pstO->stRect.y && y<=(pstO->stRect.y+pstO->stRect.height))
		{
			if(pstO->iState!=STATE_DISABLED) {
				switch(pstO->iType) {
					case TYPE_BUTTON:
						pstO->iState = STATE_ACTIVE;
						if(pstO->bSticky) {
							pstO->bPressed = !pstO->bPressed;
							pstO->bChecked = pstO->bPressed;
						}
						else pstO->bPressed = true;
						break;
					case TYPE_CHECK:
						pstO->iState = STATE_ACTIVE;
						pstO->bPressed = true;
						break;
					case TYPE_LIST:
						if(pstO->bListHasSelection && pstO->pListEl) {
							pstO->iState = STATE_ACTIVE;
							pstO->bPressed = true;

							//modify selection
							if((pstO->iListVBarStatus&0x01)==0 || x<pstO->stRect.x+pstO->stRect.width-16) {
								if(y>=(pstO->stRect.y+EBORDER_SIZE) && y<=((pstO->stRect.y+pstO->stRect.height)-EBORDER_SIZE)) {
									int iSize = (int)pstO->pListEl->size();
									int iNumToDraw = (pstO->stRect.height-EBORDER_SIZE*2)/16;
									if(iNumToDraw>iSize) iNumToDraw = iSize;

									int iOldSel = pstO->iListSelection;

									int iSelRow = (y-(pstO->stRect.y+EBORDER_SIZE))/16;
									int iFirstRow = ListGetNumRowsUntil(pstO->iID, pstO->iListTop);
									int iSelRow2 = iFirstRow, i = 0;
									while(iSelRow2-iFirstRow<=iSelRow) {
										i++;
										iSelRow2 = ListGetNumRowsUntil(pstO->iID, pstO->iListTop+i);
										if(pstO->iListTop+i>=iSize) break;
									} 
									pstO->iListSelection = pstO->iListTop+(i-1);

									if(iOldSel != pstO->iListSelection) pstO->bChanged = 1;
								}
							}

							//scrollbar
							pstO->iListVBarStatus &= 0x01; //assume reset
							if(pstO->iListVBarStatus&0x01) {
								//list shows scrollbar
								if(x>=pstO->stRect.x+pstO->stRect.width-18 && x<=pstO->stRect.x+pstO->stRect.width-4) {
									//within bar
									pstO->iListVBarStatus |= (y<=pstO->stRect.y+18) ? 0x02+0x08 : 0; //up
									pstO->iListVBarStatus |= (y>=pstO->stRect.y+pstO->stRect.height-18) ? 0x04+0x10 : 0; //down

									pstO->iListVBarStatus |= (y>=pstO->iBarPosY && y<pstO->iBarPosY+pstO->iBarHeight) ? 0x20+0x40 : 0; //scroller
									pstO->iListClickedY = y;
									pstO->iListClickedSelection = pstO->iListSelection;
								}
							}
						}
						break;
					case TYPE_EDIT:
						pstO->iState = STATE_ACTIVE;
						pstO->bPressed = true;
						break;
				}
			}
		} else { //not within rect
			if(pstO->iState == STATE_ACTIVE) {
				pstO->iState = STATE_ENABLED;
			}
			switch(pstO->iType) {
				case TYPE_BUTTON:
					if(pstO->bSticky) break;
				default:
					pstO->bPressed = false;
			}
		}
	}
}

void C_GUI::MouseUp(int x, int y)
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(x>=pstO->stRect.x && x<=(pstO->stRect.x+pstO->stRect.width)
			&& y>=pstO->stRect.y && y<=(pstO->stRect.y+pstO->stRect.height))
		{
			if(pstO->iState!=STATE_DISABLED) {
				switch(pstO->iType) {
					case TYPE_BUTTON:
						if(pstO->bPressed) {
							//pressed for real, report to user
							pstO->bClicked = true;
						}
						if(pstO->bSticky) pstO->bChanged = 1;
						break;
					case TYPE_CHECK:
						if(pstO->bPressed) {
							pstO->bChecked = !pstO->bChecked; //toggle
							pstO->bChanged = 1;
						}
						break;
				}
			}
		}

		switch(pstO->iType) {
			case TYPE_BUTTON:
				if(pstO->iState != STATE_DISABLED) {
					if(!pstO->bSticky) {
						pstO->bPressed = false;
					}
				}
				break;
			case TYPE_LIST:
				pstO->iListVBarStatus &= 0x01+0x02+0x04; //reset
				break;
		}
	}
}

void C_GUI::MouseWheel(int iWay)
{
	ListModSelection(iWay, -1);
}

void C_GUI::EnterChar(int c)
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(pstO->iState == STATE_ACTIVE && pstO->iType == TYPE_EDIT && pstO->bPressed) {
			int iLen = (int)strlen(pstO->szText);
			if(c==8) {
				if(iLen>0) {
					pstO->szText[iLen-1] = 0;
					pstO->bChanged = 1;
				}
			} else if(c==13) {
				pstO->iState = STATE_ENABLED;
				pstO->bPressed = false;
				pstO->bClicked = true;
			} else if(iLen<(pstO->iMaxText-1)) {
				if(pstO->bOnlyEmail) {
					if((c<'0' || c>'9') && (c<'A' || c>'Z') && (c<'a' || c>'z') && c!='.' && c!='_' && c!='-' && c!='@' && c!=32) break;
				}
				if((pstO->bOnlyNumbers && c>='0' && c<='9') || (!pstO->bOnlyNumbers && c>31)) {
					pstO->szText[iLen] = (char)c;
					pstO->szText[iLen+1] = 0;
					pstO->bChanged = 1;
				}
			}
			break;
		}
	}
}

void C_GUI::ListModSelection(int iWay, int iID)
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(pstO->iType == TYPE_LIST && pstO->bListHasSelection) {
			if((iID == -1 && pstO->iState == STATE_ACTIVE /* && pstO->bPressed*/) || (pstO->iID == iID)) {
				int iSize2 = 0;
				if(pstO->pListEl) iSize2 = (int)pstO->pListEl->size();

				int iFirstRow = ListGetNumRowsUntil(pstO->iID, pstO->iListTop);

				int iNumToDraw = (pstO->stRect.height-EBORDER_SIZE*2)/16;
				int iOldSel = pstO->iListSelection;

				//modify and clip it
				pstO->iListSelection += iWay;
				if(pstO->iListSelection<0 || iSize2==0) pstO->iListSelection = 0;
				else if(pstO->iListSelection>=iSize2) pstO->iListSelection = iSize2-1;

				int iSelRow = ListGetNumRowsUntil(pstO->iID, pstO->iListSelection+1);

				//modify drawing area
				if(iSelRow-iFirstRow>iNumToDraw) {
					do {
						iFirstRow = ListGetNumRowsUntil(pstO->iID, ++pstO->iListTop);
					} while(iSelRow-iFirstRow>iNumToDraw);
				}
				else if(pstO->iListSelection<pstO->iListTop && pstO->iListSelection>=0) {
					pstO->iListTop = pstO->iListSelection;
				}

				if(iOldSel!=pstO->iListSelection) pstO->bChanged = 1;
				if(iSize==0) pstO->bChanged = 1;

				break;
			}
		}
	}
}

int C_GUI::ListGetNumRowsUntil(int iListId, int iLast)
{
	S_GUIObject *pCon = GetObject(iListId);

	int iNumRows = 0;
	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->pListEl) {
			int i, j, iSize2 = (int)pCon->pListEl->size();
			if(iLast<iSize2) iSize2 = iLast;
			for(j=0; j<iSize2; j++) {
				S_GUIListEl *pEl = &(*pCon->pListEl)[j];

				int iMaxRows = 1;
				for(i=0; i<pCon->iListColumns; i++) {
					if(pEl->szText[i] != 0) {
						int iRows = ListGetElRows(pCon->iListColumnW[i+(pEl->pIcon ? 1 : 0)], pEl->szText[i]);
						if(iRows>iMaxRows) iMaxRows = iRows;
					}
				}
				iNumRows += iMaxRows;
			}
		}
	}

	return iNumRows;
}

int C_GUI::ListGetElRows(int iWidth, char *szText)
{
	int iNumRows = 0;

	int iStart = 0, iNum = 2;
	int iLen;
	bool bContinue = true;
	while(bContinue) {
		strncpy(m_aszRows[iNumRows], &szText[iStart], iNum);
		m_aszRows[iNumRows][iNum] = 0; //null terminate
		iLen = (int)strlen(m_aszRows[iNumRows]);

		if(GetTextWidth(m_aszRows[iNumRows]) >iWidth || iLen<iNum) {
			//remove last char and then search to last space if it exists, line finished

			int iAdd = 0;
			if(iLen<iNum) {
				//last line
				bContinue = false;
				iNum--;
			} else {
				char *lastspace = strrchr(m_aszRows[iNumRows], ' ');
				if(lastspace==NULL) iNum--;
				else {
					iNum= (uint32_t)(lastspace-m_aszRows[iNumRows]);
					iAdd = 1;
				}
			}

			m_aszRows[iNumRows][iNum] = 0; //null terminate
			iStart += (int)strlen(m_aszRows[iNumRows])+iAdd;
			iNumRows++;
			iNum=1; //prepared for next line
		}

		iNum++;
	}

	return iNumRows;
}

void C_GUI::ListDelSelection(int iID)
{
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(pstO->iType == TYPE_LIST && pstO->bListHasSelection) {
			if((iID == -1 && pstO->iState == STATE_ACTIVE && pstO->bPressed) || (pstO->iID == iID)) {
				if(pstO->pListEl) {
					pstO->bDeletePressed = 1; //delete pressed for selection
				}
				break;
			}
		}
	}
}


//render
void C_GUI::Draw()
{
	//pass 1
	int i, iSize = (int)m_pObj->size();
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(pstO->iType==TYPE_BK) {
			DrawBorderedRect(&pstO->stRect, &pstO->stCol[pstO->iState], s_pEdit, &s_stEditR[0], EBORDER_SIZE);
		}
	}
	//pass 2
	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		switch(pstO->iType) {
			case TYPE_BUTTON:
				{
					DrawBorderedRect(&pstO->stRect, &pstO->stCol[pstO->iState], s_pButton[pstO->bPressed?1:0], &s_stButtonR[0], BBORDER_SIZE);
					if(pstO->szText[0]!=0) {
						int iW, iH1, iH2;
						s_pFont->GetTextSize(pstO->szText, &iW, NULL, &iH1, &iH2);
						s_pFont->SetColor(pstO->stTextCol);
						s_pFont->Print(pstO->stRect.x+((pstO->stRect.width-iW)/2), pstO->stRect.y+((pstO->stRect.height-iH2)/2-iH1), pstO->szText);
					}
				}
				break;
			case TYPE_CHECK:
				{
					DrawBorderedRect(&pstO->stRect, &pstO->stCol[pstO->iState], s_pCheck[0], &s_stCheckR[0], EBORDER_SIZE);
					if(pstO->bChecked) m_pGraph->Blt(s_pCheck[1], NULL, &pstO->stRect, true);
				}
				break;
			case TYPE_EDIT:
				{
					DrawBorderedRect(&pstO->stRect, &pstO->stCol[pstO->iState], s_pEdit, &s_stEditR[0], EBORDER_SIZE);

					int j = 0;
					char szString[256];
					strcpy(szString, pstO->szText);
					if(pstO->bIsPassWord && ((strcmp(szString, "Password") != 0) && (strcmp(szString, "Last Password") != 0))) {
						while(szString[j] != 0) szString[j++] = '*'; //hide password, special case not to hide it if='Password' or 'Last password'
					}
					if(pstO->bPressed) strcat(szString, "_");
					int iW, iBeginChar = 0;
					s_pFont->GetTextSize(szString, &iW);
					int iLen = (int)strlen(szString);
					while(iW>pstO->stRect.width-2*EBORDER_SIZE) {
						if(iBeginChar>=iLen-1) break;
						iBeginChar++;
						s_pFont->GetTextSize(&szString[iBeginChar], &iW);
					}

					s_pFont->SetColor(pstO->stTextCol);
					s_pFont->Print(pstO->stRect.x+EBORDER_SIZE+2, pstO->stRect.y+EBORDER_SIZE+2, &szString[iBeginChar]);
				}
				break;
			case TYPE_LIST:
				{
					DrawBorderedRect(&pstO->stRect, &pstO->stCol[pstO->iState], s_pEdit, &s_stEditR[0], EBORDER_SIZE);

					if(pstO->pListEl && !pstO->pListEl->empty()) {
						int iNumRows = (pstO->stRect.height-EBORDER_SIZE*2)/16;

						//draw vscrollbar
						int iAllRows = ListGetNumRowsUntil(pstO->iID, 1000000);
						bool bShowScrollbar = iAllRows>iNumRows;
						pstO->iListVBarStatus |= bShowScrollbar ? 1 : 0; //shown
						if(bShowScrollbar) {
							int iSelectPos = ListGetNumRowsUntil(pstO->iID, pstO->iListSelection+1);

							int iAreaHeight = pstO->stRect.height-36;
							int iBarHeight  = (int)(((double)iNumRows/(double)iAllRows) * iAreaHeight);
							if(iBarHeight<8) iBarHeight=8;
							int iBarPosY    = pstO->stRect.y+18+(int)(((double)iSelectPos/(double)iAllRows) * (iAreaHeight-iBarHeight));

							pstO->iBarHeight = iBarHeight;
							pstO->iBarPosY   = iBarPosY;

							int iBarX = pstO->stRect.x+pstO->stRect.width-20;
							S_Rect stRect = S_RECTMAKE(iBarX, pstO->stRect.y+18, 16, iAreaHeight);
							if(pstO->iState != STATE_DISABLED)
								m_pGraph->SetBltColor(&pstO->stCol[(pstO->iListVBarStatus & 0x2) !=0 ? STATE_ACTIVE : STATE_ENABLED]);
							m_pGraph->Blt(s_pListVBar, &s_stListVBarR[0], iBarX, pstO->stRect.y+4, true); //up button
							if(pstO->iState != STATE_DISABLED)
								m_pGraph->SetBltColor(&pstO->stCol[(pstO->iListVBarStatus & 0x4) !=0 ? STATE_ACTIVE : STATE_ENABLED]);
							m_pGraph->Blt(s_pListVBar, &s_stListVBarR[1], iBarX, pstO->stRect.y+pstO->stRect.height-18, true); //down button
							if(pstO->iState != STATE_DISABLED)
								m_pGraph->SetBltColor(&pstO->stCol[STATE_ENABLED]);
							m_pGraph->Blt(s_pListVBar, &s_stListVBarR[2], &stRect, true); //area
							if(pstO->iState != STATE_DISABLED)
								m_pGraph->SetBltColor(&pstO->stCol[(pstO->iListVBarStatus & 0x20) !=0 ? STATE_ACTIVE : STATE_ENABLED]);
							m_pGraph->Blt(s_pListVBar, &s_stListVBarR[3], iBarX, iBarPosY, true); //bar upper part
							m_pGraph->Blt(s_pListVBar, &s_stListVBarR[5], iBarX, iBarPosY+iBarHeight-4, true); //bar lower part
							if(iBarHeight>8) {
								//draw middle part
								stRect = S_RECTMAKE(iBarX, iBarPosY+4, 16, iBarHeight-8);
								m_pGraph->Blt(s_pListVBar, &s_stListVBarR[4], &stRect, true); //bar middle part
							}
						}
						pstO->iListColumnW[pstO->iListColumns-1]=pstO->iLastColumnWOriginal-(bShowScrollbar?16:0);

						//draw all elements
						s_pFont->SetColor(pstO->stTextCol);
						S_GUIListEl *pEl;
						int iElNum = pstO->iListTop;
						int j, iSize2 = (int)pstO->pListEl->size();
						int y = pstO->stRect.y+EBORDER_SIZE;
						for(j=0; j<iNumRows; j++) {
							if(iElNum>=iSize2 || iElNum<0) break;
							pEl = &(*pstO->pListEl)[iElNum];
							int iNum = pstO->iListColumns;
							int posx = 0, iListCol = 0;
							//m_pGraph->SetBltColor(&pstO->stCol[pstO->iState]); //set back blt color, Print() sets it to NULL
							m_pGraph->SetBltColor(NULL); //keep blt color at NULL

							if(pEl->pIcon) {
								//draw icon in column 0
								S_Rect stSrcRect = {pEl->iIconRect*16, 0, 16, 16};
								m_pGraph->Blt(pEl->pIcon, &stSrcRect, pstO->stRect.x+EBORDER_SIZE+posx, pstO->stRect.y+EBORDER_SIZE+j*16, true);
								posx = pstO->iListColumnW[0];
								iNum--; iListCol = 1;
							}

							int iYAdd = 16;
							for(int k=0; k<iNum; k++) {
								//draw text columns
								if(pEl->szText[k][0]!=0) {
									int x = pstO->stRect.x+EBORDER_SIZE+1+posx;
									int iNumRows2 = ListGetElRows(pstO->iListColumnW[iListCol], pEl->szText[k]);
									if(iNumRows2>1) iNumRows2 = iNumRows2;
									for(int l=0; l<iNumRows2; l++) {
										if(l>0) {
											j++;
											if(j>=iNumRows) break;
										}
										s_pFont->Print(x, y+(l*16), m_aszRows[l]);
									}
									if(iYAdd<iNumRows2*16) iYAdd = iNumRows2*16;
								}
								posx += pstO->iListColumnW[iListCol++];
							}
							y+=iYAdd;
							iElNum++;
						}
						//draw selection mark
						if(pstO->bListHasSelection) {
							int iSel = ListGetNumRowsUntil(pstO->iID, pstO->iListSelection);
							int y2 = (iSel - ListGetNumRowsUntil(pstO->iID, pstO->iListTop))*16+EBORDER_SIZE;
							int h2 = (ListGetNumRowsUntil(pstO->iID, pstO->iListSelection+1)-iSel)*16;
							S_Rect stRect = S_RECTMAKE(pstO->stRect.x+EBORDER_SIZE, pstO->stRect.y+y2, pstO->stRect.width-EBORDER_SIZE*2-(bShowScrollbar*16), h2);
							m_pGraph->SetColor(&pstO->stSelectCol);
							m_pGraph->Rect(&stRect, true);
						}
					}
				}
				break;
			case TYPE_STATIC:
				{
					s_pFont->SetColor(pstO->stTextCol);
					s_pFont->Print(pstO->stRect.x, pstO->stRect.y-12, pstO->szText);
				}
				break;
		}
	}

	m_pGraph->SetBltColor(NULL);
}

void C_GUI::DrawBorderedRect(S_Rect *pDstRect, S_FColor *pModColor, C_Image *pSrcImage, S_Rect *pSrcRects, int iBorderSize)
{
	//dst rects
	int w  = pDstRect->width -2*iBorderSize; //inner width
	int h  = pDstRect->height-2*iBorderSize; //inner height
	int w2 = pDstRect->width -iBorderSize;
	int h2 = pDstRect->height-iBorderSize;

	S_Rect dstRectW1 = S_RECTMAKE(pDstRect->x+iBorderSize,  pDstRect->y,                 w,iBorderSize);
	S_Rect dstRectW2 = S_RECTMAKE(pDstRect->x+iBorderSize,  pDstRect->y+h+iBorderSize, w,iBorderSize);
	S_Rect dstRectH1 = S_RECTMAKE(pDstRect->x,                pDstRect->y+iBorderSize,   iBorderSize,h);
	S_Rect dstRectH2 = S_RECTMAKE(pDstRect->x+w+iBorderSize,pDstRect->y+iBorderSize,   iBorderSize,h);
	S_Rect dstRectM  = S_RECTMAKE(pDstRect->x+iBorderSize,  pDstRect->y+iBorderSize,   w,h);

	//draw
	m_pGraph->SetBltColor(pModColor);
	m_pGraph->Blt(pSrcImage, &pSrcRects[0], pDstRect->x,    pDstRect->y, true);
	m_pGraph->Blt(pSrcImage, &pSrcRects[1], pDstRect->x+w2, pDstRect->y, true);
	m_pGraph->Blt(pSrcImage, &pSrcRects[2], pDstRect->x,    pDstRect->y+h2, true);
	m_pGraph->Blt(pSrcImage, &pSrcRects[3], pDstRect->x+w2, pDstRect->y+h2, true);

	m_pGraph->Blt(pSrcImage, &pSrcRects[4], &dstRectW1, true);
	m_pGraph->Blt(pSrcImage, &pSrcRects[5], &dstRectW2, true);
	m_pGraph->Blt(pSrcImage, &pSrcRects[6], &dstRectH1, true);
	m_pGraph->Blt(pSrcImage, &pSrcRects[7], &dstRectH2, true);

	m_pGraph->Blt(pSrcImage, &pSrcRects[8], &dstRectM,  true);
}



//gui control
S_GUIObject *C_GUI::GetObject(int iId)
{
	int i, iSize = (int)m_pObj->size();

	for(i=0; i<iSize; i++) {
		S_GUIObject *pstO = &(*m_pObj)[i];

		if(pstO->iID == iId) {
			return pstO;
		}
	}
	return NULL;
}

void C_GUI::SetText(int iId, char *i_szText)
{
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		if(strcmp(pCon->szText, i_szText)!=0) {
			strcpy(pCon->szText, i_szText);
			if(pCon->iType == TYPE_EDIT) pCon->bChanged = 1; //has changed
		}
	}
}

void C_GUI::GetText(int iId, char *o_szText)
{
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		strcpy(o_szText, pCon->szText);
	}
}

bool C_GUI::IsChecked(int iId)
{
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		return pCon->bChecked!=0;
	}
	return false;
}

bool C_GUI::WasClicked(int iId)
{
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		if(pCon->bClicked!=0) {
			pCon->bClicked = 0; //reset
			return true;
		}
	}
	return false;
}

void C_GUI::ListAddElement(int iListId, S_GUIListEl *pListEl)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST) {
			if(!pCon->pListEl) {
				pCon->pListEl = new C_GUIListElements();
			}
			pCon->pListEl->push_back(*pListEl);
			pCon->bChanged = 1;
			ListModSelection(0, iListId);
		}
	}
}

void C_GUI::ListRemoveElementById(int iListId, int iListEl)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->pListEl) {
			int j, iSize2 = (int)pCon->pListEl->size();
			for(j=0; j<iSize2; j++) {
				S_GUIListEl *pEl = &(*pCon->pListEl)[j];
				if(pEl->iID == iListEl) {
					pCon->pListEl->erase(pCon->pListEl->begin()+j);
					ListModSelection(0, iListId);
					return;
				}
			}
		}
	}
}

void C_GUI::ListRemoveElement(int iListId, char *szListText0)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->pListEl) {
			int j, iSize2 = (int)pCon->pListEl->size();
			for(j=0; j<iSize2; j++) {
				S_GUIListEl *pEl = &(*pCon->pListEl)[j];
				if(strcmp(pEl->szText[0], szListText0)==0) {
					pCon->pListEl->erase(pCon->pListEl->begin()+j);
					ListModSelection(0, iListId);
					return;
				}
			}
		}
	}
}

void C_GUI::ListRemoveAll(int iListId)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->pListEl) {
			pCon->pListEl->clear();
			pCon->iListSelection = 0;
			pCon->iListTop = 0;
			pCon->bChanged = 1;
			return;
		}
	}
}

S_GUIListEl *C_GUI::ListGetElementById(int iListId, int iListEl)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->pListEl) {
			int j, iSize2 = (int)pCon->pListEl->size();
			for(j=0; j<iSize2; j++) {
				S_GUIListEl *pEl = &(*pCon->pListEl)[j];
				if(pEl->iID == iListEl) {
					return pEl;
				}
			}
		}
	}
	return NULL;
}

S_GUIListEl *C_GUI::ListGetElement(int iListId, char *szListText0)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->pListEl) {
			int j, iSize2 = (int)pCon->pListEl->size();
			for(j=0; j<iSize2; j++) {
				S_GUIListEl *pEl = &(*pCon->pListEl)[j];
				if(strcmp(pEl->szText[0], szListText0)==0) {
					return pEl;
				}
			}
		}
	}
	return NULL;
}

void C_GUI::ListSetSelectedElement(int iListId, char *szListText0)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->pListEl) {
			int iPos = 0;
			int j, iSize2 = (int)pCon->pListEl->size();
			for(j=0; j<iSize2; j++) {
				S_GUIListEl *pEl = &(*pCon->pListEl)[j];
				if(szListText0==NULL) {
					ListModSelection(1000000, iListId);
					return;
				} else if(strcmp(pEl->szText[0], szListText0)==0) {
					ListModSelection(iPos - pCon->iListSelection, iListId);
					return;
				}
				iPos++;
			}
		}
	}
}

void C_GUI::ListSetSelectedElement(int iListId, int iListIndex)
{
	S_GUIObject* pCon = GetObject(iListId);

	if (pCon) {
		if (pCon->iType == TYPE_LIST && pCon->pListEl) {
			int j, iSize2 = (int)pCon->pListEl->size();
			for (j = 0; j < iSize2; j++) {
				if (iListIndex == -1) {
					ListModSelection(1000000, iListId);
					return;
				}
				else if (j == iListIndex) {
					ListModSelection(j, iListId);
					return;
				}
			}
		}
	}
}

S_GUIListEl *C_GUI::ListGetSelectedElement(int iListId, int *o_iId)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST) {
			C_GUIListElements *pEl = pCon->pListEl;
			if(!pEl || pCon->iListSelection>=(int)pEl->size()) return NULL;
			if(o_iId) *o_iId = pCon->iListSelection;
			return &(*pEl)[pCon->iListSelection];
		}
	}
	return NULL;
}

void C_GUI::ListSetHasSelection(int iListId, bool bHasSelection)
{
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST) {
			pCon->bListHasSelection = bHasSelection;
			if(!bHasSelection) {
				pCon->iListSelection = 0;
				pCon->iListTop = 0;
			}
		}
	}
}

bool C_GUI::ListWasDeletePressed(int iListId)
{
	bool bDelPressed = false;
	S_GUIObject *pCon = GetObject(iListId);

	if(pCon) {
		if(pCon->iType == TYPE_LIST && pCon->bListHasSelection) {
			if((iListId == -1 && pCon->iState == STATE_ACTIVE && pCon->bPressed) || (pCon->iID == iListId)) {
				if(pCon->pListEl) {
					bDelPressed = pCon->bDeletePressed!=0;
					pCon->bDeletePressed = 0; //reset
				}
			}
		}
	}
	return bDelPressed;
}

void C_GUI::SetActive(int iId, bool bActive)
{
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		if(pCon->iState == STATE_DISABLED && bActive) {
			pCon->iState = STATE_ENABLED;
		} else if(pCon->iState != STATE_DISABLED && !bActive) {
			pCon->iState = STATE_DISABLED;
			//pCon->bPressed = false;
		}
	}
}

void C_GUI::SetFocused(int iId, int iState)
{
	//TYPE_EDIT, TYPE_LIST
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		pCon->iState = iState;
		pCon->bPressed = true;
	}
}

bool C_GUI::IsAnyEditFocused()
{
	int i, iSize = (int)m_pObj->size();

	for(i=0; i<iSize; i++) {
		S_GUIObject *pstCon = &(*m_pObj)[i];

		if(pstCon->iType == TYPE_EDIT) {
			if(pstCon->iState == STATE_ACTIVE) {
				return true;
			}
		}
	}
	return false;
}

bool C_GUI::IsActive(int iId)
{
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		return (pCon->iState != STATE_DISABLED);
	}
	return false;
}

void C_GUI::SetColor(int iId, int iColId, S_FColor *pColor)
{
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		if(iColId<3) {
			pCon->stCol[iColId] = *pColor;
		} else if(iColId==COLOR_TEXT) {
			pCon->stTextCol = *pColor;
		} else if(iColId==COLOR_SELECT) {
			pCon->stSelectCol = *pColor;
		}
	}
}
void C_GUI::SetColor(int iId, int iColId, float r, float g, float b, float a)
{
	S_FColor stColor = {r, g, b, a};
	SetColor(iId, iColId, &stColor);
}

bool C_GUI::WasChanged(int iId)
{
	//TYPE_EDIT, TYPE_LIST (selection), TYPE_CHECK
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		if(pCon->bChanged!=0) {
			pCon->bChanged = 0; //reset
			return true;
		}
	}
	return false;
}

void C_GUI::SetChecked(int iId, bool bChecked)
{
	//TYPE_CHECK, TYPE_BUTTON (sticky)
	S_GUIObject *pCon = GetObject(iId);

	if(pCon) {
		pCon->bChecked = bChecked;
		if(pCon->bSticky) pCon->bPressed = bChecked;
	}
}

int C_GUI::GetTextWidth(char *szText)
{
	int iW;
	s_pFont->GetTextSize(szText, &iW);
	return iW;
}
