// C_Control

//class to unify input handling of different OS
// - keyboard 
// - mouse
// - joystick pad
// max one device of each type

//since the class was first written for DirectInput, defines
// from it's headers lives on

#include "input.h"
#include <math.h> //fabs

#define KEYDOWN(name,key) (char)(name[key] & 0x80)
C_Control *C_Control::s_pTheControl = NULL;

////////////////////////////////////////////////////////////////////////////////

C_Control::C_Control()
{
	memset(&m_iMButtonStateBuf[0], 0, sizeof(m_iMButtonStateBuf));
	memset(&m_iMButtonState[0], 0, sizeof(m_iMButtonState));
	m_iMStateXBuf = m_iMStateYBuf = 0;
	memset(&m_stJState, 0, sizeof(m_stJState));

	m_pTimer = new C_Timer();
	m_iRepeatRate = 45;
	m_iRepeatDelay = 200;
	InitAsciiLookup();
	memset(m_acKBuf, 0, sizeof(m_acKBuf));

	m_dCurX = 0, m_dCurY = 0;
	m_dStepX = 0, m_dStepY = 0;

	m_hController = NULL;
	
	SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1"); //apparently needed in windows 11
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
	int nJoysticks = SDL_NumJoysticks();
	for (int i = 0; i < nJoysticks; i++) {
		//const char *name = SDL_JoystickNameForIndex(i);
		if (SDL_IsGameController(i)) {
			m_hController = SDL_GameControllerOpen(i);
			break;
		}
	}

	m_iMStateXBufLast = m_iMStateYBufLast = 0;
	m_iMStateXBufLast2 = m_iMStateYBufLast2 = 0;

	m_pMouseEvents = new std::deque<S_MouseEvent>;

	s_pTheControl = this;
}

void C_Control::DisableGamepad()
{
	if (m_hController != NULL) SDL_GameControllerClose(m_hController);
	m_hController = NULL;
}

C_Control::~C_Control()
{
	if(m_hController != NULL) SDL_GameControllerClose(m_hController);
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);

	delete m_pMouseEvents;
	delete m_pTimer;
	s_pTheControl = NULL;
}

//keyboard
int C_Control::IsPressed(int iKey)
{
	m_acPressed[iKey] = KEYDOWN(m_acKBuf, iKey);
	return m_acPressed[iKey];
}

int C_Control::IsPressedOnce(int iKey)
{
	if(!m_acPressed[iKey] && KEYDOWN(m_acKBuf, iKey)) {
		m_acPressed[iKey] = 1;
		m_adRepeat[iKey] = 0.1;
		m_aiRepeatCount[iKey] = 0;
		return 1;
	}
	return 0;
}

int C_Control::IsPressedRepeat(int iKey)
{
	uint32_t iRepeatVal = m_iRepeatRate;
	if(m_aiRepeatCount[iKey] == 1) iRepeatVal = m_iRepeatDelay;

	if(m_adRepeat[iKey] > iRepeatVal) {
		m_aiRepeatCount[iKey]++;
		m_acPressed[iKey] = 1;
		m_adRepeat[iKey] = 0.1;
		return 1;
	}
	return 0;
}

int C_Control::GetAscii(int iDiIndex)
{
	if(KEYDOWN(m_acKBuf, DIK_RSHIFT) | KEYDOWN(m_acKBuf, DIK_LSHIFT))
		return ASCIILOOKUPSHIFTED[iDiIndex];
	else
		return ASCIILOOKUP[iDiIndex];
}

