// C_TargaImg

//class to load/save .tga images
// - tga uncompressed
// - tga rle compressed

#include "fileresource.h"
#include "glconverter.h"
#include "gltarga.h"

#pragma pack(1)
struct S_TargaHeader
{
	uint8_t  id_length;
	uint8_t  colmap_type;
	uint8_t  img_typecode;
	uint16_t colmap_origin;
	uint16_t colmap_length;
	uint8_t  colmap_entrysize;
	uint16_t xstart, ystart;
	uint16_t width, height;
	uint8_t  bitsperpixel;
	uint8_t  img_descriptor;
};
#pragma pack()

C_Image *C_TargaImg::Load(void *i_pTargaData)
{
	return LoadPrivate(NULL, NULL, i_pTargaData);
}

C_Image *C_TargaImg::Load(const char *i_szFilename, const char *i_szResname)
{
	return LoadPrivate(i_szFilename, i_szResname, NULL);
}

C_Image *C_TargaImg::LoadPrivate(const char *i_szFilename, const char *i_szResname, void *i_pTargaData)
{
	S_TargaHeader stHeader;                    //the header
	C_Image       *pclImg;                     //the image to fill

	uint8_t       aiImgPal[1024];
	uint8_t       aiTgaPal[768];               //temporary pal
	uint8_t       *piPalSrc, *piPalDst;        //temporary step pointers

	int           iDstInc, i, j;
	uint8_t       *pDstPos;
	int           iSrcPos;

	uint8_t       *pFilebuf = NULL;            //buffer for the tga-file-data
	uint8_t       *pFilebufCpy;                //temporary pointer for step through
	bool          bDelmem = true;              //delete file-data-mem? (false for user mem)
	C_Resource    *pclResource = NULL;

	bool          bCompressed = false;         //guess uncompressed (raw)
	uint32_t      iBytesPerPixel;              //number of bytes per pixel (1,2,3,4)
	int           iPixcount;                   //number of pixels on row so far
	int           iWidth, iHeight;
	int           iPixelfmt, iLineBytes;

	if(i_szFilename || i_szResname) {
		pclResource = new C_Resource();
		pclResource->SetFilename(i_szFilename, i_szResname);
		if(!pclResource->Read(&stHeader, sizeof(S_TargaHeader))) goto error; //read header
	} else {
		if(!i_pTargaData) goto error;
		pFilebufCpy = pFilebuf = (uint8_t*)i_pTargaData;
		memcpy(&stHeader, pFilebuf, sizeof(S_TargaHeader));
		pFilebufCpy += sizeof(S_TargaHeader);
		bDelmem = false;
	}

	//check that the targa is valid
	iBytesPerPixel = stHeader.bitsperpixel/8;
	if(stHeader.colmap_type>1 || iBytesPerPixel<1 || iBytesPerPixel>4 ||
		(stHeader.img_descriptor>>6)) goto error;
	//skip any id-field
	if(stHeader.id_length) pclResource->Seek(stHeader.id_length, SEEK_CUR);

	//create the destination image
	iWidth = stHeader.width;
	iHeight = stHeader.height;
	iPixelfmt = iBytesPerPixel;
	if(iBytesPerPixel == 2) iPixelfmt = _BGR555;
	pclImg = new C_Image(stHeader.width, stHeader.height, iPixelfmt);
	//^can the pixelformat be something else?

	//check if the targa is compressed (with RLE)
	if(stHeader.img_typecode > 8) {
		bCompressed = true;
		stHeader.img_typecode -= 8;
	}

	switch(stHeader.img_typecode) {
		case 1:  //read palette if exists
			piPalDst = aiImgPal;
			piPalSrc = aiTgaPal;
			if(stHeader.colmap_entrysize != 24) goto error;
			pclResource->Read(aiTgaPal, stHeader.colmap_length*3);
			for(i=0; i<256; i++) {
				piPalDst[2] = *piPalSrc++; piPalDst[1] = *piPalSrc++;
				piPalDst[0] = *piPalSrc++; //piPalDst[3] = 0;
				piPalDst += 4;
			}
			pclImg->SetPal(aiImgPal);
			break;
		case 3:  //create own palette if greyscale
			piPalDst = aiImgPal;
			for(i=0; i<256; i++) {
				*piPalDst++ = (uint8_t)i; *piPalDst++ = (uint8_t)i;
				*piPalDst++ = (uint8_t)i; *piPalDst++ = 0;
			}
			pclImg->SetPal(aiImgPal);
			break;
	}

	//if compressed then read file to mem
	if(bCompressed && pclResource) {
		int iSize = pclResource->GetSize() -
			(stHeader.colmap_length*3 + stHeader.id_length + 18);
		pFilebuf = new uint8_t[iSize]; //get tempmem
		if(!pclResource->Read(pFilebuf, iSize)) goto error;
   }

	//check if image is stored upside down
	pFilebufCpy = pFilebuf;       //source if compressed
	pclImg->GetBufferMemory(&pDstPos, NULL);
	pclImg->GetLineSizeInfo(NULL, &iLineBytes, NULL);
	if((stHeader.img_descriptor & 0x20)==0) {
		pDstPos += iLineBytes * (iHeight - 1);
		iDstInc = -iLineBytes;
	} else iDstInc = iLineBytes;

	for(i=0; i<iHeight; i++) {
		if(bCompressed) {
			uint8_t *pDstPosCpy = pDstPos;
			iPixcount = 0;
			iSrcPos = 0;
			do {
				//read new "header"
				uint8_t iNumpix = pFilebufCpy[iSrcPos++];
				if(iNumpix & 0x80) {
					//put many equal pixels
					iNumpix = (iNumpix & 0x7f)+1;
					for(j=0; j<iNumpix; j++) {
						memcpy(pDstPosCpy, pFilebufCpy+iSrcPos, iBytesPerPixel);
						pDstPosCpy += iBytesPerPixel;
					}
					iSrcPos    += iBytesPerPixel;
					iPixcount  += iNumpix;
				} else {
					//put many diffrent pixels
					iNumpix++;
					memcpy(pDstPosCpy, pFilebufCpy+iSrcPos, iNumpix*iBytesPerPixel);
					pDstPosCpy += iNumpix*iBytesPerPixel;
					iSrcPos    += iNumpix*iBytesPerPixel;
					iPixcount  += iNumpix;
				}
			} while(iPixcount<iWidth);
			pFilebufCpy += iSrcPos;
		} else {
			int iLinelen = iWidth * iBytesPerPixel;
			if(pclResource) {
				if(!pclResource->Read(pDstPos, iLinelen)) goto error;
			} else {
				memcpy(pDstPos, pFilebufCpy, iLinelen);
				pFilebufCpy += iLinelen;
			}
		}

		pDstPos += iDstInc;
	}
	if(bDelmem) delete[] pFilebuf;
	delete pclResource;

	return pclImg;         //success (or NULL)
error:
	if(bDelmem) delete[] pFilebuf;
	delete pclResource;
	return NULL;        //faliure
}

