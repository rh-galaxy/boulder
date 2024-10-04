// C_Control

#ifndef _CONTROL_H_
#define _CONTROL_H_

#include "common.h"
#include "timer.h"
#include "input_defines.h"

#include <deque>

#include <SDL.h>


class C_Control {
public:
	C_Control();
	void PollInput(SDL_Event *pKeyMouseEvent);

	~C_Control();
	static C_Control *GetControlObject() {return s_pTheControl;};

	void DisableGamepad();

	void Update();

	//keyboard
	int IsPressed(int iKey);
	int IsPressedOnce(int iKey);

	void SetRate(int iMsRate, int iMsDelay)
		{m_iRepeatRate = iMsRate; m_iRepeatDelay = iMsDelay;};
	int IsPressedRepeat(int iKey);

	int GetAscii(int iDiIndex);
	int GetAscii2(int iDiIndex);

	//mouse
	int GetMouseEvent(int *o_iX, int *o_iY, int *o_iExtraVal);
	void GetMouseMovement(int *o_iX, int *o_iY);             //pixels since last
	void GetMouseMovementSmooth(double *o_dX, double *o_dY); //pixels/frame 
	void GetMousePos(int *o_iX, int *o_iY);
	void GetMouseButton(int *o_iLeft, int *o_iRight, int *o_iMiddle); //mouse button state
	bool MouseIsPressed(int iButton); //can be 0, 1, 2 (Left, Right, Middle)
	bool MouseIsPressedOnce(int iButton);
	int GetLastWheelEvent();

	//gamepad
	void GetGamepadState(S_GamepadState *o_pState);
private:
	C_Timer             *m_pTimer;
	static C_Control    *s_pTheControl;

	//gamepad
	SDL_GameController  *m_hController;
	S_GamepadState       m_stJState;
	
	//mouse
	void SDLLateMouseEvent(SDL_Event *pMouseEvent);
	int m_iMStateXBufLast2, m_iMStateYBufLast2;
	int m_iMWheelStateBuf, m_iMWheelState;

	double               m_dMouseTimer;
	double               m_dCurX, m_dCurY;
	double               m_dStepX, m_dStepY;
	char                 m_iMButtonStateBuf[3];
	char                 m_iMButtonState[3];
	int                  m_iMStateXBuf, m_iMStateYBuf;
	int                  m_iMStateXBufLast, m_iMStateYBufLast; //mouse pos in screen coords
	std::deque<S_MouseEvent> *m_pMouseEvents;

	//keyboard
	void SDLLateKeyEvent(SDL_Event *pKeyEvent);

	void InitAsciiLookup();
	void UpdateKeys();
	int                  ASCIILOOKUP[256];
	int                  ASCIILOOKUPSHIFTED[256];
	char                 m_acKBuf[256];

	uint32_t             m_iRepeatRate;
	uint32_t             m_iRepeatDelay;
	uint32_t             m_aiRepeatCount[256];
	double               m_adRepeat[256];
	char                 m_acPressed[256];
};

#endif