void C_Control::InitAsciiLookup()
{
	int i;
	for(i=0; i<256; i++) ASCIILOOKUP[i] = -1;
	for(i=0; i<256; i++) ASCIILOOKUPSHIFTED[i] = -1; //shift has no effect
	ASCIILOOKUP[DIK_ESCAPE] = 27;
	ASCIILOOKUP[DIK_1] = '1'; ASCIILOOKUPSHIFTED[DIK_1] = '!';
	ASCIILOOKUP[DIK_2] = '2'; ASCIILOOKUPSHIFTED[DIK_2] = '@';
	ASCIILOOKUP[DIK_3] = '3'; ASCIILOOKUPSHIFTED[DIK_3] = '#';
	ASCIILOOKUP[DIK_4] = '4'; ASCIILOOKUPSHIFTED[DIK_4] = '$';
	ASCIILOOKUP[DIK_5] = '5'; //ASCIILOOKUPSHIFTED[DIK_5] = '%';
	ASCIILOOKUP[DIK_6] = '6'; ASCIILOOKUPSHIFTED[DIK_6] = '^';
	ASCIILOOKUP[DIK_7] = '7'; //ASCIILOOKUPSHIFTED[DIK_7] = '&';
	ASCIILOOKUP[DIK_8] = '8'; ASCIILOOKUPSHIFTED[DIK_8] = '*';
	ASCIILOOKUP[DIK_9] = '9'; ASCIILOOKUPSHIFTED[DIK_9] = '(';
	ASCIILOOKUP[DIK_0] = '0'; ASCIILOOKUPSHIFTED[DIK_0] = ')';
	ASCIILOOKUP[DIK_MINUS] = '-'; ASCIILOOKUPSHIFTED[DIK_MINUS] = '_';
	ASCIILOOKUP[DIK_EQUALS] = '='; ASCIILOOKUPSHIFTED[DIK_EQUALS] = '+';
	ASCIILOOKUP[DIK_BACK] = 8;
	//ASCIILOOKUP[DIK_TAB] = 9;
	ASCIILOOKUP[DIK_Q] = 'q'; ASCIILOOKUPSHIFTED[DIK_Q] = 'Q';
	ASCIILOOKUP[DIK_W] = 'w'; ASCIILOOKUPSHIFTED[DIK_W] = 'W';
	ASCIILOOKUP[DIK_E] = 'e'; ASCIILOOKUPSHIFTED[DIK_E] = 'E';
	ASCIILOOKUP[DIK_R] = 'r'; ASCIILOOKUPSHIFTED[DIK_R] = 'R';
	ASCIILOOKUP[DIK_T] = 't'; ASCIILOOKUPSHIFTED[DIK_T] = 'T';
	ASCIILOOKUP[DIK_Y] = 'y'; ASCIILOOKUPSHIFTED[DIK_Y] = 'Y';
	ASCIILOOKUP[DIK_U] = 'u'; ASCIILOOKUPSHIFTED[DIK_U] = 'U';
	ASCIILOOKUP[DIK_I] = 'i'; ASCIILOOKUPSHIFTED[DIK_I] = 'I';
	ASCIILOOKUP[DIK_O] = 'o'; ASCIILOOKUPSHIFTED[DIK_O] = 'O';
	ASCIILOOKUP[DIK_P] = 'p'; ASCIILOOKUPSHIFTED[DIK_P] = 'P';
	ASCIILOOKUP[DIK_LBRACKET] = 0xe5; ASCIILOOKUPSHIFTED[DIK_LBRACKET] = 0xc5; //åÅ
	ASCIILOOKUP[DIK_RBRACKET] = '~'; ASCIILOOKUPSHIFTED[DIK_RBRACKET] = '^';
	ASCIILOOKUP[DIK_RETURN] = 13; ASCIILOOKUPSHIFTED[DIK_RETURN] = 13;
	ASCIILOOKUP[DIK_A] = 'a'; ASCIILOOKUPSHIFTED[DIK_A] = 'A';
	ASCIILOOKUP[DIK_S] = 's'; ASCIILOOKUPSHIFTED[DIK_S] = 'S';
	ASCIILOOKUP[DIK_D] = 'd'; ASCIILOOKUPSHIFTED[DIK_D] = 'D';
	ASCIILOOKUP[DIK_F] = 'f'; ASCIILOOKUPSHIFTED[DIK_F] = 'F';
	ASCIILOOKUP[DIK_G] = 'g'; ASCIILOOKUPSHIFTED[DIK_G] = 'G';
	ASCIILOOKUP[DIK_H] = 'h'; ASCIILOOKUPSHIFTED[DIK_H] = 'H';
	ASCIILOOKUP[DIK_J] = 'j'; ASCIILOOKUPSHIFTED[DIK_J] = 'J';
	ASCIILOOKUP[DIK_K] = 'k'; ASCIILOOKUPSHIFTED[DIK_K] = 'K';
	ASCIILOOKUP[DIK_L] = 'l'; ASCIILOOKUPSHIFTED[DIK_L] = 'L';
	ASCIILOOKUP[DIK_SEMICOLON] = 0xf6; ASCIILOOKUPSHIFTED[DIK_SEMICOLON] = 0xd6; //öÖ
	ASCIILOOKUP[DIK_APOSTROPHE] = 0xe4; ASCIILOOKUPSHIFTED[DIK_APOSTROPHE] = 0xc4; //äÄ
	ASCIILOOKUP[DIK_GRAVE] = '\''; ASCIILOOKUPSHIFTED[DIK_GRAVE] = '*';
	ASCIILOOKUP[DIK_BACKSLASH] = '<'; ASCIILOOKUPSHIFTED[DIK_BACKSLASH] = '>';
	ASCIILOOKUP[DIK_Z] = 'z'; ASCIILOOKUPSHIFTED[DIK_Z] = 'Z';
	ASCIILOOKUP[DIK_X] = 'x'; ASCIILOOKUPSHIFTED[DIK_X] = 'X';
	ASCIILOOKUP[DIK_C] = 'c'; ASCIILOOKUPSHIFTED[DIK_C] = 'C';
	ASCIILOOKUP[DIK_V] = 'v'; ASCIILOOKUPSHIFTED[DIK_V] = 'V';
	ASCIILOOKUP[DIK_B] = 'b'; ASCIILOOKUPSHIFTED[DIK_B] = 'B';
	ASCIILOOKUP[DIK_N] = 'n'; ASCIILOOKUPSHIFTED[DIK_N] = 'N';
	ASCIILOOKUP[DIK_M] = 'm'; ASCIILOOKUPSHIFTED[DIK_M] = 'M';
	ASCIILOOKUP[DIK_COMMA] = ','; ASCIILOOKUPSHIFTED[DIK_COMMA] = ';';
	ASCIILOOKUP[DIK_PERIOD] = '.'; ASCIILOOKUPSHIFTED[DIK_PERIOD] = ':';
	ASCIILOOKUP[DIK_SLASH] = '-'; ASCIILOOKUPSHIFTED[DIK_SLASH] = '_';
	ASCIILOOKUP[DIK_MULTIPLY] = '*'; ASCIILOOKUPSHIFTED[DIK_MULTIPLY] = '*';
	ASCIILOOKUP[DIK_SPACE] = 32; ASCIILOOKUPSHIFTED[DIK_SPACE] = 32;
};