bool C_TargaImg::Save(const char *i_szFilename, C_Image *i_pclSrc, bool i_bFlip)
{
	S_TargaHeader stHeader;
	uint8_t       aiPal[768];
	C_Image       *pclImg = NULL; //if conversion, keep source image
	int           iWidth, iHeight, iPixelfmt;
	int           iWidthBytes, iLineBytes, iResult;

	if(!i_pclSrc) return false; //no image?

	//convert imageformat if not same...
	i_pclSrc->GetInfo(&iWidth, &iHeight, &iPixelfmt);
	switch(iPixelfmt & 0x0f) {
		case 2: iPixelfmt = _BGR555;   break;
		case 3: iPixelfmt = _BGR888;   break;
		case 4: iPixelfmt = _BGRA8888; break;
	}
	C_Converter *pConv = new C_Converter(iPixelfmt);
	pConv->Convert(i_pclSrc, &pclImg, &iResult);
	delete pConv;
	if(!iResult) return false; //error converting

	memset(&stHeader, 0, sizeof(stHeader));
	stHeader.width = (uint16_t)iWidth;
	stHeader.height = (uint16_t)iHeight;
	stHeader.bitsperpixel = (uint8_t)((iPixelfmt & 0x0f) * 8);
	if(!i_bFlip) stHeader.img_descriptor |= 0x20; //not fliped top-bottom
	stHeader.img_typecode = stHeader.bitsperpixel>8 ? 2 : 1;

	uint8_t *pMemToReadFrom, *pPalSrc;
	pclImg->GetBufferMemory(&pMemToReadFrom, &pPalSrc);

	if(stHeader.img_typecode == 1) { //palette
		stHeader.colmap_entrysize = 24;
		stHeader.colmap_length    = 256;
		stHeader.colmap_type      = 1;

		uint8_t *pPalDst = &aiPal[0]; //, *palsrc = tmp->pal;
		if(pPalSrc) for(int i=0; i<256; i++) { //make pal
			*pPalDst++ = pPalSrc[2];  //b
			*pPalDst++ = pPalSrc[1];  //g
			*pPalDst++ = pPalSrc[0];  //r
			pPalSrc += 4;
		}
	}

	C_Resource *pclFile = new C_Resource();
	pclFile->SetMode(false);
	bool bOK = pclFile->SetFilename(i_szFilename);
	if(bOK) bOK = pclFile->Write(&stHeader, sizeof(stHeader)); //write header
	if(stHeader.img_typecode == 1) {
		if(bOK) bOK = pclFile->Write(&aiPal, 768);             //if pal: write it
	}

	//write imagedata
	pclImg->GetLineSizeInfo(&iWidthBytes, &iLineBytes, NULL);
	for(int y=0; y<iHeight; y++) {
		if(!bOK) break;
		bOK = pclFile->Write(pMemToReadFrom, iWidthBytes);
		pMemToReadFrom += iLineBytes;
	}
	delete pclFile; //closes file
	delete pclImg;
	return bOK;
}


