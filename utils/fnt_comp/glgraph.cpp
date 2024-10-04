// C_GraphWrapperGL

//class to init and use opengl on different OS, with only native code, no external libraries are used.
// supports windows, mac and linux.

#include "glgraph.h"
#include <math.h>

#ifdef WIN32
#pragma comment (lib, "opengl32.lib")
#endif

C_GraphWrapperGL *C_GraphWrapperGL::s_pclTheGLGraph = NULL;

#ifdef WIN32
C_GraphWrapperGL::C_GraphWrapperGL(WNDPROC i_pWndProc, HICON i_hIcon)
{
	m_hInstance = GetModuleHandle(NULL);
	m_pWndProc = i_pWndProc;
	m_hIcon = i_hIcon;
	m_bClassRegistered = false;

	m_hWnd = NULL;
	m_hRC = NULL;
	m_hDC = NULL;
#else
C_GraphWrapperGL::C_GraphWrapperGL()
{
	m_pDisplay = NULL;
	m_pVI      = NULL;
	m_GLC      = NULL;
	m_Win      = 0;
#endif
	m_bFullScreen = false;
	m_szErrorMsg[0] = 0;

	strcpy(m_szWndTitle, "OpenGL UNTITLED");
	s_pclTheGLGraph = this;

	SetBltColor(NULL);
}

C_GraphWrapperGL::~C_GraphWrapperGL()
{
	s_pclTheGLGraph = NULL;

	int i, iSize = (int)m_clTextureList.size();
	for(i=0; i<iSize; i++) {
		S_TextureData *pstTexture = &m_clTextureList[i];
		glDeleteTextures(1, (GLuint*)&pstTexture->hTexture);
	}

	KillGLWindow();
}

void C_GraphWrapperGL::SetWindowTitle(char *szWndTitle)
{
	strncpy(m_szWndTitle, szWndTitle, sizeof(m_szWndTitle));
	m_szWndTitle[sizeof(m_szWndTitle)-1] = 0;

	//set the title
#if defined WIN32
	if(m_hWnd) SetWindowText(m_hWnd, m_szWndTitle);
#else
	if(m_pDisplay) XStoreName(m_pDisplay, m_Win, m_szWndTitle);
#endif
}

//properly kill the window
bool C_GraphWrapperGL::KillGLWindow()
{
	bool bResult = true;

#if defined WIN32
	//return to our original desktop if fullscreen
	if(m_bFullScreen) {
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(true); //show mouse pointer
	}

	//do we have a rendering context?
	if(m_hRC) {
		//try to release the DC and RC contexts
		if(!wglMakeCurrent(NULL, NULL)) {
			sprintf(m_szErrorMsg, "Release of DC and RC failed.");
			bResult = false;
		}

		//try to delete the rendering context
		if(!wglDeleteContext(m_hRC)) {
			sprintf(m_szErrorMsg, "Release rendering context failed.");
			bResult = false;
		}
		m_hRC = NULL;
	}

	//try to delete device context
	if(m_hDC && !ReleaseDC(m_hWnd, m_hDC)) {
		sprintf(m_szErrorMsg, "Release device context failed.");
		bResult = false;
		m_hDC = NULL;
	}

	//free window
	if(m_hWnd && !DestroyWindow(m_hWnd)) {
		sprintf(m_szErrorMsg, "Could not release hWnd.");
		bResult = false;
		m_hWnd = NULL;
	}

	//properly kill the window, to be able to reopen another window without receiving an error
	if(m_bClassRegistered && !UnregisterClass("OpenGLClass1", m_hInstance)) {
		sprintf(m_szErrorMsg, "Could not unregister class.");
		m_bClassRegistered = false;
		bResult = false;
	}
#else
    if(m_pDisplay) {
		glXMakeCurrent(m_pDisplay, None, NULL);
		if(m_GLC) glXDestroyContext(m_pDisplay, m_GLC);
		if(m_Win) XDestroyWindow(m_pDisplay, m_Win);

		XCloseDisplay(m_pDisplay);
		m_pDisplay = NULL;
	}
    m_GLC = NULL;
#endif

	m_bFullScreen = false;

	return bResult;
}

//all setup for OpenGL goes here
bool C_GraphWrapperGL::InitGL()
{
	glShadeModel(GL_SMOOTH);               //enables smooth shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  //black background

	glClearDepth(1.0f);                    //depth buffer setup
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);                //enable culling (backface)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blending function for translucency from source alpha value
	glDisable(GL_ALPHA_TEST);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //we want the best perspective correction to be done
	glEnable(GL_LINE_SMOOTH);              //used to antialias lines

	//always enable
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	//ortho 2d
	Set2DOrthoProjectionMode();

	return true;
}