void C_Control::UpdateKeys()
{
	//set some data from the device states (m_acKBuf)
	m_dMouseTimer = m_pTimer->GetInterval();
	for(int i=0; i<256; i++) {
		if(KEYDOWN(m_acKBuf, i)) {
			if(m_adRepeat[i]>0.0) m_adRepeat[i] += m_dMouseTimer;
			else {
				m_aiRepeatCount[i] = 0;
				m_adRepeat[i] = m_iRepeatRate+0.1;
			}
		} else {
			m_adRepeat[i] = 0;
			m_acPressed[i] = 0;
		}
	}
}

//mouse
int C_Control::GetMouseEvent(int *o_iX, int *o_iY, int *o_iExtraVal)
{
	if(m_pMouseEvents->empty()) return 0;
	int iEvent = m_pMouseEvents->front().iEvent;
	if(o_iX) *o_iX = m_pMouseEvents->front().iX;
	if(o_iY) *o_iY = m_pMouseEvents->front().iY;
	if(o_iExtraVal) *o_iExtraVal = m_pMouseEvents->front().iExtraVal;
	m_pMouseEvents->pop_front();
	return iEvent;
}

void C_Control::GetMousePos(int *o_iX, int *o_iY)
{
	if(o_iX) *o_iX = m_iMStateXBufLast;
	if(o_iY) *o_iY = m_iMStateYBufLast;
}

