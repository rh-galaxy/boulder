// C_GraphWrapper

//class to init and use SDL on different OS
// supports windows, mac and linux.

#include "graph.h"
#include <math.h>

C_GraphWrapper* C_GraphWrapper::s_pTheGraph = NULL;

SDL_Renderer* C_GraphWrapper::m_pRenderer = NULL;

C_GraphWrapper::C_GraphWrapper() : m_pWindow(NULL)
{
	m_bFullScreen = false;
	m_szErrorMsg[0] = 0;

	s_pTheGraph = this;

	SetBltColor(NULL);
}

C_GraphWrapper::~C_GraphWrapper()
{
	s_pTheGraph = NULL;

	int i, iSize = (int)m_clTextureList.size();
	for (i = 0; i < iSize; i++) {
		S_TextureData* pstTexture = &m_clTextureList[i];
		FreeTextureAbs(pstTexture->hTexture);
	}

	KillWindow();
}

void C_GraphWrapper::SetWindowTitle(const char* szWndTitle)
{
	//set the title
	if (m_pWindow) SDL_SetWindowTitle(m_pWindow, szWndTitle);
}

//properly kill the window
bool C_GraphWrapper::KillWindow()
{
	bool bResult = true;

	if (m_pWindow)
	{
		SDL_DestroyWindow(m_pWindow);
		SDL_Quit();
	}

	m_bFullScreen = false;

	return bResult;
}

RESIZECALLBACKTYPE C_GraphWrapper::s_pResizeCallback = NULL;
int C_GraphWrapper::resizingEventWatcher(void* data, SDL_Event* event)
{
	if (s_pResizeCallback == NULL) return 0;

	if (event->type == SDL_WINDOWEVENT &&
		event->window.event == SDL_WINDOWEVENT_RESIZED) {
		SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
		if (win == (SDL_Window*)data) {
			int w, h;
			SDL_GetWindowSize(win, &w, &h);
			GetGraphWrapperObject()->SetSize(w, h);
			s_pResizeCallback();
		}
	}
	return 0;
}
void C_GraphWrapper::SetLiveResizeCallback(RESIZECALLBACKTYPE call)
{
	s_pResizeCallback = call;
}

bool C_GraphWrapper::SetSize(int iWidth, int iHeight)
{
	if (m_bFullScreen) return false;

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	return true;
}

bool C_GraphWrapper::SetMode(int iWidth, int iHeight, bool bFullScreen, bool bResize)
{
	//make sure no mode is set
	KillWindow();

	m_bFullScreen = bFullScreen;
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		sprintf(m_szErrorMsg, "Unable to initialize SDL");
		return false;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	if (!(m_pWindow = SDL_CreateWindow("SDL UNTITLED", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		m_iWidth, m_iHeight, (bResize ? SDL_WINDOW_RESIZABLE : 0) | SDL_WINDOW_SHOWN)))
	{
		sprintf(m_szErrorMsg, "Unable to create SDL window");
		return false;
	}

	SDL_AddEventWatch(resizingEventWatcher, m_pWindow);

	m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//Checking for SDL initialization error
	/*const char* sdl_error = SDL_GetError();
	if (*sdl_error != '\0') {
		sprintf(m_szErrorMsg, "SDL Error: %s", sdl_error);
		return false;
	}*/ //on linux we get "That operation is not supported" - but it still works

	C_Image::SetDefaultFormat(_RGBA8888); //make the image class operate with RGBA as in SDL

	return true;
}

void C_GraphWrapper::Flip()
{
	//Update screen
	SDL_RenderPresent(m_pRenderer);
	SDL_RenderClear(m_pRenderer);
}