void C_GraphWrapperGL::StateTexturing(bool bEnable)
{
	if(bEnable) glEnable(GL_TEXTURE_2D);
	else        glDisable(GL_TEXTURE_2D);
}

void C_GraphWrapperGL::StateLighting(bool bEnable)
{
	if(bEnable) glEnable(GL_LIGHTING);
	else        glDisable(GL_LIGHTING);
}

void C_GraphWrapperGL::StateWireframe(bool bEnable)
{
	if(bEnable) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool C_GraphWrapperGL::SetSize(int i_iWidth, int i_iHeight)
{
	if(m_bFullScreen) return false;

#if defined WIN32
	//make the rendering context active, although it already is...
	// on some machines (lenovo G50 to name one) this is needed after a minimize and restore of the window
	//it is strange that m_hDC==wglGetCurrentDC() and m_hRC==wglGetCurrentContext()
	wglMakeCurrent(m_hDC, m_hRC); //so this should have no effect, but it has
#endif

	m_iWidth  = i_iWidth;
	m_iHeight = i_iHeight;
	glViewport(0, 0, m_iWidth, m_iHeight);

	Set2DOrthoProjectionMode();
	
	return true;
}

bool C_GraphWrapperGL::SetMode(int i_iWidth, int i_iHeight, bool i_bFullScreen, bool i_bResize)
{
	//make sure no mode is set
	KillGLWindow();

	m_bFullScreen = i_bFullScreen;
	m_iWidth = i_iWidth;
	m_iHeight = i_iHeight;

#if defined WIN32
	int      iPixelFormat; //holds results after searching for a matching PF
	WNDCLASS wc;
	DWORD    dwExStyle;
	DWORD    dwStyle;

	RECT stWindowRect = {0, 0, i_iWidth, i_iHeight};

	wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc    = m_pWndProc;
	wc.cbClsExtra     = wc.cbWndExtra = 0;
	wc.hInstance      = m_hInstance;
	wc.hIcon          = m_hIcon;
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground  = NULL;
	wc.lpszMenuName   = NULL;
	wc.lpszClassName  = "OpenGLClass1";

	//register the class
	if(!RegisterClass(&wc)) {
		sprintf(m_szErrorMsg, "Failed to register the window class.");
		return false;
	}
	m_bClassRegistered = true;

	int iPixelBPP = 24;

	//fullscreen mode or windowed mode
	if(m_bFullScreen) {
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = m_iWidth;  //selected screen width
		dmScreenSettings.dmPelsHeight = m_iHeight; //selected screen height
		dmScreenSettings.dmBitsPerPel = 32;        //selected bits per pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//try to set selected mode and get results
		int iRes = -1;
		while(dmScreenSettings.dmBitsPerPel>=16) {
			iRes = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
			if(iRes!=DISP_CHANGE_SUCCESSFUL) dmScreenSettings.dmBitsPerPel-=8;
			else break;
		}

		if(iRes!=DISP_CHANGE_SUCCESSFUL) {
			//if the mode fails run in a window
			sprintf(m_szErrorMsg, "The requested fullscreen mode is not supported.\nWindowed mode used instead.");
			m_bFullScreen = false;
			dmScreenSettings.dmBitsPerPel = 24; //reset
		}
		iPixelBPP = dmScreenSettings.dmBitsPerPel;
	}

	if(m_bFullScreen) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle   = WS_POPUP;
		ShowCursor(false);
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle   = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		if(i_bResize) dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
	}
	//this adjustment will make the window exactly the resolution we request
	//normally the borders will overlap parts of our window
	AdjustWindowRectEx(&stWindowRect, dwStyle, false, dwExStyle);

	//create the window
	m_hWnd = CreateWindowEx(dwExStyle, "OpenGLClass1",
		m_szWndTitle, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, 0, 0,
		stWindowRect.right-stWindowRect.left, stWindowRect.bottom-stWindowRect.top,
		NULL, NULL, m_hInstance, NULL);
	if(!m_hWnd) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Window creation error.");
		return false;
	}

	//describes a pixel format
	int iDoubleBuf = PFD_DOUBLEBUFFER;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,                    // version Number
		PFD_DRAW_TO_WINDOW |  // format must support window
		PFD_SUPPORT_OPENGL |  // format must support OpenGL
		(DWORD) iDoubleBuf,           // double buffering?
		PFD_TYPE_RGBA,        // request an RGBA format
		(BYTE) iPixelBPP,            // select color depth
		0, 0, 0, 0, 0, 0,     // color bits ignored
		0,                    // no alpha buffer
		0,                    // shift bit ignored
		0,                    // no accumulation buffer
		0, 0, 0, 0,           // accumulation bits ignored
		0/*16*/,              // 16Bit z-buffer (depth buffer)
		0,                    // no stencil buffer
		0,                    // no auxiliary buffer
		PFD_MAIN_PLANE,       // main drawing layer
		0,                    // reserved
		0, 0, 0               // layer masks ignored
	};

	//try to get an OpenGL device context
	if(!(m_hDC=GetDC(m_hWnd))) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Can't create a GL device context.");
		return false;
	}

	//try to find a suitable pixel format
	if(!(iPixelFormat=ChoosePixelFormat(m_hDC, &pfd))) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Can't find s suitable pixel format.");
		return false;
	}

	//try setting the found pixel format
	if(!SetPixelFormat(m_hDC, iPixelFormat, &pfd)) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Can't set the pixel format.");
		return false;
	}

	//get a rendering context
	if(!(m_hRC=wglCreateContext(m_hDC))) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Can't create a GL rendering context.");
		return false;
	}

	//make the rendering context active
	if(!wglMakeCurrent(m_hDC, m_hRC)) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Can't activate the GL rendering context.");
		return false;
	}

	//show the window, make sure it displays
	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);
	UpdateWindow(m_hWnd);

	DescribePixelFormat(m_hDC, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	m_iPixelFmt = pfd.cColorBits/8; //bytes per pixel
	if(pfd.cRedShift == 0) m_iPixelFmt += 0x10; //RGB
	if(pfd.cGreenBits == 5) m_iPixelFmt += 0x20; //555

#else
	m_pDisplay = XOpenDisplay(NULL);
	if(!m_pDisplay) {
		sprintf(m_szErrorMsg, "Cannot connect to X server.");
		return false;
	}
	m_WinRoot = DefaultRootWindow(m_pDisplay);


	GLint attributes[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 16, None };
	m_pVI = glXChooseVisual(m_pDisplay, 0, attributes);
	if(!m_pVI) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "No appropriate visual found.");
		return false;
	}
	m_CMap = XCreateColormap(m_pDisplay, m_WinRoot, m_pVI->visual, AllocNone);

	XSetWindowAttributes swa;
	swa.colormap = m_CMap;
	swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask;
	m_Win = XCreateWindow(m_pDisplay, m_WinRoot, 0, 0, m_iWidth, m_iHeight, 0, m_pVI->depth, InputOutput, m_pVI->visual, CWColormap | CWEventMask, &swa);

	XMapWindow(m_pDisplay, m_Win);

	if(!i_bResize) {
		XSizeHints hints;
		hints.flags = PMinSize|PMaxSize;
		hints.min_width = m_iWidth;
		hints.max_width = m_iWidth;
		hints.max_height = m_iHeight;
		hints.min_height = m_iHeight;
		XSetNormalHints(m_pDisplay, m_Win, &hints);
	}

	m_GLC = glXCreateContext(m_pDisplay, m_pVI, NULL, GL_TRUE);
	if(!m_GLC) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Create context failed.");
		return false;
	}
	glXMakeCurrent(m_pDisplay, m_Win, m_GLC);

	m_Atom_DeleteWindow = XInternAtom(m_pDisplay, "WM_DELETE_WINDOW", false);
	XSelectInput(m_pDisplay, m_Win, swa.event_mask);
	XSetWMProtocols(m_pDisplay, m_Win, &m_Atom_DeleteWindow, 1);

	//make autorepeat send keypress/keypress/.../keyrelease instead of intervening keyrelease
	XkbSetDetectableAutoRepeat(m_pDisplay, true, NULL);

	//TODO: format can maybe be something else (24bpp or BGR? or even more than 32bpp)
	m_iPixelFmt = _RGBA8888;