void C_Control::GetMouseButton(int *o_iLeft, int *o_iRight, int *o_iMiddle)
{
	if(o_iLeft) *o_iLeft = m_iMButtonStateBuf[0];
	if(o_iRight) *o_iRight = m_iMButtonStateBuf[1];
	if(o_iMiddle) *o_iMiddle = m_iMButtonStateBuf[2];
}

bool C_Control::MouseIsPressed(int iButton)
{
	if(iButton<0 || iButton>2) return false;
	return m_iMButtonStateBuf[iButton] != 0;
}

bool C_Control::MouseIsPressedOnce(int iButton)
{
	if(iButton<0 || iButton>2) return false;
	if(m_iMButtonState[0]==0 && m_iMButtonStateBuf[0]!=0) {
		m_iMButtonState[0] = 1;
		return true;
	}
	return false;
}

int C_Control::GetLastWheelEvent()
{
	return m_iMWheelState;
}

void C_Control::GetMouseMovement(int *o_iX, int *o_iY)
{
	if(o_iX) *o_iX = m_iMStateXBuf;
	if(o_iY) *o_iY = m_iMStateYBuf;
}

#define SPREADTIME 50.0 //ms
void C_Control::GetMouseMovementSmooth(double *o_dX, double *o_dY)
{
	double dResultX = 0, dResultY = 0;
	if(m_iMStateXBuf || m_iMStateYBuf) {
		m_dCurX += m_iMStateXBuf;
		m_dCurY += m_iMStateYBuf;

		double dStep = m_dMouseTimer/SPREADTIME; //% of movement to distribute each time called
		if(dStep > 1.0) dStep = 1.0;  //if frametime too long we must not move faster
		if(dStep <= 0.0) dStep = 1.0; //if the timer has to low resolution compared to the framerate

		m_dStepX = m_dCurX * dStep;
		m_dStepY = m_dCurY * dStep;
	}

	if(fabs(m_dCurX) > fabs(m_dStepX)) {
		m_dCurX -= m_dStepX;
		dResultX += m_dStepX;
	} else {
		dResultX += m_dCurX;
		m_dCurX = 0;
	}

	if(fabs(m_dCurY) > fabs(m_dStepY)) {
		m_dCurY -= m_dStepY;
		dResultY += m_dStepY;
	} else {
		dResultY += m_dCurY;
		m_dCurY = 0;
	}

	if(o_dX) *o_dX = dResultX;
	if(o_dY) *o_dY = dResultY;
}

void C_Control::GetGamepadState(S_GamepadState *o_pState)
{
	*o_pState = m_stJState;
}