#define CMPPIXEL(pcSrcBuf_, iSize_, bResult_) \
{ \
	switch(iSize_) { \
		case 1: bResult_ = pcSrcBuf_[0] == pcSrcBuf_[1]; break;  \
		case 2: bResult_ = ((uint16_t*)pcSrcBuf_)[0] == ((uint16_t*)pcSrcBuf_)[1]; break; \
		case 3: bResult_ = (pcSrcBuf_[0] == pcSrcBuf_[3]) && (pcSrcBuf_[1] == pcSrcBuf_[4]) && (pcSrcBuf_[2] == pcSrcBuf_[5]); break;\
		case 4: bResult_ = ((uint32_t*)pcSrcBuf_)[0] == ((uint32_t*)pcSrcBuf_)[1]; break; \
	} \
}

#define CPYPIXEL(pcSrcBuf_, pcDstBuf_, iSize_) \
{ \
	switch(iSize_) { \
		case 1: pcDstBuf_[0] = pcSrcBuf_[0]; break;  \
		case 2: ((uint16_t*)pcDstBuf_)[0] = ((uint16_t*)pcSrcBuf_)[0]; break; \
		case 3: ((uint16_t*)pcDstBuf_)[0] = ((uint16_t*)pcSrcBuf_)[0]; pcDstBuf_[2] = pcSrcBuf_[2]; break; \
		case 4: ((uint32_t*)pcDstBuf_)[0] = ((uint32_t*)pcSrcBuf_)[0]; break; \
	} \
}