#endif

	//default viewarea
	glViewport(0, 0, m_iWidth, m_iHeight);

	C_Image::SetDefaultFormat(_RGBA8888); //make the image class operate with RGBA as in OpenGL

	//set up lighting, textures, and anything else
	if(!InitGL()) {
		KillGLWindow();
		sprintf(m_szErrorMsg, "Initialization failed.");
		return false;
	}

	return true;
}

bool C_GraphWrapperGL::BeginScene(bool i_bClear)
{
	int iClearColorBuffer = i_bClear ? GL_COLOR_BUFFER_BIT : 0; //clear the screen?
	iClearColorBuffer |= GL_DEPTH_BUFFER_BIT; //depth not used in 2d, but clear it anyway
	glClear(iClearColorBuffer);
	glEnable(GL_TEXTURE_2D); //added because on new Mac build this somehow is reset (resulting in all white screen)

	return true;
}

void C_GraphWrapperGL::Flip()
{
	glFinish(); //this does have an effect on smoothness (at least for linux)
	// despite this: http://www.opengl.org/wiki/Common_Mistakes#glFinish_and_glFlush
#if defined WIN32
	if(m_hWnd) SwapBuffers(m_hDC);
#else
	glXSwapBuffers(m_pDisplay, m_Win);
#endif
}

