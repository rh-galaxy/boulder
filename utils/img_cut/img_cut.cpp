
//program to create a set of one or many images from a set of one or many other images.
// all images in a set have the same dimensions.
//
//useful to combine many small images into one larger image,
// or the other way around (split).
//
//all images are saved as rle compressed .tga. input can be .jpeg/.bmp/.tga.

#include "common.h"
#include "image.h"
#include "targa.h"

#ifdef WIN32
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#endif

//from settings file
char g_szInBaseName[256];
char g_szInExtension[32];
int  g_iInBaseIndex, g_iInNumNameDigits;
char g_szOutBaseName[64];
int  g_iInSizeX, g_iInSizeY;
int  g_iInNumX, g_iInNumY;
int  g_iOutSizeX, g_iOutSizeY;
int  g_iOutNumX, g_iOutNumY;
int  g_iFlipXY; //0 or 1

//global read text file helpers
//////////////////////////////////////////////////////////////////////////////////////////
char *GetNextLineAndCommand(FILE *pFile, char *szLine, char * o_szCommand)
{
	o_szCommand[0] = 0; //reset command
	char *szRet = fgets(szLine, 512, pFile);
	if(szRet) sscanf(szLine, "%s", o_szCommand);
	return szRet;
}

void GetWithin(char *io_szString, char cCh)
{
	int i = 0, iStart = 0;

	while(io_szString[i]!=cCh && io_szString[i]!=0) i++;
	i++;

	while(io_szString[i]!=cCh && io_szString[i]!=0) io_szString[iStart++] = io_szString[i++];
	io_szString[iStart] = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////

bool ReadSettings()
{
	//settings.txt example
	//*INPUTBASENAME img
	//*INPUTBASEINDEX "0000"
	//*INPUTFILETYPE ".tga"
	//*INPUTSIZE 500 500
	//*INPUT_NUM_XY 3 3
	//*OUTPUTBASENAME outimg
	//*OUTPUTSIZE 1024 768
	//*FLIPOUTXY 0
	FILE   *pFile;
	char   szLine[512], *pszStr;
	char   szCommand[100];

	int iAllRead = 0;

	pFile = fopen("settings.txt", "rt");
	if(!pFile) return false;

	do {
		pszStr = GetNextLineAndCommand(pFile, szLine, szCommand);

		if(strcmp(szCommand, "*INPUTBASENAME") == 0) {
			GetWithin(szLine, '"');
			if(strlen(szLine)>255) continue;
			strcpy(g_szInBaseName, szLine);
			iAllRead++;
			continue;
		}
		if(strcmp(szCommand, "*INPUTBASEINDEX") == 0) {
			GetWithin(szLine, '"');
			g_iInNumNameDigits = (int)strlen(szLine);
			if(g_iInNumNameDigits>5) continue;
			sscanf(szLine, "%d", &g_iInBaseIndex);
			iAllRead++;
			continue;
		}
		if(strcmp(szCommand, "*INPUTFILETYPE") == 0) {
			GetWithin(szLine, '"');
			if(strlen(szLine)>6) continue;
			strcpy(g_szInExtension, szLine);
			iAllRead++;
			continue;
		}
		if(strcmp(szCommand, "*INPUTSIZE") == 0) {
			sscanf(szLine, "%s %d %d", szCommand, &g_iInSizeX, &g_iInSizeY);
			iAllRead++;         
			continue;
		}
		if(strcmp(szCommand, "*INPUT_NUM_XY") == 0) {
			sscanf(szLine, "%s %d %d", szCommand, &g_iInNumX, &g_iInNumY);
			iAllRead++;
			continue;
		}
		if(strcmp(szCommand, "*OUTPUTBASENAME") == 0) {
			GetWithin(szLine, '"');
			if(strlen(szLine)>63) continue;
			strcpy(g_szOutBaseName, szLine);
			iAllRead++;
			continue;
		}
		if(strcmp(szCommand, "*OUTPUTSIZE") == 0) {
			sscanf(szLine, "%s %d %d", szCommand, &g_iOutSizeX, &g_iOutSizeY);
			iAllRead++;
			continue;
		}
		if(strcmp(szCommand, "*FLIPOUTXY") == 0) {
			sscanf(szLine, "%s %d", szCommand, &g_iFlipXY);
			iAllRead++;
			continue;
		}
	} while(pszStr);

	fclose(pFile);
	if(iAllRead!=8) return false; //not all info loaded

	g_iOutNumX = g_iInSizeX*g_iInNumX;
	g_iOutNumY = g_iInSizeY*g_iInNumY;
	g_iOutNumX = (g_iOutNumX/g_iOutSizeX) + ((g_iOutNumX%g_iOutSizeX)>0?1:0);
	g_iOutNumY = (g_iOutNumY/g_iOutSizeY) + ((g_iOutNumY%g_iOutSizeY)>0?1:0);
	return true;
}

void GetOutputName(int iX, int iY, char *o_szName)
{
	if(g_iFlipXY) {
		int iTmp = iY;
		iY = iX;
		iX = iTmp;
	}
	sprintf(o_szName, "%s%05d_%05d.tga", g_szOutBaseName, iX, iY);
}

void GetInputName(int iX, int iY, char *o_szName)
{
	int iNumber = g_iInBaseIndex+(iY*g_iInNumX)+iX;
	sprintf(o_szName, "%s%0*d%s", g_szInBaseName, g_iInNumNameDigits, iNumber, g_szInExtension);
}

int main(int argc, char *argv[])
{
	printf("img_cut v1.00\n");
	if(!ReadSettings()) {
		printf("error: could not read 'settings.txt'\n");
		return false;
	}

	C_Image::SetDefaultFormat(_BGRA8888);

	char szOutName[MAX_PATH], szInName[MAX_PATH];
	bool bResult;
	int i, j, i2, j2;
	for(j=0; j<g_iOutNumY; j++) {
		for(i=0; i<g_iOutNumX; i++) { //make each output
			int iStartInImgX = (i*g_iOutSizeX)/g_iInSizeX;
			int iStartInImgY = (j*g_iOutSizeY)/g_iInSizeY;
			int iInOffsetX, iStartInOffsetX = (i*g_iOutSizeX)%g_iInSizeX;
			int iInOffsetY = (j*g_iOutSizeY)%g_iInSizeY;

			int iOutOffsetX = 0, iOutOffsetY = 0;
			C_Image *pclOutImg = new C_Image(g_iOutSizeX, g_iOutSizeY, _DEFAULT);
			pclOutImg->FillWithAbsoluteColor(NULL, 0);
			for(j2=iStartInImgY; j2<g_iInNumY; j2++) {
				iOutOffsetX = 0;
				iInOffsetX = iStartInOffsetX;
				for(i2=iStartInImgX; i2<g_iInNumX; i2++) { //read and paste each input used for this output
					//read inimage
					GetInputName(i2, j2, szInName);
					C_Image *pclInImg = new C_Image(szInName, NULL, &bResult, _DEFAULT);
					if(!bResult) {
						printf("error: could not read input image (%s)\n", szInName);
						return false;
					}

					//put pclInImg to pclOutImg at iOutOffset (and clipped to border of pclOutImg)
					S_Rect stSrcRect = {iInOffsetX, iInOffsetY, g_iInSizeX-iInOffsetX, g_iInSizeY-iInOffsetY};
					int iSpaceX = g_iOutSizeX-iOutOffsetX;
					int iSpaceY = g_iOutSizeY-iOutOffsetY;
					if(iSpaceX<stSrcRect.width) stSrcRect.width = iSpaceX;
					if(iSpaceY<stSrcRect.height) stSrcRect.height = iSpaceY;
					pclOutImg->Blt2(pclInImg, &stSrcRect, iOutOffsetX, iOutOffsetY);
					delete pclInImg;

					iOutOffsetX+=stSrcRect.width;
					if(iOutOffsetX>=g_iOutSizeX) break;
					iInOffsetX=0;
				}
				iOutOffsetY+=g_iInSizeY-iInOffsetY;
				if(iOutOffsetY>=g_iOutSizeY) break;
				iInOffsetY=0;
			}

			//save image
			GetOutputName(i, j, szOutName);
			C_TargaImg::SaveCompressed(szOutName, pclOutImg);
			delete pclOutImg;
		}
	}

	return true;
}