//texture and draw functions
SDL_Texture* C_GraphWrapper::GetTexture(char* szFilename, char* szResname)
{
	SDL_Texture* hTexture = NULL;
	S_TextureData stTextureData;

	int i, iSize = (int)m_clTextureList.size();
	for (i = 0; i < iSize; i++) {
		S_TextureData* pstTexture = &m_clTextureList[i];
		if (strncmp(pstTexture->szFilename, szFilename, TX_MAXNAMESIZE - 1) == 0) {
			hTexture = pstTexture->hTexture;
			pstTexture->iRefCount++;
		}
	}

	if (hTexture == NULL) { //texture not loaded; load
		stTextureData.iRefCount = 1;
		strncpy(stTextureData.szFilename, szFilename, TX_MAXNAMESIZE);

		if (szResname)
			strncpy(stTextureData.szResname, szResname, TX_MAXNAMESIZE);
		else
			stTextureData.szResname[0] = 0;

		stTextureData.szFilename[TX_MAXNAMESIZE - 1] = 0; //be sure to NULLterminate
		stTextureData.szResname[TX_MAXNAMESIZE - 1] = 0; //be sure to NULLterminate

		if ((hTexture = LoadTexture(szFilename, szResname)) != NULL)
		{
			stTextureData.hTexture = hTexture;
			m_clTextureList.push_back(stTextureData);
		}
	}

	return hTexture;
}

void C_GraphWrapper::ReloadTextures()
{
	int i, iSize = (int)m_clTextureList.size();
	for (i = 0; i < iSize; i++) {
		S_TextureData* pstTexture = &m_clTextureList[i];
		if (pstTexture->hTexture)
			SDL_DestroyTexture(pstTexture->hTexture);
		pstTexture->hTexture = LoadTexture(pstTexture->szFilename, pstTexture->szResname);
	}
}

SDL_Texture* C_GraphWrapper::LoadTexture(C_Image* pSrcImg)
{
	SDL_Texture* texture = NULL;
	SDL_Surface* surf8;

	int iWidth, iHeight, iPixelFmt;
	uint8_t* pBuf;

	if (pSrcImg) {
		pSrcImg->ExpandToCompatibleDim();
		pSrcImg->GetInfo(NULL, NULL, &iPixelFmt);
		iWidth = pSrcImg->m_iTextureW;
		iHeight = pSrcImg->m_iTextureH;
		pSrcImg->GetBufferMemory(&pBuf, NULL);

		int iPixelSize = iPixelFmt & 0x0f;

		// fprintf(stdout, "param: %d %d %d\n", iWidth, iHeight, iPixelSize);

		surf8 = NULL;
		switch (iPixelSize) {
		case 1: break; //no support
		case 2: break; //not yet
		case 3:
			surf8 = SDL_CreateRGBSurfaceWithFormatFrom((void*)pBuf, iWidth, iHeight, 24, iWidth * 3, SDL_PIXELFORMAT_RGB24);
			break;
		case 4:
			surf8 = SDL_CreateRGBSurfaceWithFormatFrom((void*)pBuf, iWidth, iHeight, 32, iWidth * 4, SDL_PIXELFORMAT_RGBA32);
			break;
		}

		if (surf8)
		{
			texture = SDL_CreateTextureFromSurface(m_pRenderer, surf8);
			SDL_FreeSurface(surf8);
		}
		else
		{
			//const char *sdl_error = SDL_GetError();
			//if(*sdl_error != '\0')
			//	sprintf(m_szErrorMsg, "%d %d SDL Error: %s", iWidth, iHeight, sdl_error);
		}
	}

	return texture;
}

SDL_Texture* C_GraphWrapper::LoadTexture(char* szFilename, char* szResname)
{
	bool bResult;
	C_Image* pImg;
	SDL_Texture* texture = NULL;
	char* szResname2 = szResname;

	if (szResname && szResname[0] == 0)
		szResname2 = NULL;

	pImg = new C_Image(szFilename, szResname2, &bResult, _RGBA8888);

	if (bResult)
		texture = LoadTexture(pImg);

	delete pImg;

	return texture;
}

SDL_Texture* C_GraphWrapper::LoadTextureAbs(C_Image* pSrcImg)
{
	return LoadTexture(pSrcImg);
}