void C_Control::Update()
{
	//mouse
	if(m_iMButtonStateBuf[0] == 0) m_iMButtonState[0] = 0;
	if(m_iMButtonStateBuf[1] == 0) m_iMButtonState[1] = 0;
	if(m_iMButtonStateBuf[2] == 0) m_iMButtonState[2] = 0;
	m_iMWheelState = m_iMWheelStateBuf;
	m_iMWheelStateBuf = 0;

	//gamepad
	if(m_hController != NULL && SDL_GameControllerGetAttached(m_hController)) {
		SDL_GameControllerUpdate();

		//get state to m_stJState
		m_stJState.iX = SDL_GameControllerHasAxis(m_hController, SDL_CONTROLLER_AXIS_LEFTX) ? SDL_GameControllerGetAxis(m_hController, SDL_CONTROLLER_AXIS_LEFTX) : 0;
		m_stJState.iY = SDL_GameControllerHasAxis(m_hController, SDL_CONTROLLER_AXIS_LEFTY) ? SDL_GameControllerGetAxis(m_hController, SDL_CONTROLLER_AXIS_LEFTY) : 0;
		m_stJState.iX2 = SDL_GameControllerHasAxis(m_hController, SDL_CONTROLLER_AXIS_RIGHTX) ? SDL_GameControllerGetAxis(m_hController, SDL_CONTROLLER_AXIS_RIGHTX) : 0;
		m_stJState.iY2 = SDL_GameControllerHasAxis(m_hController, SDL_CONTROLLER_AXIS_RIGHTY) ? SDL_GameControllerGetAxis(m_hController, SDL_CONTROLLER_AXIS_RIGHTY) : 0;
		m_stJState.iTrigger = SDL_GameControllerHasAxis(m_hController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) ? SDL_GameControllerGetAxis(m_hController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) : 0;
		m_stJState.iTrigger2 = SDL_GameControllerHasAxis(m_hController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) ? SDL_GameControllerGetAxis(m_hController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) : 0;

		m_stJState.iButton = 0;
		m_stJState.iDpad = 0;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_A)) m_stJState.iButton |= 0x001;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_B)) m_stJState.iButton |= 0x002;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_X)) m_stJState.iButton |= 0x004;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_Y)) m_stJState.iButton |= 0x008;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_BACK)) m_stJState.iBack |= 1;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) m_stJState.iButton |= 0x010;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) m_stJState.iButton |= 0x020;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_DPAD_UP)) m_stJState.iDpad |= 0x001;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) m_stJState.iDpad |= 0x002;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) m_stJState.iDpad |= 0x004;
		if (SDL_GameControllerGetButton(m_hController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) m_stJState.iDpad |= 0x008;
	}

	UpdateKeys();
}

int C_Control::GetAscii2(int iDiIndex)
{
	return GetAscii(iDiIndex);
}

void C_Control::PollInput(SDL_Event *pKeyMouseEvent)
{
	SDLLateKeyEvent(pKeyMouseEvent);
	SDLLateMouseEvent(pKeyMouseEvent);
}