bool C_TargaImg::SaveCompressed(const char *i_szFilename, C_Image *i_pclSrc, bool i_bFlip)
{
	S_TargaHeader stHeader;
	uint8_t       aiPal[768];
	C_Image       *pclImg = NULL; //if conversion, keep source image
	int           iWidth, iHeight, iPixelfmt;
	int           iWidthBytes, iLineBytes, iResult;

	if(!i_pclSrc) return false; //no image?

	//convert imageformat if not same...
	i_pclSrc->GetInfo(&iWidth, &iHeight, &iPixelfmt);
	switch(iPixelfmt & 0x0f) {
		case 2: iPixelfmt = _BGR555;   break;
		case 3: iPixelfmt = _BGR888;   break;
		case 4: iPixelfmt = _BGRA8888; break;
	}
	C_Converter *pConv = new C_Converter(iPixelfmt);
	pConv->Convert(i_pclSrc, &pclImg, &iResult);
	delete pConv;
	if(!iResult) return false; //error converting

	memset(&stHeader, 0, sizeof(stHeader));
	stHeader.width = (uint16_t)iWidth;
	stHeader.height = (uint16_t)iHeight;
	stHeader.bitsperpixel = (uint8_t)((iPixelfmt & 0x0f) * 8);
	if(!i_bFlip) stHeader.img_descriptor |= 0x20; //not fliped top-bottom
	stHeader.img_typecode = stHeader.bitsperpixel>8 ? 10 : 9; //compressed

	uint8_t *pSrcBuf, *pPalSrc;
	pclImg->GetBufferMemory(&pSrcBuf, &pPalSrc);

	if(stHeader.img_typecode == 9) { //palette
		stHeader.colmap_entrysize = 24;
		stHeader.colmap_length    = 256;
		stHeader.colmap_type      = 1;

		uint8_t *pPalDst = &aiPal[0]; //, *palsrc = tmp->pal;
		if(pPalSrc) for(int i=0; i<256; i++) { //make pal
			*pPalDst++ = pPalSrc[2];  //b
			*pPalDst++ = pPalSrc[1];  //g
			*pPalDst++ = pPalSrc[0];  //r
			pPalSrc += 4;
		}
	}

	FILE *pFile = fopen(i_szFilename, "wb");
	fwrite(&stHeader, sizeof(stHeader), 1, pFile);                //write header
	if(stHeader.img_typecode == 9) fwrite(&aiPal, 768, 1, pFile); //if pal: write it

	//create imagedata
	pclImg->GetLineSizeInfo(&iWidthBytes, &iLineBytes, NULL);

	uint8_t *pDstBuf, *pDstBufCpy;
	pDstBufCpy = pDstBuf = new uint8_t[iWidthBytes * iHeight]; //get tempmem

	int iPixelSize = (iPixelfmt & 0x0f);
	for(int y=0; y<iHeight; y++) {
		int posx = 0;
		uint8_t *pSrcBufCpy = pSrcBuf;

		while(posx<iWidth) {
			bool bTest = false;
			uint8_t *pHdrbyte = pDstBufCpy; pDstBufCpy++;
			int iMax = (iWidth-posx)-1 < 127 ?(iWidth-posx)-1:127;
			CMPPIXEL(pSrcBufCpy, iPixelSize, bTest)

			CPYPIXEL(pSrcBufCpy, pDstBufCpy, iPixelSize)
			pSrcBufCpy += iPixelSize;
			pDstBufCpy += iPixelSize;
			*pHdrbyte = 0;
			posx++;
			if(bTest) { //is same

				while(*pHdrbyte<iMax) {
					CMPPIXEL(pSrcBufCpy, iPixelSize, bTest)
					pSrcBufCpy += iPixelSize;
					*pHdrbyte += 1;
					posx++;
					if(!bTest) break;
				}
				*pHdrbyte += 128;
			} else {   //is not same
				while(*pHdrbyte<iMax) {
					CMPPIXEL(pSrcBufCpy, iPixelSize, bTest)
					if(bTest && (*pHdrbyte!=iMax-1)) break;

					CPYPIXEL(pSrcBufCpy, pDstBufCpy, iPixelSize)
					pSrcBufCpy += iPixelSize;
					pDstBufCpy += iPixelSize;

					*pHdrbyte += 1;
					posx++;
				}

			}
		}
		pSrcBuf += iLineBytes;
	}

	//write imagedata
	fwrite(pDstBuf, pDstBufCpy-pDstBuf, 1, pFile);

	delete[] pDstBuf;
	fclose(pFile);
	delete pclImg;
	return true;
}