void C_GraphWrapperGL::Set2DOrthoProjectionMode()
{
	S_Rect stViewArea = {0, 0, m_iWidth, m_iHeight};
	//new ortho projection
	glMatrixMode(GL_PROJECTION);  //select the projection matrix
	glLoadIdentity();             //reset the projection matrix
	glOrtho(stViewArea.x, stViewArea.x+stViewArea.width, stViewArea.y, stViewArea.y+stViewArea.height, -1.0f, 1.0f);
	glMatrixMode(GL_TEXTURE); 
	glLoadIdentity();             //no textureops just a plain image
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();             //no translations just render an axis aligned quad

	//set render state
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	glRasterPos2f(0.0f, 0.0f);
}

void C_GraphWrapperGL::Set3DProjectionMode()
{
	//reset projection matrix
	//glMatrixMode(GL_PROJECTION);
	//glLoadMatrixf(afMatProj); ...need to be set externally
	glMatrixMode(GL_MODELVIEW);

	//reset render state
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

//texture and draw functions
GLuint C_GraphWrapperGL::GetTexture(char *i_szFilename, char *i_szResname)
{
	GLuint hTexture = 0;
	S_TextureData stTextureData;

	int i, iSize = (int)m_clTextureList.size();
	for(i=0; i<iSize; i++) {
		S_TextureData *pstTexture = &m_clTextureList[i];
		if(strncmp(pstTexture->szFilename, i_szFilename, TX_MAXNAMESIZE-1) == 0) {
			hTexture = pstTexture->hTexture;
			pstTexture->iRefCount++;
		}
	}

	if(hTexture == 0) { //texture not loaded; load
		stTextureData.iRefCount = 1;
		strncpy(stTextureData.szFilename, i_szFilename, TX_MAXNAMESIZE);
		if(i_szResname) strncpy(stTextureData.szResname, i_szResname, TX_MAXNAMESIZE);
		else stTextureData.szResname[0] = 0;
		stTextureData.szFilename[TX_MAXNAMESIZE-1] = 0; //be sure to nullterminate
		stTextureData.szResname[TX_MAXNAMESIZE-1]  = 0; //be sure to nullterminate

		glGenTextures(1, &hTexture);
		if(LoadTexture(hTexture, i_szFilename, i_szResname)) {
			stTextureData.hTexture = hTexture;
			m_clTextureList.push_back(stTextureData);
		} else { 
			glDeleteTextures(1, &hTexture);
			hTexture = 0;
		}
	}

	return hTexture;
}

void C_GraphWrapperGL::ReloadTextures()
{
	int i, iSize = (int)m_clTextureList.size();
	for(i=0; i<iSize; i++) {
		S_TextureData *pstTexture = &m_clTextureList[i];
		LoadTexture(pstTexture->hTexture, pstTexture->szFilename, pstTexture->szResname);
	}
}

bool C_GraphWrapperGL::LoadTexture(GLuint i_hDstTexture, C_Image *i_pclSrcImg)
{
	int iWidth, iHeight, iPixelFmt;
	uint8_t *pBuf;

	if(i_pclSrcImg) {
		i_pclSrcImg->ExpandToOpenGLCompatibleDim();
		i_pclSrcImg->GetInfo(NULL, NULL, &iPixelFmt);
		iWidth  = i_pclSrcImg->m_iGLTextureW;
		iHeight = i_pclSrcImg->m_iGLTextureH;
		i_pclSrcImg->GetBufferMemory(&pBuf, NULL);

		glBindTexture(GL_TEXTURE_2D, i_hDstTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//filter: GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST (bilinear), GL_LINEAR_MIPMAP_LINEAR (trilinear)

		int iPixelSize = iPixelFmt & 0x0f;
		switch(iPixelSize) {
			case 1: break; //no support
			case 2: break; //not yet
			case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pBuf); break;
			case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBuf); break;
		}
		return true;
	}

	return false;
}