void C_Control::SDLLateKeyEvent(SDL_Event *pKeyEvent)
{
	if(pKeyEvent == NULL)
		return;

	if(pKeyEvent->type == SDL_KEYUP || pKeyEvent->type == SDL_KEYDOWN)
	{
		int key = -1;
		SDL_Keysym keysym = pKeyEvent->key.keysym;

		switch(keysym.sym)
		{
			case SDLK_PAGEUP:      key = DIK_PRIOR;    break;
			case SDLK_PAGEDOWN:    key = DIK_NEXT;   break;
			case SDLK_HOME:        key = DIK_HOME;    break;
			case SDLK_END:         key = DIK_END;     break;
			case SDLK_KP_4:
			case SDLK_LEFT:        key = DIK_LEFT;    break;
			case SDLK_KP_6:
			case SDLK_RIGHT:       key = DIK_RIGHT;   break;
			case SDLK_KP_2:
			case SDLK_DOWN:        key = DIK_DOWN;    break;
			case SDLK_KP_8:
			case SDLK_UP:          key = DIK_UP;      break;
			case SDLK_INSERT:      key = DIK_INSERT;  break;
			case SDLK_DELETE:      key = DIK_DELETE;  break;

			case SDLK_ESCAPE:      key = DIK_ESCAPE;  break;
			case SDLK_KP_ENTER:    key = DIK_RETURN;  break;
			case SDLK_RETURN:      key = DIK_RETURN;  break;

			case SDLK_SPACE:       key = DIK_SPACE;  break;
			case SDLK_TAB:         key = DIK_TAB;  break;
			case SDLK_F1:          key = DIK_F1;   break;
			case SDLK_F2:          key = DIK_F2;   break;
			case SDLK_F3:          key = DIK_F3;   break;
			case SDLK_F4:          key = DIK_F4;   break;
			case SDLK_F5:          key = DIK_F5;   break;
			case SDLK_F6:          key = DIK_F6;   break;
			case SDLK_F7:          key = DIK_F7;   break;
			case SDLK_F8:          key = DIK_F8;   break;
			case SDLK_F9:          key = DIK_F9;   break;
			case SDLK_F10:         key = DIK_F10;  break;
			case SDLK_F11:         key = DIK_F11;  break;
			case SDLK_F12:         key = DIK_F12;  break;

			case SDLK_BACKSPACE:   key = DIK_BACK; break;

			case SDLK_LSHIFT:      key = DIK_LSHIFT;   break;
			case SDLK_RSHIFT:      key = DIK_RSHIFT;   break;

			//case SDLK_RETURN:
			case SDLK_LCTRL:
			case SDLK_RCTRL:       key = DIK_LCONTROL; break;

			case SDLK_MINUS:       key = DIK_MINUS; break;
			case SDLK_PLUS:        key = DIK_BACKSLASH; break;

			case SDLK_RALT:
			case SDLK_LALT:        key = DIK_LMENU;    break;

			case SDLK_KP_MULTIPLY: key = DIK_MULTIPLY; break;
			case SDLK_KP_PLUS:     key = DIK_ADD;      break;
			case SDLK_KP_MINUS:    key = DIK_SUBTRACT; break;
			case SDLK_KP_DIVIDE:   key = DIK_DIVIDE;   break;

			case SDLK_0:           key = DIK_0; break;
			case SDLK_1:           key = DIK_1; break;
			case SDLK_2:           key = DIK_2; break;
			case SDLK_3:           key = DIK_3; break;
			case SDLK_4:           key = DIK_4; break;
			case SDLK_5:           key = DIK_5; break;
			case SDLK_6:           key = DIK_6; break;
			case SDLK_7:           key = DIK_7; break;
			case SDLK_8:           key = DIK_8; break;
			case SDLK_9:           key = DIK_9; break;

			case SDLK_a:           key = DIK_A; break;
			case SDLK_b:           key = DIK_B; break;
			case SDLK_c:           key = DIK_C; break;
			case SDLK_d:           key = DIK_D; break;
			case SDLK_e:           key = DIK_E; break;
			case SDLK_f:           key = DIK_F; break;
			case SDLK_g:           key = DIK_G; break;
			case SDLK_h:           key = DIK_H; break;
			case SDLK_i:           key = DIK_I; break;
			case SDLK_j:           key = DIK_J; break;
			case SDLK_k:           key = DIK_K; break;
			case SDLK_l:           key = DIK_L; break;
			case SDLK_m:           key = DIK_M; break;
			case SDLK_n:           key = DIK_N; break;
			case SDLK_o:           key = DIK_O; break;
			case SDLK_p:           key = DIK_P; break;
			case SDLK_q:           key = DIK_Q; break;
			case SDLK_r:           key = DIK_R; break;
			case SDLK_s:           key = DIK_S; break;
			case SDLK_t:           key = DIK_T; break;
			case SDLK_u:           key = DIK_U; break;
			case SDLK_v:           key = DIK_V; break;
			case SDLK_w:           key = DIK_W; break;
			case SDLK_x:           key = DIK_X; break;
			case SDLK_y:           key = DIK_Y; break;
			case SDLK_z:           key = DIK_Z; break;
			case SDLK_COMMA:       key = DIK_COMMA;  break;
			case SDLK_PERIOD:      key = DIK_PERIOD; break;
		}

		if(key >= 0)
			m_acKBuf[key] = (pKeyEvent->type == SDL_KEYDOWN) ? (char)0x80 : 0;
	}
}