void C_GraphWrapper::FreeTextureAbs(SDL_Texture* hTexture)
{
	SDL_DestroyTexture(hTexture);
}

bool C_GraphWrapper::Blt(SDL_Texture* hTexture, int iW, int iH, int iX, int iY, bool bTransparent)
{
	int iWidth, iHeight;

	if (SDL_QueryTexture(hTexture, NULL, NULL, &iWidth, &iHeight) < 0)
	{
		return false;
	}
	if (iWidth <= 0 || iHeight <= 0)
		return false; //texture size not supported

	float maxh = (float)iH / iHeight;
	float maxw = (float)iW / iWidth;

	if (bTransparent) SDL_SetTextureBlendMode(hTexture, SDL_BLENDMODE_BLEND);
	else SDL_SetTextureBlendMode(hTexture, SDL_BLENDMODE_NONE);

	SDL_SetTextureAlphaMod(hTexture, Uint8(m_stBltColor.a * 255));
	SDL_SetTextureColorMod(hTexture, Uint8(m_stBltColor.r * 255), Uint8(m_stBltColor.g * 255), Uint8(m_stBltColor.b * 255));

	SDL_Rect dst_r, src_r;
	src_r.x = 0;
	src_r.y = 0;
	src_r.w = iW;
	src_r.h = iH;
	dst_r.x = iX;
	dst_r.y = iY;
	dst_r.w = iW;
	dst_r.h = iH;

	SDL_RenderSetViewport(m_pRenderer, NULL);
	//Render texture to screen
	SDL_RenderCopy(m_pRenderer, hTexture, &src_r, &dst_r);

	return true;
}

bool C_GraphWrapper::Blt(SDL_Texture* hTexture, int iW, int iH, S_Rect* pSrcRect, int iX, int iY, bool bTransparent)
{
	if (pSrcRect == NULL)
		return Blt(hTexture, iW, iH, iX, iY, bTransparent);

	int iWidth, iHeight;
	uint8_t alpha = 0;

	if (SDL_QueryTexture(hTexture, NULL, NULL, &iWidth, &iHeight) < 0)
	{
		return false;
	}
	if (iWidth <= 0 || iHeight <= 0)
		return false; //texture size not supported

	if (bTransparent) SDL_SetTextureBlendMode(hTexture, SDL_BLENDMODE_BLEND);
	else SDL_SetTextureBlendMode(hTexture, SDL_BLENDMODE_NONE);

	SDL_SetTextureAlphaMod(hTexture, Uint8(m_stBltColor.a * 255));
	SDL_SetTextureColorMod(hTexture, Uint8(m_stBltColor.r * 255), Uint8(m_stBltColor.g * 255), Uint8(m_stBltColor.b * 255));

	SDL_Rect dst_r, src_r;
	src_r.x = pSrcRect->x;
	src_r.y = pSrcRect->y;
	src_r.w = pSrcRect->width;
	src_r.h = pSrcRect->height;
	dst_r.x = iX;
	dst_r.y = iY;
	dst_r.w = pSrcRect->width;
	dst_r.h = pSrcRect->height;

	SDL_RenderSetViewport(m_pRenderer, NULL);
	//Render texture to screen
	SDL_RenderCopy(m_pRenderer, hTexture, &src_r, &dst_r);

	return true;
}

