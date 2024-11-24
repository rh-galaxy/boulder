// C_GraphWrapperGL

#ifndef _GLGRAPH_H
#define _GLGRAPH_H

#if defined WIN32
	#include <windows.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#else
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/Xatom.h>
	#include <X11/XKBlib.h>
	#include <X11/keysym.h>

	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glx.h>
#endif
#include <vector>

#include "glimage.h"


#define TX_MAXNAMESIZE 256
struct S_TextureData
{
	char     szFilename[TX_MAXNAMESIZE];
	char     szResname[TX_MAXNAMESIZE];
	uint32_t hTexture;
	int      iRefCount;
};
typedef std::vector<S_TextureData> C_TextureList;


class C_GraphWrapperGL
{
public:
#ifdef WIN32
	C_GraphWrapperGL(WNDPROC i_pWndProc, HICON i_hIcon);
	HWND GetWindow() {return m_hWnd;};
	HDC  GetWindowDC() {return m_hDC;};
#else
	C_GraphWrapperGL();
	Display *GetDisplay() {return m_pDisplay;};
	Window GetWindow(int i_iNum = 1) { \
		if(i_iNum==0) return m_WinRoot; \
		if(i_iNum==1) return m_Win; return 0;};
	Atom m_Atom_DeleteWindow;
#endif
	~C_GraphWrapperGL();

	char *GetErrorMsg() {return m_szErrorMsg;};

	//init
	void SetWindowTitle(char *szWndTitle);
	bool SetMode(int i_iWidth, int i_iHeight, bool i_bFullScreen = false, bool i_bResize = false);
	bool SetSize(int i_iWidth, int i_iHeight); //should be called when the drawing area changes (on resize)
	int GetModeHeight() {return m_iHeight;};
	int GetModeWidth() {return m_iWidth;};
	int GetModeFmt() {return m_iPixelFmt;};

	//control
	static C_GraphWrapperGL *GetGraphWrapperObject() {return s_pTheGLGraph;};
	void Set2DOrthoProjectionMode();
	void Set3DProjectionMode();
	bool BeginScene(bool i_bClear = true); //should be run before any drawing/rendering-operations (every frame)
	void Flip(); //to display a rendered frame

	//textures and draw
	GLuint GetTexture(char *i_szFilename, char *i_szResname = NULL); //texture will be loaded if not already loaded, inc refcount
	bool FreeTexture(char *i_szFilename); //dec refcount for the texture. if it reaches 0 the texture can be freed
	bool FreeTexture(GLuint i_hTexture);
	void ReloadTextures(); //reloads all textures from disk without changing the refcount (do this when the textures are lost because of graphmode switching)

	static GLuint LoadTextureAbs(C_Image *i_pSrcImg);
	static void FreeTextureAbs(GLuint i_hTexture);

	void SetBltColor(S_FColor *i_pstColor);
	bool Blt(C_Image *i_pSrcImg, S_Rect *i_pstSrcRect, int i_iX, int i_iY, bool i_bTransparent = false);
	bool Blt(C_Image *i_pSrcImg, S_Rect *i_pstSrcRect, S_Rect *i_pstDstRect, bool i_bTransparent = false);
	//renders a previously loaded texture modulated with the current glColor() onto screen
	//the projection must be in ortho mode. i_iX, i_iY is the UPPER left corner.

	//for blt between images in memory use C_Image::Blt2()

	//draw 2d
	void SetColor(S_FColor *i_pstColor);
	void Line(int i_iX, int i_iY, int i_iX2, int i_iY2, float i_fWidth = 1.0f, bool i_bTransparent = false);
	void Rect(S_Rect *i_pstDstRect, bool i_bTransparent = false);
	void Circle(int x, int y, int i_iRadius, bool i_bTransparent = false);

	void SetClippingRect(S_Rect *i_pstRect);

	//save screenshot (to be done after flip)
	C_Image *GetRenderedAsImage(S_Rect *i_pstRect = NULL, S_Color *i_pstTranspCol = NULL);
	bool SaveRenderedToTGA(char *i_szFilename, S_Rect *i_pstRect = NULL, S_Color *i_pstTranspCol = NULL);

	void StateTexturing(bool bEnable);
	void StateLighting(bool bEnable);
	void StateWireframe(bool bEnable);
private:
	//os
#if defined WIN32
	HINSTANCE             m_hInstance;
	WNDPROC               m_pWndProc;
	HWND                  m_hWnd;
	HGLRC                 m_hRC;       // permanent rendering context
	HDC                   m_hDC;       // private GDI device context
	HICON                 m_hIcon;
	bool                  m_bClassRegistered;
#else
	Display               *m_pDisplay;
	Window                m_WinRoot;
	Window                m_Win;
	Colormap              m_CMap;
	XVisualInfo           *m_pVI;
	GLXContext            m_GLC;
#endif
	char                  m_szWndTitle[200];

	static C_GraphWrapperGL *s_pTheGLGraph; //there can only be one GLGraph and this is it

	//info and state var
	int  m_iWidth, m_iHeight, m_iPixelFmt; //see image.h
	bool m_bFullScreen;
	char m_szErrorMsg[256];

	//private helpers
	bool InitGL();
	bool KillGLWindow();

	static bool LoadTexture(GLuint i_hDstTexture, C_Image *i_pSrcImg);
	bool LoadTexture(GLuint i_hDstTexture, char *i_szFilename, char *i_szResname);
	C_TextureList m_clTextureList;

	bool Blt(GLuint i_hTexture, int i_iW, int i_iH, int i_iX, int i_iY, bool i_bTransparent = false);
	bool Blt(GLuint i_hTexture, int i_iW, int i_iH, S_Rect *i_pstSrcRect, int i_iX, int i_iY, bool i_bTransparent = false);
	bool Blt(GLuint i_hTexture, int i_iW, int i_iH, S_Rect *i_pstSrcRect, S_Rect *i_pstDstRect, bool i_bTransparent = false);
	S_FColor m_stBltColor;
};

#endif