void C_Control::SDLLateMouseEvent(SDL_Event *pMouseEvent)
{
	m_iMStateXBuf = 0;
	m_iMStateYBuf = 0;

	if(pMouseEvent == NULL)
		return;

	int x, y;
	SDL_GetMouseState(&x, &y);

	if(pMouseEvent->type == SDL_MOUSEMOTION || pMouseEvent->type == SDL_MOUSEBUTTONDOWN
		|| pMouseEvent->type == SDL_MOUSEBUTTONUP || pMouseEvent->type == SDL_MOUSEWHEEL)
	{
		static int aiButtonLookup[4] = {0,0,2,1};

		if((pMouseEvent->type == SDL_MOUSEBUTTONDOWN || pMouseEvent->type == SDL_MOUSEBUTTONUP)
		 && pMouseEvent->button.button >= 1 && pMouseEvent->button.button <= 3)
		{
			char cButton = pMouseEvent->type == SDL_MOUSEBUTTONDOWN ? 1 : 0;
			m_iMButtonStateBuf[aiButtonLookup[pMouseEvent->button.button]] = cButton;
		}

		m_iMStateXBuf = x - m_iMStateXBufLast2;
		m_iMStateYBuf = y - m_iMStateYBufLast2;

		m_iMStateXBufLast2 = x;
		m_iMStateYBufLast2 = y;
		m_iMStateXBufLast = x;
		m_iMStateYBufLast = y;

		S_MouseEvent stEvent;
		stEvent.iX = x;
		stEvent.iY = y;
		if(pMouseEvent->type == SDL_MOUSEMOTION)
		{
			stEvent.iEvent = EVT_MOUSE_MOVE;
			m_pMouseEvents->push_back(stEvent);
		}
		else if(pMouseEvent->type == SDL_MOUSEBUTTONDOWN)
		{
			switch(pMouseEvent->button.button) {
				case SDL_BUTTON_LEFT: stEvent.iEvent = EVT_MOUSE_LEFT_DOWN;  m_pMouseEvents->push_back(stEvent); break;
				case SDL_BUTTON_RIGHT: stEvent.iEvent = EVT_MOUSE_RIGHT_DOWN; m_pMouseEvents->push_back(stEvent); break;
				case SDL_BUTTON_MIDDLE: stEvent.iEvent = EVT_MOUSE_MID_DOWN;   m_pMouseEvents->push_back(stEvent); break;
				// case Button4: stEvent.iEvent = EVT_MOUSE_WHEEL_UP;   stEvent.iExtraVal = -1; m_pMouseEvents->push_back(stEvent); break;
				// case Button5: stEvent.iEvent = EVT_MOUSE_WHEEL_DOWN; stEvent.iExtraVal = 1;  m_pMouseEvents->push_back(stEvent); break;
			}
		}
		else if(pMouseEvent->type == SDL_MOUSEBUTTONUP)
		{
			switch(pMouseEvent->button.button) {
				case SDL_BUTTON_LEFT: stEvent.iEvent = EVT_MOUSE_LEFT_UP;  m_pMouseEvents->push_back(stEvent); break;
				case SDL_BUTTON_RIGHT: stEvent.iEvent = EVT_MOUSE_RIGHT_UP; m_pMouseEvents->push_back(stEvent); break;
				case SDL_BUTTON_MIDDLE: stEvent.iEvent = EVT_MOUSE_MID_UP;   m_pMouseEvents->push_back(stEvent); break;
				// case Button4: stEvent.iEvent = EVT_MOUSE_WHEEL_UP;   stEvent.iExtraVal = -1; m_pMouseEvents->push_back(stEvent); break;
				// case Button5: stEvent.iEvent = EVT_MOUSE_WHEEL_DOWN; stEvent.iExtraVal = 1;  m_pMouseEvents->push_back(stEvent); break;
			}
		}
		else if(pMouseEvent->type == SDL_MOUSEWHEEL)
		{
			if(pMouseEvent->wheel.y > 0) // scroll up
			{
				m_iMWheelStateBuf = EVT_MOUSE_WHEEL_UP;
				stEvent.iEvent = EVT_MOUSE_WHEEL_UP;
				stEvent.iExtraVal = -1;
				m_pMouseEvents->push_back(stEvent);
			}
			else if(pMouseEvent->wheel.y < 0) // scroll down
			{
				m_iMWheelStateBuf = EVT_MOUSE_WHEEL_DOWN;
				stEvent.iEvent = EVT_MOUSE_WHEEL_DOWN;
				stEvent.iExtraVal = 1;
				m_pMouseEvents->push_back(stEvent);
			}
		}

		if(m_pMouseEvents->size()>12)
			m_pMouseEvents->pop_front();
	}
}