bool C_GraphWrapper::Blt(SDL_Texture* hTexture, int iW, int iH, S_Rect* pSrcRect, S_Rect* pDstRect, bool bTransparent)
{
	if (pDstRect == NULL)
		return false;

	int iWidth, iHeight;
	uint8_t alpha = 0;

	if (SDL_QueryTexture(hTexture, NULL, NULL, &iWidth, &iHeight) < 0)
	{
		return false;
	}
	if (iWidth <= 0 || iHeight <= 0)
		return false; //texture size not supported

	if (bTransparent) SDL_SetTextureBlendMode(hTexture, SDL_BLENDMODE_BLEND);
	else SDL_SetTextureBlendMode(hTexture, SDL_BLENDMODE_NONE);

	SDL_SetTextureAlphaMod(hTexture, Uint8(m_stBltColor.a * 255));
	SDL_SetTextureColorMod(hTexture, Uint8(m_stBltColor.r * 255), Uint8(m_stBltColor.g * 255), Uint8(m_stBltColor.b * 255));

	SDL_Rect dst_r, src_r;
	if (pSrcRect) {
		src_r.x = pSrcRect->x;
		src_r.y = pSrcRect->y;
		src_r.w = pSrcRect->width;
		src_r.h = pSrcRect->height;
	}
	else {
		src_r.x = 0;
		src_r.y = 0;
		src_r.w = iW;
		src_r.h = iH;
	}
	dst_r.x = pDstRect->x;
	dst_r.y = pDstRect->y;
	dst_r.w = pDstRect->width;
	dst_r.h = pDstRect->height;

	SDL_RenderSetViewport(m_pRenderer, NULL);
	//Render texture to screen
	SDL_RenderCopy(m_pRenderer, hTexture, &src_r, &dst_r);

	return true;
}

bool C_GraphWrapper::Blt(C_Image* pSrcImg, S_Rect* pSrcRect, int iX, int iY, bool bTransparent)
{
	bool bResult = false;

	if (!pSrcImg) return false;
	if (pSrcImg->m_hTexture == 0) pSrcImg->m_hTexture = LoadTextureAbs(pSrcImg);

	if (pSrcImg->m_hTexture) {
		bResult = Blt(pSrcImg->m_hTexture, pSrcImg->m_iWidth, pSrcImg->m_iHeight,
			pSrcRect, iX, iY, bTransparent);
	}
	return bResult;
}

bool C_GraphWrapper::Blt(C_Image* pSrcImg, S_Rect* pSrcRect, S_Rect* pDstRect, bool bTransparent)
{
	bool bResult = false;

	if (pSrcImg->m_hTexture == 0) pSrcImg->m_hTexture = LoadTextureAbs(pSrcImg);

	if (pSrcImg->m_hTexture) {
		bResult = Blt(pSrcImg->m_hTexture, pSrcImg->m_iWidth, pSrcImg->m_iHeight,
			pSrcRect, pDstRect, bTransparent);
	}
	return bResult;
}

void C_GraphWrapper::SetBltColor(S_FColor* pColor)
{
	if (pColor == NULL) m_stBltColor = S_FCOLORMAKE(1, 1, 1, 1);
	else m_stBltColor = *pColor;
}

void C_GraphWrapper::SetColor(S_FColor* pColor)
{
	//change draw color
	if (pColor == NULL) m_stDrawColor = S_FCOLORMAKE(1, 1, 1, 1);
	else m_stDrawColor = *pColor;
	SDL_SetRenderDrawColor(m_pRenderer, Uint8(m_stDrawColor.r * 255), Uint8(m_stDrawColor.g * 255), Uint8(m_stDrawColor.b * 255), Uint8(m_stDrawColor.a * 255));
}

void C_GraphWrapper::Line(int iX, int iY, int iX2, int iY2, float fWidth, bool bTransparent)
{
	if (bTransparent) SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);
	else SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_NONE);
	SDL_RenderDrawLine(m_pRenderer, iX, iY, iX2, iY2);
}

void C_GraphWrapper::Rect(S_Rect* pDstRect, bool bTransparent)
{
	if (bTransparent) SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);
	else SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_NONE);
	if (pDstRect) {
		SDL_Rect r = { pDstRect->x, pDstRect->y, pDstRect->width, pDstRect->height };
		SDL_RenderFillRect(m_pRenderer, &r);
	}
	else SDL_RenderFillRect(m_pRenderer, NULL);
}

