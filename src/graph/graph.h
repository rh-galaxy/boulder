#pragma once

#include <SDL.h>
#include <vector>

#include "image.h"

#ifdef WIN32
#pragma comment(lib, "SDL2.lib")
#endif

#define TX_MAXNAMESIZE 512
struct S_TextureData
{
	char        szFilename[TX_MAXNAMESIZE];
	char        szResname[TX_MAXNAMESIZE];
	SDL_Texture* hTexture;
	int         iRefCount;
};
typedef std::vector<S_TextureData> C_TextureList;

typedef void((*RESIZECALLBACKTYPE)(void));
class C_GraphWrapper
{
public:
	C_GraphWrapper();
	~C_GraphWrapper();

	const char* GetErrorMsg() { return m_szErrorMsg; };

	//init
	void SetWindowTitle(const char* szWndTitle);
	bool SetMode(int iWidth, int iHeight, bool bFullScreen = false, bool bResize = false);
	bool SetSize(int iWidth, int iHeight); //should be called when the drawing area changes (on resize)
	int GetModeHeight() { return m_iHeight; };
	int GetModeWidth() { return m_iWidth; };
	int GetModeFmt() { return m_iPixelFmt; };
	void SetLiveResizeCallback(RESIZECALLBACKTYPE call);

	//control
	static C_GraphWrapper* GetGraphWrapperObject() { return s_pTheGraph; };
	void Flip(); //to display a rendered frame

	//textures and draw
	SDL_Texture* GetTexture(char* szFilename, char* szResname = NULL); //texture will be loaded if not already loaded, inc refcount
	void ReloadTextures(); //reloads all textures from disk without changing the refcount (do this when the textures are lost because of graphmode switching)

	static SDL_Texture* LoadTextureAbs(C_Image* pSrcImg);
	static void FreeTextureAbs(SDL_Texture* hTexture);

	void SetBltColor(S_FColor* pColor);
	bool Blt(C_Image* pSrcImg, S_Rect* pSrcRect, int iX, int iY, bool bTransparent = false);
	bool Blt(C_Image* pSrcImg, S_Rect* pSrcRect, S_Rect* pDstRect, bool bTransparent = false);
	//renders a previously loaded texture modulated with the current SetBltColor() onto screen

	//for blt between images in memory use C_Image::Blt2()

	//draw 2d
	void SetColor(S_FColor* pColor);
	void Line(int iX, int iY, int iX2, int iY2, float fWidth = 1.0f, bool bTransparent = false);
	void Rect(S_Rect* pDstRect, bool bTransparent = false);
	void Circle(int x, int y, int iRadius, bool bTransparent = false);

	void SetClippingRect(S_Rect* pRect);

	//save screenshot (to be done after flip)
	C_Image* GetRenderedAsImage(S_Rect* pRect = NULL, S_Color* pTranspCol = NULL);
	bool SaveRenderedToTGA(char* szFilename, S_Rect* pRect = NULL, S_Color* pTranspCol = NULL);
private:
	//os
	SDL_Window* m_pWindow;
	static SDL_Renderer* m_pRenderer;

	static int resizingEventWatcher(void* data, SDL_Event* event);
	static RESIZECALLBACKTYPE s_pResizeCallback;

	static C_GraphWrapper* s_pTheGraph;

	//info and state var
	int  m_iWidth, m_iHeight, m_iPixelFmt; //see image.h
	bool m_bFullScreen;
	char m_szErrorMsg[256];

	//private helpers
	bool KillWindow();

	static SDL_Texture* LoadTexture(C_Image* pSrcImg);
	static SDL_Texture* LoadTexture(char* szFilename, char* szResname);

	C_TextureList m_clTextureList;

	bool Blt(SDL_Texture* hTexture, int iW, int iH, int iX, int iY, bool bTransparent = false);
	bool Blt(SDL_Texture* hTexture, int iW, int iH, S_Rect* pSrcRect, int iX, int iY, bool bTransparent = false);
	bool Blt(SDL_Texture* hTexture, int iW, int iH, S_Rect* pSrcRect, S_Rect* pDstRect, bool bTransparent = false);
	S_FColor m_stBltColor;
	S_FColor m_stDrawColor;
};
