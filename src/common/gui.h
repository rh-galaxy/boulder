// C_GUI

#ifndef _GUI_H 
#define _GUI_H

#include "graph.h"
#include "font.h"
#include "image.h"
#include "input.h"
#include "timer.h"

#include <vector>

#define TYPE_BK        1
#define TYPE_BUTTON    2
#define TYPE_CHECK     3
#define TYPE_EDIT      4
#define TYPE_LIST      5
#define TYPE_STATIC    6

#define STATE_DISABLED 0
#define STATE_ENABLED  1
#define STATE_ACTIVE   2

#define COLOR_DISABLED STATE_DISABLED
#define COLOR_ENABLED  STATE_ENABLED
#define COLOR_ACTIVE   STATE_ACTIVE
#define COLOR_TEXT     3
#define COLOR_SELECT   4

#define MAX_COLUMNS    6
#define MAX_TEXT       128

struct S_GUIListEl
{
	int iID;

	char szText[MAX_COLUMNS][MAX_TEXT]; //short text for columns
	C_Image *pIcon;                   //shown in column 0 if used
	int iIconRect;
};
typedef std::vector<S_GUIListEl> C_GUIListElements;

struct S_GUIObject
{
	int iID;       //unique id

	int iType;     //one of the above
	int iState;    //one of the above. disabled shows in darker color and will not respond to input. active has focus.

	int bPressed; //valid for all, is currently pressed.
	int bChecked; //valid for TYPE_CHECK TYPE_BUTTON (sticky). is checked.
	int bClicked; //valid for TYPE_BUTTON TYPE_EDIT (when enter pressed).
	int bChanged; //valid for TYPE_EDIT, TYPE_LIST (selction)

	int bDeletePressed; //valid for TYPE_LIST (selction)

	S_Rect stRect; //valid for TYPE_BUTTON, TYPE_CHECK, TYPE_EDIT, TYPE_LIST. outer dim.

	char szText[MAX_TEXT]; //short text
	int iMaxText;

	int bOnlyNumbers; //valid for TYPE_EDIT
	int bOnlyEmail;   //valid for TYPE_EDIT (0-9,a-z,A-Z,@,.,-,_, space is also allowed to be able to use this for other than email)
	int bIsPassWord;  //valid for TYPE_EDIT
	int bSticky;      //valid for TYPE_BUTTON

	S_FColor stCol[3];
	S_FColor stTextCol;
	S_FColor stSelectCol;

	C_GUIListElements *pListEl;
	int iListColumns; //number of columns in list
	int iListColumnW[MAX_COLUMNS];
	int iLastColumnWOriginal;
	int iListSelection; //index of selection
	int iListTop;       //index currently drawn at top of list
	int iListVBarStatus; //bits [isshown][buttonupactive][buttondownactive][buttonuppressed][buttondownpressed]
	int iBarPosY, iBarHeight; //saved scrollbar info
	int iListClickedY, iListClickedSelection; //saved scrollbar clicked info

	bool bListHasSelection;
};
typedef std::vector<S_GUIObject> C_GUIObjects;

class C_GUI
{
public:
	C_GUI();
	~C_GUI();

	//input and output
	bool LoadTemplate(char *szFile, char *szResource, int iScrOffsX, int iScrOffsY);
	bool SetPos(int iScrOffsX, int iScrOffsY);

	void ListAddElement(int iListId, S_GUIListEl *pListEl);
	void ListRemoveElementById(int iListId, int iListEl);
	void ListRemoveElement(int iListId, char *szListText0);
	void ListRemoveAll(int iListId);
	S_GUIListEl *ListGetElementById(int iListId, int iListEl);
	S_GUIListEl *ListGetElement(int iListId, char *szListText0);
	S_GUIListEl *ListGetSelectedElement(int iListId, int *o_iId = NULL);
	void ListSetSelectedElement(int iListId, char *szListText0);
	void ListSetSelectedElement(int iListId, int iListIndex);
	void ListSetHasSelection(int iListId, bool bHasSelection);
	bool ListWasDeletePressed(int iListId);

	S_GUIObject *GetObject(int iId);

	void SetText(int iId, char *szText);
	void GetText(int iId, char *o_szText);

	bool IsChecked(int iId);
	bool WasClicked(int iId);
	bool WasChanged(int iId); //TYPE_EDIT, TYPE_LIST (selection), TYPE_CHECK
	void SetFocused(int iId, int iState = STATE_ACTIVE); //TYPE_EDIT, TYPE_LIST
	bool IsAnyEditFocused();

	void SetChecked(int iId, bool bChecked); //TYPE_CHECK, TYPE_BUTTON (sticky)
	void SetActive(int iId, bool bActive);
	bool IsActive(int iId);

	void SetColor(int iId, int iColId, S_FColor *pColor);
	void SetColor(int iId, int iColId, float r, float g, float b, float a);

	int GetTextWidth(char *szText);
	C_Font *GetFont() {return s_pFont;}; //to lend the font

	//event handlers
	void MouseMove(int x, int y);
	void MouseDown(int x, int y);
	void MouseUp(int x, int y);
	void MouseWheel(int iWay);
	void UpdateInput();

	void Draw();
private:
	int m_iScrOffsX, m_iScrOffsY;
	//default colors
	static S_FColor s_stStateCol[3];
	static S_FColor s_stSelectCol;
	static S_FColor s_stTextColor;

	//button
	static C_Image *s_pButton[2]; //normal, pressed
	static S_Rect s_stButtonR[9];

	//check
	static C_Image *s_pCheck[2]; //unchecked, checked
	static S_Rect s_stCheckR[9];

	//edit
	static C_Image *s_pEdit; //normal
	static S_Rect s_stEditR[9];

	//list
	static C_Image *s_pListVBar; //normal
	static S_Rect s_stListVBarR[6];

	void ListModSelection(int iWay, int i_iID = -1); //-1 for current active
	void ListDelSelection(int iID = -1); //-1 for current active

	int ListGetNumRowsUntil(int iListId, int iLast);
	int ListGetElRows(int iWidth, char *szText);
	char m_aszRows[10][MAX_TEXT];

	double m_dTime, m_dKeyTimer;

	//common
	static int     s_iObjCount;
	static C_Font  *s_pFont;
	C_GraphWrapper *m_pGraph;
	C_GUIObjects   *m_pObj;

	int GetInput();
	void EnterChar(int c);
	C_Control *m_pInput;

	C_Timer   *m_pTimer;

	void DrawBorderedRect(S_Rect *pDstRect, S_FColor *pModColor, C_Image *pSrcImage, S_Rect *pSrcRects/*9*/, int iBorderSize);

	bool m_bLastMouseLeft;
	int m_iLastMouseX, m_iLastMouseY;
};

#endif