void C_GraphWrapper::Circle(int x, int y, int iRadius, bool bTransparent)
{
	SDL_Color c = { Uint8(m_stDrawColor.r * 255), Uint8(m_stDrawColor.g * 255), Uint8(m_stDrawColor.b * 255), Uint8(m_stDrawColor.a * 255) };
	SDL_Vertex v[29];
	int i = 0;
	int idx[3 * 28];
	v[0].color = c;
	v[0].position.x = (float)x;
	v[0].position.y = (float)y;
	v[0].tex_coord.x = v[0].tex_coord.y = 0.0f;
	for (float angle = (float)(2 * PI); angle >= 0.0f; angle -= (float)(2 * PI) / 28.0f) {
		v[i + 1].color = c;
		v[i + 1].position.x = (float)(x + iRadius * cos(angle));
		v[i + 1].position.y = (float)(y + iRadius * sin(angle));
		v[i + 1].tex_coord.x = v[0].tex_coord.y = 0.0f;
		idx[i * 3 + 0] = 0;
		idx[i * 3 + 1] = i + 1;
		idx[i * 3 + 2] = i + 2;
		i++;
	}
	idx[3 * 28 - 1] = 1;

	SDL_RenderGeometry(m_pRenderer, NULL, &v[0], 29, idx, 3 * 28);
}

void C_GraphWrapper::SetClippingRect(S_Rect* pRect)
{
	if (pRect) {
		SDL_Rect dst_r;
		dst_r.x = pRect->x;
		dst_r.y = pRect->y;
		dst_r.w = pRect->width;
		dst_r.h = pRect->height;

		SDL_RenderSetClipRect(m_pRenderer, &dst_r);
	}
	else {
		SDL_RenderSetClipRect(m_pRenderer, NULL);
	}
}


#include "targa.h"
bool C_GraphWrapper::SaveRenderedToTGA(char* szFilename, S_Rect* pRect, S_Color* pTranspCol)
{
	bool bResult = false;

	//save image
	C_Image* pImageObj = GetRenderedAsImage(pRect, pTranspCol);
	if (pImageObj) {
		bResult = C_TargaImg::SaveCompressed(szFilename, pImageObj, true);
		delete pImageObj;
	}

	return bResult;
}

C_Image* C_GraphWrapper::GetRenderedAsImage(S_Rect* pRect, S_Color* pTranspCol)
{
	S_Rect stRect = { 0, 0, m_iWidth, m_iHeight }; //full area
	if (pRect) {
		if (pRect->x + pRect->width > m_iWidth || pRect->y + pRect->height > m_iHeight) return NULL;
		stRect = *pRect; //given area
		stRect.y = (m_iHeight - stRect.height) + stRect.y;
	}
	bool bUseTransparent = pTranspCol != NULL;

	//get image
	uint8_t* pData = new uint8_t[stRect.width * stRect.height * (bUseTransparent ? 4 : 3)]; //allocate memory for storing the image
	SDL_Rect r = { stRect.x, stRect.y, stRect.width, stRect.height };
	SDL_RenderReadPixels(m_pRenderer, &r, bUseTransparent ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24, pData, r.w * (bUseTransparent ? 4 : 3));

	//process image
	C_Image* pImageObj = new C_Image(stRect.width, stRect.height, bUseTransparent ? _RGBA8888 : _RGB888, pData);
	if (pImageObj) {
		if (bUseTransparent) {
			uint32_t iTransparentCol = pTranspCol->r | (pTranspCol->g << 8) | (pTranspCol->b << 16);
			//set alpha channel to 0 for all pixels in image that match iTransparentCol
			int x, y, iPad;
			AllPtrType pBuf;
			pImageObj->GetBufferMemory(&pBuf.u8ptr);
			pImageObj->GetLineSizeInfo(NULL, NULL, &iPad);
			for (y = 0; y < stRect.height; y++) {
				for (x = 0; x < stRect.width; x++) {
					if ((*pBuf.u32ptr & 0x00ffffff) == iTransparentCol) *pBuf.u32ptr = 0x00000000;
					pBuf.u32ptr++;
				}
				*pBuf.u8ptr += iPad;
			}
		}
		pImageObj->m_bUsermem = false; //make allocated memory belong to the object
	}
	else delete[] pData;

	return pImageObj;
}