bool C_GraphWrapperGL::LoadTexture(GLuint i_hDstTexture, char* i_szFilename, char* i_szResname)
{
	bool bResult;
	C_Image *pclImg;
	char *szResname = i_szResname;

	if(i_szResname && i_szResname[0]==0) szResname = NULL;
	pclImg = new C_Image(i_szFilename, szResname, &bResult, _RGBA8888);
	if(bResult) bResult = LoadTexture(i_hDstTexture, pclImg);
	delete pclImg;

	return bResult;
}

bool C_GraphWrapperGL::FreeTexture(char *i_szFilename)
{
	bool bReturn = false;

	int i, iSize = (int)m_clTextureList.size();
	for(i=0; i<iSize; i++) {
		S_TextureData *pstTexture = &m_clTextureList[i];
		if(strncmp(pstTexture->szFilename, i_szFilename, TX_MAXNAMESIZE-1) == 0) { //texture found
			bReturn = true;
			pstTexture->iRefCount--;
			/*if(pstTexture->iRefCount == 0) {

			}*/
		}
	}
	return bReturn;
}

bool C_GraphWrapperGL::FreeTexture(GLuint i_hTexture)
{
	bool bReturn = false;

	int i, iSize = (int)m_clTextureList.size();
	for(i=0; i<iSize; i++) {
		S_TextureData *pstTexture = &m_clTextureList[i];
		if(pstTexture->hTexture == i_hTexture) { //texture found
			bReturn = true;
			pstTexture->iRefCount--;
			/*if(pstTexture->iRefCount == 0) {

			}*/
		}
	}
	return bReturn;
}

GLuint C_GraphWrapperGL::LoadTextureAbs(C_Image *i_pclSrcImg)
{
	GLuint hTexture;
	glGenTextures(1, &hTexture);

	if(!LoadTexture(hTexture, i_pclSrcImg)) {
		glDeleteTextures(1, &hTexture);
		hTexture = 0;
	}

	return hTexture;
}

void C_GraphWrapperGL::FreeTextureAbs(GLuint i_hTexture)
{
	glDeleteTextures(1, &i_hTexture);
}

