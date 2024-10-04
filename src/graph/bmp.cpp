// C_BmpImg

//class to load .bmp images

#include "bmp.h"
#include "fileresource.h"

#pragma pack(1)
struct S_BmpHeader
{
	uint8_t   TypeB;         // signature - 'BM'
	uint8_t   TypeM;         // ..
	uint32_t  Size;          // file size in bytes
	uint16_t  Reserved1;     // 0
	uint16_t  Reserved2;     // 0
	uint32_t  OffBits;       // offset to bitmap
	uint32_t  StructSize;    // size of this struct (40)
	uint32_t  Width;         // width in pixels
	uint32_t  Height;        // height in pixels
	uint16_t  Planes;        // num planes - always 1
	uint16_t  BitCount;      // bits per pixel
	uint32_t  Compression;   // compression flag
	uint32_t  SizeImage;     // image size in bytes
	int32_t   XPelsPerMeter; // horizontal resolution
	int32_t   YPelsPerMeter; // vertical resolution
	uint32_t  ClrUsed;       // 0 -> color table size
	uint32_t  ClrImportant;  // important color count
};
 
#pragma pack()

C_Image *C_BmpImg::Load(const char *szFilename, const char *szResname)
{
	if(!szFilename && !szResname) return NULL;

	S_BmpHeader   stHeader;              //the header
	uint8_t       aPal[1024];            //pal in bmp
	C_Image       *pImg = NULL;        //the image to fill

	C_Resource    *pResource;

	uint32_t      iBytesPerPixel;        //number of bytes per pixel (1,2,3,4)
	int           iPixelformat;
	int y, iWidth, iHeight, iWidthBytes, iLineBytes, iBmpPadBytes;
	uint8_t *pBuf;

	pResource = new C_Resource();
	pResource->SetFilename(szFilename, szResname); //open file
	if(!pResource->Read(&stHeader, sizeof(S_BmpHeader))) goto error; //read header

	//check that the bmp is valid
	iBytesPerPixel = stHeader.BitCount/8;
	if(stHeader.TypeB!='B' || stHeader.TypeM!='M') goto error;
	if(stHeader.StructSize!=40) goto error;
	if(iBytesPerPixel<1 || iBytesPerPixel>4) goto error;
	if(stHeader.Compression!=0) goto error;   

	//create the destination image
	iPixelformat = iBytesPerPixel;
	if(iBytesPerPixel == 1) iPixelformat = _PAL8;
	iWidth  = stHeader.Width;
	iHeight = stHeader.Height;
	iBmpPadBytes = (((iWidth*iBytesPerPixel+3)&(~0x3)))-(iWidth*iBytesPerPixel);
	pImg = new C_Image(iWidth, iHeight, iPixelformat);
	pImg->GetBufferMemory(&pBuf);
	pImg->GetLineSizeInfo(&iWidthBytes, &iLineBytes, NULL);

	//get palette
	if(stHeader.ClrUsed) {
		if(stHeader.ClrUsed==256) {
			if(!pResource->Read(aPal, stHeader.ClrUsed*4)) goto error; //read pal
			pImg->SetPal(aPal);
		} else pResource->Seek(stHeader.ClrUsed*4, SEEK_CUR); //skip
	}

	//get image
	pBuf += iHeight*iLineBytes;
	for(y=0; y<iHeight; y++) {
		pBuf -= iLineBytes;
		pResource->Read(pBuf, iWidthBytes);
		if(iBmpPadBytes) pResource->Seek(iBmpPadBytes, SEEK_CUR);
	}

	delete pResource;
	return pImg;
error:
	delete pResource;
	delete pImg;
	return NULL;
}