bool C_GraphWrapperGL::Blt(GLuint i_hTexture, int i_iW, int i_iH, int i_iX, int i_iY, bool i_bTransparent)
{
	GLint iWidth, iHeight;

	//set the texture
	glBindTexture(GL_TEXTURE_2D, i_hTexture);

	//get width, height of texture
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight);
	if(iWidth<=0 || iHeight<=0) return false; //texture size not supported
	float maxh = (float)i_iH/iHeight;
	float maxw = (float)i_iW/iWidth;

	if(i_bTransparent) glEnable(GL_BLEND);

	//render a quad at the specified pos, and generate
	//mappingcoords based on the size of the texture
	glColor4fv(&m_stBltColor.r);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, maxh);
	glVertex2i(i_iX, (m_iHeight-i_iY)-i_iH);
	glTexCoord2f(0, 0);
	glVertex2i(i_iX, m_iHeight-i_iY);
	glTexCoord2f(maxw, maxh);
	glVertex2i(i_iX+i_iW, (m_iHeight-i_iY)-i_iH);
	glTexCoord2f(maxw, 0);
	glVertex2i(i_iX+i_iW, m_iHeight-i_iY); 
	glEnd();

	if(i_bTransparent) glDisable(GL_BLEND);

	return true;
}

bool C_GraphWrapperGL::Blt(GLuint i_hTexture, int i_iW, int i_iH, S_Rect *i_pstSrcRect, int i_iX, int i_iY, bool i_bTransparent)
{
	if(i_pstSrcRect==NULL) return Blt(i_hTexture, i_iW, i_iH, i_iX, i_iY, i_bTransparent);

	//set the texture
	glBindTexture(GL_TEXTURE_2D, i_hTexture);

	GLint iWidth, iHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight);
	if(iWidth<=0 || iHeight<=0) return false; //texture size not supported
	float x1 = (float)i_pstSrcRect->x / iWidth;
	float y1 = (float)i_pstSrcRect->y / iHeight;
	float x2 = (float)(i_pstSrcRect->x+i_pstSrcRect->width) / iWidth;
	float y2 = (float)(i_pstSrcRect->y+i_pstSrcRect->height) / iHeight;

	if(i_bTransparent) {
		glEnable(GL_BLEND);
	}

	//render a quad at the specified pos, and generate
	//mappingcoords based on the size of the texture
	glColor4fv(&m_stBltColor.r);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(x1, y2);
	glVertex2i(i_iX, (m_iHeight-i_iY)-i_pstSrcRect->height); 
	glTexCoord2f(x1, y1);
	glVertex2i(i_iX, m_iHeight-i_iY);
	glTexCoord2f(x2, y2);
	glVertex2i(i_iX+i_pstSrcRect->width, (m_iHeight-i_iY)-i_pstSrcRect->height);
	glTexCoord2f(x2, y1);
	glVertex2i(i_iX+i_pstSrcRect->width, m_iHeight-i_iY); 
	glEnd();

	if(i_bTransparent) {
		glDisable(GL_BLEND);
	}

	return true;
}

bool C_GraphWrapperGL::Blt(GLuint i_hTexture, int i_iW, int i_iH, S_Rect *i_pstSrcRect, S_Rect *i_pstDstRect, bool i_bTransparent)
{
	float x1, x2, y1, y2;
	if(i_pstDstRect==NULL) return false;

	//set the texture
	glBindTexture(GL_TEXTURE_2D, i_hTexture);

	GLint iWidth, iHeight;
	//get width, height of texture
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight);
	if(iWidth<=0 || iHeight<=0) return false; //texture size not supported
	if(i_pstSrcRect==NULL) {
		x1 = 0;
		y1 = 0;
		x2 = (float)i_iW/iWidth;
		y2 = (float)i_iH/iHeight;
	} else {
		x1 = (float)i_pstSrcRect->x / iWidth;
		y1 = (float)i_pstSrcRect->y / iHeight;
		x2 = (float)(i_pstSrcRect->x+i_pstSrcRect->width) / iWidth;
		y2 = (float)(i_pstSrcRect->y+i_pstSrcRect->height) / iHeight;
	}

	if(i_bTransparent) glEnable(GL_BLEND);

	//render a quad at the specified pos, and generate
	//mappingcoords based on the size of the texture
	glColor4fv(&m_stBltColor.r);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(x1, y2);
	glVertex2i(i_pstDstRect->x, (m_iHeight-i_pstDstRect->y)-i_pstDstRect->height); 
	glTexCoord2f(x1, y1);
	glVertex2i(i_pstDstRect->x, m_iHeight-i_pstDstRect->y);
	glTexCoord2f(x2, y2);
	glVertex2i(i_pstDstRect->x+i_pstDstRect->width, (m_iHeight-i_pstDstRect->y)-i_pstDstRect->height);
	glTexCoord2f(x2, y1);
	glVertex2i(i_pstDstRect->x+i_pstDstRect->width, m_iHeight-i_pstDstRect->y); 
	glEnd();

	if(i_bTransparent) glDisable(GL_BLEND);

	return true;
}

bool C_GraphWrapperGL::Blt(C_Image *i_pclSrcImg, S_Rect *i_pstSrcRect, int i_iX, int i_iY, bool i_bTransparent)
{
	bool bResult = false;

	if(!i_pclSrcImg) return false;
	if(i_pclSrcImg->m_hTexture==0) i_pclSrcImg->m_hTexture = LoadTextureAbs(i_pclSrcImg);

	if(i_pclSrcImg->m_hTexture) {
		bResult = Blt(i_pclSrcImg->m_hTexture, i_pclSrcImg->m_iWidth, i_pclSrcImg->m_iHeight,
			i_pstSrcRect, i_iX, i_iY, i_bTransparent);
	}
	return bResult;
}

bool C_GraphWrapperGL::Blt(C_Image *i_pclSrcImg, S_Rect *i_pstSrcRect, S_Rect *i_pstDstRect, bool i_bTransparent)
{
	bool bResult = false;

	if(i_pclSrcImg->m_hTexture==0) i_pclSrcImg->m_hTexture = LoadTextureAbs(i_pclSrcImg);

	if(i_pclSrcImg->m_hTexture) {
		bResult = Blt(i_pclSrcImg->m_hTexture, i_pclSrcImg->m_iWidth, i_pclSrcImg->m_iHeight,
			i_pstSrcRect, i_pstDstRect, i_bTransparent);
	}
	return bResult;
}

void C_GraphWrapperGL::SetBltColor(S_FColor *i_pstColor)
{
	if(i_pstColor==NULL) m_stBltColor = S_FCOLORMAKE(1,1,1,1);
	else m_stBltColor = *i_pstColor;
}

void C_GraphWrapperGL::SetColor(S_FColor *i_pstColor)
{
	//change gl color
	glColor4fv(&i_pstColor->r);
}

void C_GraphWrapperGL::Line(int i_iX, int i_iY, int i_iX2, int i_iY2, float i_fWidth, bool i_bTransparent)
{
	GLboolean bTexture = glIsEnabled(GL_TEXTURE_2D);
	if(bTexture) glDisable(GL_TEXTURE_2D);

	if(i_bTransparent) glEnable(GL_BLEND);

	if(i_fWidth!=1.0) glLineWidth(i_fWidth);

	//render a line
	glBegin(GL_LINES);
	glVertex2i(i_iX, m_iHeight-i_iY);
	glVertex2i(i_iX2, m_iHeight-i_iY2);
	glEnd();

	if(i_fWidth!=1.0) glLineWidth(1.0f);

	if(i_bTransparent) glDisable(GL_BLEND);

	if(bTexture) glEnable(GL_TEXTURE_2D);
}

void C_GraphWrapperGL::Rect(S_Rect *i_pstDstRect, bool i_bTransparent)
{
	GLboolean bTexture = glIsEnabled(GL_TEXTURE_2D);
	if(bTexture) glDisable(GL_TEXTURE_2D);

	if(i_bTransparent) glEnable(GL_BLEND);

	S_Rect stDstRect = {0,0, m_iWidth, m_iHeight};
	if(i_pstDstRect) stDstRect = *i_pstDstRect;

	//render a quad
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(stDstRect.x, (m_iHeight-stDstRect.y)-stDstRect.height); 
	glVertex2i(stDstRect.x, m_iHeight-stDstRect.y);
	glVertex2i(stDstRect.x+stDstRect.width, (m_iHeight-stDstRect.y)-stDstRect.height);
	glVertex2i(stDstRect.x+stDstRect.width, m_iHeight-stDstRect.y); 
	glEnd();

	if(i_bTransparent) glDisable(GL_BLEND);

	if(bTexture) glEnable(GL_TEXTURE_2D);
}

void C_GraphWrapperGL::Circle(int x, int y, int i_iRadius, bool i_bTransparent)
{
	GLboolean bTexture = glIsEnabled(GL_TEXTURE_2D);
	if(bTexture) glDisable(GL_TEXTURE_2D);

	if(i_bTransparent) glEnable(GL_BLEND);
	y = (m_iHeight-y); //invert to opengl y coord

	//render a circle in 28 steps
	//glBegin(GL_LINES);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f((float)x, (float)y); //centre
	for(float angle = (float)(2*PI); angle >= -1.0f; angle-=(float)PI/28) { //-1.0f for rounding errors
		glVertex2f((float)(x+ i_iRadius * cos(angle)), (float)(y+ i_iRadius * sin(angle)));
	}
	glEnd();

	if(i_bTransparent) glDisable(GL_BLEND);

	if(bTexture) glEnable(GL_TEXTURE_2D);
}


void C_GraphWrapperGL::SetClippingRect(S_Rect *i_pstRect)
{
	if(i_pstRect) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(i_pstRect->x, m_iHeight-(i_pstRect->y+i_pstRect->height), i_pstRect->width, i_pstRect->height);
	} else {
		glScissor(0, 0, m_iWidth, m_iHeight);
		glDisable(GL_SCISSOR_TEST);
	}
}


#include "gltarga.h"
bool C_GraphWrapperGL::SaveRenderedToTGA(char *i_szFilename, S_Rect *i_pstRect, S_Color *i_pstTranspCol)
{
	bool bResult = false;

	//save image
	C_Image *pImageObj = GetRenderedAsImage(i_pstRect, i_pstTranspCol);
	if(pImageObj) {
		bResult = C_TargaImg::SaveCompressed(i_szFilename, pImageObj, true);
		delete pImageObj;
	}

	return bResult;
}

C_Image *C_GraphWrapperGL::GetRenderedAsImage(S_Rect *i_pstRect, S_Color *i_pstTranspCol)
{
	S_Rect stRect = {0, 0, m_iWidth, m_iHeight}; //full area
	if(i_pstRect) {
		if(i_pstRect->x+i_pstRect->width>m_iWidth || i_pstRect->y+i_pstRect->height>m_iHeight) return NULL;
		stRect = *i_pstRect; //given area
		stRect.y = (m_iHeight-stRect.height)+stRect.y;
	}
	bool bUseTransparent = i_pstTranspCol!=NULL;

	//get image
	glReadBuffer(GL_BACK);
	uint8_t *pData = new uint8_t[stRect.width*stRect.height*(bUseTransparent?4:3)]; //allocate memory for storing the image
	glReadPixels(stRect.x, stRect.y, stRect.width, stRect.height, bUseTransparent?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pData);

	//process image
	C_Image *pImageObj = new C_Image(stRect.width, stRect.height, bUseTransparent?_RGBA8888:_RGB888, pData);
	if(pImageObj) {
		if(bUseTransparent) {
			uint32_t iTransparentCol = i_pstTranspCol->r | (i_pstTranspCol->g<<8) | (i_pstTranspCol->b<<16);
			//set alpha channel to 0 for all pixels in image that match iTransparentCol
			int x, y, iPad;
			AllPtrType pBuf;
			pImageObj->GetBufferMemory(&pBuf.u8ptr);
			pImageObj->GetLineSizeInfo(NULL, NULL, &iPad);
			for(y=0; y<stRect.height; y++) {
				for(x=0; x<stRect.width; x++) {
					if((*pBuf.u32ptr&0x00ffffff)==iTransparentCol) *pBuf.u32ptr = 0x00000000;
					pBuf.u32ptr++;
				}
				*pBuf.u8ptr+=iPad;
			}
		}
		pImageObj->m_bUsermem = false; //make allocated memory belong to the object
	} else delete[] pData;

	return pImageObj;
}
