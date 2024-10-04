// C_JpegImg

//class to load .jpg images

#include "common.h"
#include "fileresource.h" //file handling
#include "jpeg.h"
#include "jpeg_include.h"

#define DCTSIZE 8
#define DEQUANTIZE(coef,quantval)  (((double)(coef)) * (quantval))
#define RIGHT_SHIFT(x,shft)        ((x) >> (shft))
#define DESCALE(x,n)               RIGHT_SHIFT((x) + (1 << ((n)-1)), n)

static const int ZAG[64+16] = {
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63,
	0,   0,  0,  0,  0,  0,  0,  0, //extra entries in case >63 below
	0,   0,  0,  0,  0,  0,  0,  0
};

static const double SFACTOR[DCTSIZE] = {
	1.0, 1.387039845, 1.306562965, 1.175875602,
	1.0, 0.785694958, 0.541196100, 0.275899379
};

C_JpegImg::C_JpegImg()
{
	memset(ac, 0, sizeof(ac));
	memset(dc, 0, sizeof(dc));
	m_pComponent = NULL;

	if(!s_iObjCount) s_pYUV = new C_ColorYUV();
	s_iObjCount++;
}

C_Image *C_JpegImg::Load(const char *szFilename, const char *szResname)
{
	uint8_t *pFilebuf;

	C_Resource *pResource = new C_Resource();
	if(!pResource->SetFilename(szFilename, szResname)) {
		delete pResource;
		return NULL;
	}
	uint32_t iSize = pResource->GetSize();
	pFilebuf = new uint8_t[iSize];
	pResource->Read(pFilebuf, iSize); //read jpeg data to temp-memory
	delete pResource;

	C_Image *pImg = Load(pFilebuf);
	delete[] pFilebuf;
	return pImg;
}

C_Image *C_JpegImg::Load(void *pJpegData)
{
	C_Image    *pImg = NULL;
	bool       bGrey;

	//reset data
	m_iRestartInterval = 0;

	//set beginning of jpeg data
	fb.vptr = pJpegData;

	//decode jpeg data
	if(!ProcessData()) goto error;

	bGrey = (m_iNumComponents == 1);
	if(bGrey) pImg = new C_Image(m_iWidth, m_iHeight, _PAL8);
	else pImg = new C_Image(m_iWidth, m_iHeight, _RGB888);

	//framebuffer (GREY or YUV) -> image (PAL8 or RGB)
	if(bGrey) { //greyscale
		uint8_t *pImgBuf;
		int iLinebytes, i;
		pImg->GetBufferMemory(&pImgBuf, NULL);
		pImg->GetLineSizeInfo(NULL, &iLinebytes, NULL);
		for(i=0; i<m_iHeight; i++, pImgBuf+=iLinebytes) {
			memcpy(pImgBuf, &m_pComponent[0].m_piBuffer[m_iWidthMCUs*m_iDimMCUx*i], m_iWidth);
		}
		//create greyscale pal
		uint8_t aPal[1024];
		for(i=0; i<256; i++) {
			aPal[i*4+2] = aPal[i*4+1] = aPal[i*4] = (uint8_t)i;
		}
		pImg->SetPal(aPal);
	} else s_pYUV->Convert(pImg, m_pComponent, m_iDimMCUx, m_iDimMCUy); //color

error:
	FreeMem();
	return pImg;       //success or NULL
}

C_JpegImg::~C_JpegImg()
{
	if(!(--s_iObjCount)) {
		delete s_pYUV;
		s_pYUV = NULL;
	}
}

//statics
int         C_JpegImg::s_iObjCount = 0;
C_ColorYUV *C_JpegImg::s_pYUV    = NULL;

//frees all temporary memory used to load one jpegimage
void C_JpegImg::FreeMem()
{
	//free YUV framebuffer
	delete[] m_pComponent;
	m_pComponent = NULL;

	//free huffman tables
	int i;
	for(i=0; i<4; i++) {
		delete[] dc[i].piLookNBits;
		delete[] dc[i].piLookSym;
		delete[] ac[i].piLookNBits;
		delete[] ac[i].piLookSym;
	}
	memset(ac, 0, sizeof(ac));
	memset(dc, 0, sizeof(dc));
}

//gets info for jpeg decoder, and starts decoder
bool C_JpegImg::ProcessData()
{
	//check for JFIF/JPEG file
	if(*fb.u16ptr++ != 0xd8ff) return false; //0xff,0xd8 JPEG/JFIF SOI

	do {
		if(*fb.u8ptr++ != 0xff) return false;

		switch(*fb.u8ptr++) {
			case 0xdb:   //define quantization table
				DefineQT();
				break;
			case 0xc0:   //start of frame 0
				if(!HandleFrame0()) return false;
				break;
			case 0xc4:   //define huffman table
				DefineHT();
				//check error
				break;
			case 0xda:   //start of scan
				HandleSOS();
				DecodeMCUs(); //decode the image
				//check error
				break;
			//uninteresting markers
			case 0xe0:   //APP0
			case 0xe1:   //APP1
			case 0xe2:   //APP2
			case 0xe3:   //APP3
			case 0xe4:   //APP4
			case 0xe5:   //APP5
			case 0xe6:   //APP6
			case 0xe7:   //APP7
			case 0xe8:   //APP8
			case 0xe9:   //APP9
			case 0xea:   //APP10
			case 0xeb:   //APP11
			case 0xec:   //APP12
			case 0xed:   //APP13
			case 0xee:   //APP14
			case 0xef:   //APP15
			case 0xfe:   //COM
				fb.u8ptr += Swap(*fb.u16ptr);
				break;

			//parameterless markers
			/*case 0xd0: //RST0
			case 0xd1: //RST1
			case 0xd2: //RST2
			case 0xd3: //RST3
			case 0xd4: //RST4
			case 0xd5: //RST5
			case 0xd6: //RST6
			case 0xd7: //RST7
				break;*/

			case 0xdd: //DRI
				if(Swap(*fb.u16ptr++) != 4) return false; //incorrect length
				m_iRestartInterval = Swap(*fb.u16ptr++);
				break;
			default:
				//printf("%d", *fb.u8ptr);
				break;
		}
	} while(*fb.u16ptr!=0xd9ff); //EOI ?

	return true;
}

//swaps two bytes within a word
uint16_t C_JpegImg::Swap(uint16_t iParam)
{
	return ((iParam>>8)&0x00ff) | ((iParam<<8)&0xff00);
}

//define quantization table
void C_JpegImg::DefineQT()
{
	uint8_t *piEndpos = fb.u8ptr;
	uint8_t iTemp, iQTNum;
	int16_t iQTRead[64];

	piEndpos += Swap(*fb.u16ptr++)-2;
	while(fb.u8ptr < piEndpos) {
		int i, iRow, iCol;
		iTemp = *fb.u8ptr++;
		iQTNum = (uint8_t)(iTemp & 0x0f);

		for(i=0; i<64; i++) {
			if(iTemp >= 16) iQTRead[ZAG[i]] = Swap(*fb.u16ptr++);
			else iQTRead[ZAG[i]] = *fb.u8ptr++;
		}

		i = 0;
		for(iRow=0; iRow<DCTSIZE; iRow++) {
			for(iCol=0; iCol<DCTSIZE; iCol++, i++) {
				qt[iQTNum][i] = ((double) iQTRead[i] * SFACTOR[iRow] * SFACTOR[iCol]);
			}
		}
	}
}

//define huffman table
void C_JpegImg::DefineHT()
{
	int          i;
	uint8_t      *piEndpos = fb.u8ptr;
	S_HuffTable  *pstHTbl;

	piEndpos += Swap(*fb.u16ptr++)-2;
	while(fb.u8ptr < piEndpos) {
		uint8_t iTemp = (uint8_t)(*fb.u8ptr & 0x0f);
		if((*fb.u8ptr >> 4)==0) pstHTbl = &dc[iTemp]; //dc
		else pstHTbl = &ac[iTemp];                    //ac
		fb.u8ptr++;

		iTemp = 0;
		memcpy(&pstHTbl->piBits[1], fb.u8ptr, 16); fb.u8ptr += 16;
		for(i=1; i<=16; i++) iTemp += pstHTbl->piBits[i];
		memcpy(&pstHTbl->pcHT[0], fb.u8ptr, iTemp); fb.u8ptr += iTemp;
		MakeHuffCodes(pstHTbl);
	}
}

//start of frame 0
bool C_JpegImg::HandleFrame0()
{
	C_Component *pCurComp;
	int         i;

	fb.u16ptr++; //skip size of marker, it will end anyway :)
	/*m_iDataPrecision = * */fb.u8ptr++;
	m_iHeight = Swap(*fb.u16ptr++);
	m_iWidth  = Swap(*fb.u16ptr++);

	m_iNumComponents = *fb.u8ptr++;
	if(m_iNumComponents >= 4) return false; //CMYK color not supported

	m_pComponent = new C_Component[m_iNumComponents];
	m_iMCUDcts = 0; m_iDimMCUx = 8; m_iDimMCUy = 8; //assume things

	int compid[3], dimx[3], dimy[3];
	for(i=0; i<m_iNumComponents; i++) {
		int iTemp = *fb.u8ptr++;
		pCurComp  = &m_pComponent[--iTemp];
		compid[i] = iTemp;

		int iDctX = dimx[i] = (*fb.u8ptr >> 4);
		int iDctY = dimy[i] = (*fb.u8ptr & 0x0f);

		int iDcts = iDctX*iDctY;
		fb.u8ptr++;

		if(m_iDimMCUx<8*iDctX) m_iDimMCUx = 8*iDctX;
		if(m_iDimMCUy<8*iDctY) m_iDimMCUy = 8*iDctY;

		pCurComp->m_iQTNum = *fb.u8ptr++;
		while(iDcts--) m_iMCUMember[m_iMCUDcts++] = iTemp;
	}

	for(i=0; i<m_iNumComponents; i++) {
		pCurComp  = &m_pComponent[compid[i]];
		pCurComp->SetNumDcts(dimx[i]*dimy[i], dimx[i]*8, dimy[i]*8);
	}

	m_iWidthMCUs  = (m_iWidth +(m_iDimMCUx-1))/m_iDimMCUx;
	m_iHeightMCUs = (m_iHeight+(m_iDimMCUy-1))/m_iDimMCUy;

	for(i=0; i<m_iNumComponents; i++) {
		m_pComponent[i].AllocBuffer(m_iWidthMCUs, m_iHeightMCUs);
	}

	return true;
}

//start of scan
void C_JpegImg::HandleSOS()
{
	fb.u16ptr++; //skip size of marker
	uint8_t iNum = *fb.u8ptr++;
	for(int i=0; i<iNum; i++) {
		int iTemp = (*fb.u8ptr++)-1;
		m_pComponent[iTemp].m_iACTab = (uint8_t)(*fb.u8ptr & 0x0f);
		m_pComponent[iTemp].m_iDCTab = (uint8_t)(*fb.u8ptr >> 4);
		fb.u8ptr++;
	}
	fb.u8ptr += 3;
}

//precalculate HUFFMAN-codes for faster lookup
void C_JpegImg::MakeHuffCodes(S_HuffTable *pstHs)
{
	uint8_t         aiHuffSize[257];
	uint16_t        aiHuffCode[257], iCode;
	int             i, si, l, p;
	int             iLookBits;

	//make table of huffman code length for
	//each symbol in code-length order
	for(l=1, p=0; l<=16; l++) {
		i = (int)pstHs->piBits[l];
		while(i--) aiHuffSize[p++] = (uint8_t)l;
	}
	aiHuffSize[p] = 0;

	//generate the codes themselves in code-length order
	p = iCode = 0;
	si = aiHuffSize[0];
	while(aiHuffSize[p]) {
		while(((int)aiHuffSize[p]) == si) {
			aiHuffCode[p++] = iCode;
			iCode++;
		}
		iCode <<= 1;
		si++;
	}

	//make lookup table and zero it
	pstHs->piLookNBits = new uint8_t[1<<HUFF_LOOKAHEAD];
	pstHs->piLookSym = new uint8_t[1<<HUFF_LOOKAHEAD];
	memset(pstHs->piLookNBits, 0, (1<<HUFF_LOOKAHEAD));
	memset(pstHs->piLookSym, 0, (1<<HUFF_LOOKAHEAD));

	//fill the table
	for(l=1, p=0; l<=HUFF_LOOKAHEAD; l++) {
		for(i=1; i<=(int)pstHs->piBits[l]; i++, p++) {
			//l = current code's length, p = its index in huffcode[] & hs->ht[]
			//generate left-justified code followed by all possible bit sequences
			iLookBits = aiHuffCode[p]<<(HUFF_LOOKAHEAD-l);
			for(int ctr = 1<<(HUFF_LOOKAHEAD-l); ctr>0; ctr--) {
				pstHs->piLookNBits[iLookBits] = (uint8_t)l;
				pstHs->piLookSym[iLookBits] = pstHs->pcHT[p];
				iLookBits++;
			}
		}
	}
}

#define huff_EXTEND(x,s)  ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))
const int extend_test[16] = {  //entry n is 2**(n-1)
	0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
	0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000
};

const int extend_offset[16] = { //entry n is (-1 << n) + 1
	0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
	((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
	((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
	((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1
};

//decodes all mcus in the entire image
void C_JpegImg::DecodeMCUs()
{
	S_HuffTable     *pstDcTbl, *pstAcTbl;
	C_Component     *pCurComp;

	int16_t         aiMCU[64];

	uint16_t        iCode16;
	int16_t         s, r;
	uint8_t         iTemp;
	int             x, y;

	//handles all mcus in the entire image
	C_JpegBitStream *pStream = new C_JpegBitStream(fb.u8ptr);
	for(y=0; y<m_iHeightMCUs; y++) {
		for(x=0; x<m_iWidthMCUs; x++) {

			//outer loop handles each block in the MCU
			for(int iBlkN=0; iBlkN<m_iMCUDcts; iBlkN++) {
				memset(aiMCU, 0, sizeof(aiMCU));
				//decode a single block of coefficients
				pCurComp = &m_pComponent[m_iMCUMember[iBlkN]];
				pstDcTbl = &dc[pCurComp->m_iDCTab];
				pstAcTbl = &ac[pCurComp->m_iACTab];

				//decode the DC coefficient difference
				iCode16  = (uint16_t)pStream->GetBits(HUFF_LOOKAHEAD);
				iTemp   = pstDcTbl->piLookSym[iCode16];
				pStream->IncPos(pstDcTbl->piLookNBits[iCode16]);
				if(iTemp) {
					//get difference
					r = (int16_t)pStream->GetBits(iTemp);
					pStream->IncPos(iTemp);
					s = (int16_t)huff_EXTEND(r, iTemp);

					//convert DC difference to actual value, update last_dc_val
					s += (int16_t)pCurComp->m_iLastDCVal;
					pCurComp->m_iLastDCVal = s;

					//output the DC coefficient (assumes ZAG[0] = 0)
					aiMCU[0] = s;
				} else aiMCU[0] = (int16_t)pCurComp->m_iLastDCVal;


				//decode the AC coefficients
				//since zeros are skipped, output area must be cleared beforehand
				for(int k=1; k<64; k++) {
					iCode16 = (uint16_t)pStream->GetBits(HUFF_LOOKAHEAD);
					iTemp = pstAcTbl->piLookSym[iCode16];
					pStream->IncPos(pstAcTbl->piLookNBits[iCode16]);

					r = (uint8_t)(iTemp >> 4);
					iTemp &= 15;
					if(iTemp) {
						k += r;
						r = (int16_t)pStream->GetBits(iTemp);
						pStream->IncPos(iTemp);

						s = (int16_t)huff_EXTEND(r, iTemp);
						//output coefficient in natural (de-zigzagged) order
						aiMCU[ZAG[k]] = s;
					} else {
						if(r != 15) break;
						k += 15;
					}
				}

				InvDCT(aiMCU, qt[pCurComp->m_iQTNum]);
				pCurComp->PutDCT(m_aiBlock);
			}
		}
	}

	pStream->IncPos(0); //align data on byte boundary (ie add pos if bitpos>0)
	fb.u8ptr = (uint8_t*)pStream->GetStreamPosPtr();
	delete pStream;
}

//inverse DCT
void C_JpegImg::InvDCT(short *piMcu, double *pdQt)
{
	double          tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	double          tmp10, tmp11, tmp12, tmp13;
	double          z5, z10, z11, z12, z13;
	short           *piIn, *piOut;
	double          *pdQuant, *pdWs;
	double          adWorkspace[DCTSIZE*DCTSIZE]; //buffers data between passes
	int             iCount;

	//pass 1: process columns from input, store into work array.
	piIn    = piMcu;
	pdQuant = pdQt;
	pdWs    = adWorkspace;
	for(iCount=DCTSIZE; iCount>0; iCount--) {
		if ((piIn[DCTSIZE*1] | piIn[DCTSIZE*2] | piIn[DCTSIZE*3] |
			piIn[DCTSIZE*4] | piIn[DCTSIZE*5] | piIn[DCTSIZE*6] |
			piIn[DCTSIZE*7]) == 0)
		{
			//AC terms all zero
			double dDCVal = DEQUANTIZE(piIn[DCTSIZE*0], pdQuant[DCTSIZE*0]);
			pdWs[DCTSIZE*0] = dDCVal;
			pdWs[DCTSIZE*1] = dDCVal;
			pdWs[DCTSIZE*2] = dDCVal;
			pdWs[DCTSIZE*3] = dDCVal;
			pdWs[DCTSIZE*4] = dDCVal;
			pdWs[DCTSIZE*5] = dDCVal;
			pdWs[DCTSIZE*6] = dDCVal;
			pdWs[DCTSIZE*7] = dDCVal;

			piIn++; //advance pointers to next column
			pdQuant++;
			pdWs++;
			continue;
		}

		//even part
		tmp0  = DEQUANTIZE(piIn[DCTSIZE*0], pdQuant[DCTSIZE*0]);
		tmp1  = DEQUANTIZE(piIn[DCTSIZE*2], pdQuant[DCTSIZE*2]);
		tmp2  = DEQUANTIZE(piIn[DCTSIZE*4], pdQuant[DCTSIZE*4]);
		tmp3  = DEQUANTIZE(piIn[DCTSIZE*6], pdQuant[DCTSIZE*6]);
		tmp10 = tmp0 + tmp2;   //phase 3
		tmp11 = tmp0 - tmp2;
		tmp13 = tmp1 + tmp3;   //phases 5-3
		tmp12 = (tmp1 - tmp3) * ((double) 1.414213562) - tmp13;
		tmp0  = tmp10 + tmp13; //phase 2
		tmp3  = tmp10 - tmp13;
		tmp1  = tmp11 + tmp12;
		tmp2  = tmp11 - tmp12;

		//odd part
		tmp4  = DEQUANTIZE(piIn[DCTSIZE*1], pdQuant[DCTSIZE*1]);
		tmp5  = DEQUANTIZE(piIn[DCTSIZE*3], pdQuant[DCTSIZE*3]);
		tmp6  = DEQUANTIZE(piIn[DCTSIZE*5], pdQuant[DCTSIZE*5]);
		tmp7  = DEQUANTIZE(piIn[DCTSIZE*7], pdQuant[DCTSIZE*7]);
		z13   = tmp6 + tmp5;   //phase 6
		z10   = tmp6 - tmp5;
		z11   = tmp4 + tmp7;
		z12   = tmp4 - tmp7;
		tmp7  = z11 + z13;     //phase 5
		tmp11 = (z11 - z13) * ((double) 1.414213562);
		z5    = (z10 + z12) * ((double) 1.847759065);
		tmp10 = ((double) 1.082392200) * z12 - z5;
		tmp12 = ((double) -2.613125930) * z10 + z5;
		tmp6  = tmp12 - tmp7;  //phase 2
		tmp5  = tmp11 - tmp6;
		tmp4  = tmp10 + tmp5;

		pdWs[DCTSIZE*0] = tmp0 + tmp7;
		pdWs[DCTSIZE*7] = tmp0 - tmp7;
		pdWs[DCTSIZE*1] = tmp1 + tmp6;
		pdWs[DCTSIZE*6] = tmp1 - tmp6;
		pdWs[DCTSIZE*2] = tmp2 + tmp5;
		pdWs[DCTSIZE*5] = tmp2 - tmp5;
		pdWs[DCTSIZE*4] = tmp3 + tmp4;
		pdWs[DCTSIZE*3] = tmp3 - tmp4;

		piIn++; //advance pointers to next column
		pdQuant++;
		pdWs++;
	}

	//pass 2: process rows from work array, store into output array.
	// note that we must descale the results by a factor of 8 == 2**3.
	pdWs = adWorkspace;
	for(iCount=0; iCount<DCTSIZE; iCount++) {
		piOut = &m_aiBlock[iCount*8];

		//even part
		tmp10 = pdWs[0] + pdWs[4];
		tmp11 = pdWs[0] - pdWs[4];
		tmp13 = pdWs[2] + pdWs[6];
		tmp12 = (pdWs[2] - pdWs[6]) * ((double) 1.414213562) - tmp13;
		tmp0  = tmp10 + tmp13;
		tmp3  = tmp10 - tmp13;
		tmp1  = tmp11 + tmp12;
		tmp2  = tmp11 - tmp12;

		//odd part
		z13   = pdWs[5] + pdWs[3];
		z10   = pdWs[5] - pdWs[3];
		z11   = pdWs[1] + pdWs[7];
		z12   = pdWs[1] - pdWs[7];
		tmp7  = z11 + z13;
		tmp11 = (z11 - z13) * ((double) 1.414213562);
		z5    = (z10 + z12) * ((double) 1.847759065);
		tmp10 = ((double) 1.082392200) * z12 - z5;
		tmp12 = ((double) -2.613125930) * z10 + z5;
		tmp6  = tmp12 - tmp7;
		tmp5  = tmp11 - tmp6;
		tmp4  = tmp10 + tmp5;

		//final output stage: scale down by a factor of 8 and range-limit
		piOut[0] = (int16_t) DESCALE((int) (tmp0 + tmp7), 3);
		piOut[7] = (int16_t) DESCALE((int) (tmp0 - tmp7), 3);
		piOut[1] = (int16_t) DESCALE((int) (tmp1 + tmp6), 3);
		piOut[6] = (int16_t) DESCALE((int) (tmp1 - tmp6), 3);
		piOut[2] = (int16_t) DESCALE((int) (tmp2 + tmp5), 3);
		piOut[5] = (int16_t) DESCALE((int) (tmp2 - tmp5), 3);
		piOut[4] = (int16_t) DESCALE((int) (tmp3 + tmp4), 3);
		piOut[3] = (int16_t) DESCALE((int) (tmp3 - tmp4), 3);

		pdWs += DCTSIZE; //advance pointer to next row
	}
}



//YUV color converter
#define FIX(x)   ((int) ((x) * (1L<<16) + 0.5))
//statics that keep track of if the range limit table shall
// be created (in constructor) and deleted (in destructor)
uint8_t *C_ColorYUV::s_piRGBRangeLimit = NULL;
int     *C_ColorYUV::s_CrRTab = NULL;
int     *C_ColorYUV::s_CbBTab = NULL;
int     *C_ColorYUV::s_CrGTab = NULL;
int     *C_ColorYUV::s_CbGTab = NULL;
int     C_ColorYUV::s_iObjCount = 0;

//initialize the objects static variables (if not done already)
C_ColorYUV::C_ColorYUV()
{
	//fill YUV -> RGB conversion table
	int i, x;
	if(!s_iObjCount) {
		s_CrRTab = new int[256];
		s_CbBTab = new int[256];
		s_CrGTab = new int[256];
		s_CbGTab = new int[256];
		for(i=0, x=-128; i<=255; i++, x++) {
			s_CrRTab[i]=(int)RIGHT_SHIFT(FIX(1.40200) * x + 32768, 16);
			s_CbBTab[i]=(int)RIGHT_SHIFT(FIX(1.77200) * x + 32768, 16);
			s_CrGTab[i]=(- FIX(0.71414)) * x;
			s_CbGTab[i]=(- FIX(0.34414)) * x + 32768;
		}

		//fill range limit for YUV -> RGB
		s_piRGBRangeLimit = new uint8_t[1024];
		for(i=0; i<256; i++) s_piRGBRangeLimit[i]=(uint8_t)i;
		memset(&s_piRGBRangeLimit[256], 255, 384);
		memset(&s_piRGBRangeLimit[640], 0, 384);
	}
	s_iObjCount++;
}

//delete statics if no objects present
C_ColorYUV::~C_ColorYUV()
{
	if (!(--s_iObjCount)) {
		delete[] s_piRGBRangeLimit;
		delete[] s_CrRTab;
		delete[] s_CbBTab;
		delete[] s_CrGTab;
		delete[] s_CbGTab;
	}
}

//convert YUV to RGB and scale
void C_ColorYUV::Convert(C_Image *pImage, C_Component *pComponent, int dimx, int dimy)
{
	uint8_t *tY,  *frbY  = pComponent[0].m_piBuffer;
	uint8_t *tCb, *frbCb = pComponent[1].m_piBuffer;
	uint8_t *tCr, *frbCr = pComponent[2].m_piBuffer;

	uint8_t *piRGBDst;
	int iWidth, iHeight, iAlignBytes;
	pImage->GetBufferMemory(&piRGBDst, NULL);
	pImage->GetInfo(&iWidth, &iHeight, NULL);
	pImage->GetLineSizeInfo(NULL, NULL, &iAlignBytes);
	int iSrcBytes = (iWidth+15) & ~15;

	int divy = pComponent[0].m_iDimMCUx==dimx;
	int divcb = pComponent[1].m_iDimMCUx==dimx;
	int divcr = pComponent[2].m_iDimMCUx==dimx;

	//case 1,2,2: the only one supported
	for(int i=0; i<iHeight; i++) {
		tY = frbY; tCb = frbCb; tCr = frbCr; //rowpos save
		for(int j=0; j<iWidth; j++) {
			*piRGBDst++ = s_piRGBRangeLimit[ int(*frbY + s_CrRTab[*frbCr]) & 0x000003ff];
			*piRGBDst++ = s_piRGBRangeLimit[ int(*frbY + RIGHT_SHIFT(s_CbGTab[*frbCb] + s_CrGTab[*frbCr], 16)) & 0x000003ff];
			*piRGBDst++ = s_piRGBRangeLimit[ int(*frbY + s_CbBTab[*frbCb]) & 0x000003ff];

			if(divy) frbY++;
			else frbY += (j & 1); //add x if odd
			if(divcb) frbCb++;
			else frbCb += (j & 1); //add x if odd
			if(divcr) frbCr++;
			else frbCr += (j & 1); //add x if odd
		}
		piRGBDst += iAlignBytes;
		frbY  = tY;
		frbCb = tCb; frbCr = tCr;
		if(pComponent[0].m_iDimMCUy==dimy) frbY += iSrcBytes/(divy?1:2);
		else frbY += (i & 1)*(iSrcBytes/(divy?1:2));
		if(pComponent[1].m_iDimMCUy==dimy) frbCb += iSrcBytes/(divcb?1:2);
		else frbCb += (i & 1)*(iSrcBytes/(divcb?1:2));
		if(pComponent[2].m_iDimMCUy==dimy) frbCr += iSrcBytes/(divcr?1:2);
		else frbCr += (i & 1)*(iSrcBytes/(divcr?1:2));
	}
}



//statics that keep track of if the range limit table shall
// be created (in constructor) and deleted (in destructor)
uint8_t *C_Component::s_piYUVRangeLimit = NULL;
int     C_Component::s_iObjCount        = 0;

//initializes a component, and calculate range limit
C_Component::C_Component()
{
	int i, c;
	m_iLastDCVal = m_iQTNum = m_iACTab = m_iDCTab = 0;
	m_iCurMCUX = m_iCurDCT = 0;

	//make range limit table for yuvcolor clipping if not done
	if(!s_piYUVRangeLimit) { 
		s_piYUVRangeLimit = new uint8_t[1024];
		for(i=0, c=128; i<128; i++, c++) s_piYUVRangeLimit[i] = (uint8_t)c;
		memset(&s_piYUVRangeLimit[128], 255, 384);
		memset(&s_piYUVRangeLimit[512], 0, 384);
		for(i=896, c=0; i<1024; i++, c++) s_piYUVRangeLimit[i] = (uint8_t)c;
	}
	s_iObjCount++;
}

//destroys a component
C_Component::~C_Component()
{
	s_iObjCount--;
	if(!s_iObjCount) {
		delete[] s_piYUVRangeLimit;
		s_piYUVRangeLimit = NULL;
	}
	delete[] m_piBuffer;
}

//allocates memory for the components framebuffer
void C_Component::AllocBuffer(int iWidthMCUs, int iHeightMCUs)
{
	m_iNumMCUsX = iWidthMCUs;
	m_piBufPos = m_piBuffer = new uint8_t[iWidthMCUs * iHeightMCUs * 64 * m_iNumDCTs];
}

//put the decoded data into framebuffers (still in YCbCr format)
void C_Component::PutDCT(int16_t *piSrc)
{
	int iFrbWidth = m_iNumMCUsX * m_iDimMCUx;

	if(m_iCurDCT == m_iNumDCTs) {
		if(m_iCurDCT == 4) m_piBufPos -= iFrbWidth * 8; //rewind bufpos
		if(++m_iCurMCUX >= m_iNumMCUsX) {
			m_piBufPos += iFrbWidth * 7;
			if(m_iCurDCT == 4) m_piBufPos += iFrbWidth * 8;
			m_iCurMCUX = 0;
		}
		m_iCurDCT = 0;
	}
	uint8_t *piFrb = m_piBufPos;

	iFrbWidth -= 8;
	for(int i=0; i<8; i++) { //copy the dct into component framebuffer
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		*piFrb++ = s_piYUVRangeLimit[*piSrc++ & 0x03ff];
		piFrb += iFrbWidth;
	}
	if(m_iCurDCT == 1 && m_iNumDCTs==4) m_piBufPos += (iFrbWidth+8)*7 + iFrbWidth;
	else m_piBufPos += 8;

	m_iCurDCT++;
}



//bitstream functions
//initialize, reorder stream
C_JpegBitStream::C_JpegBitStream(uint8_t *piBitStream)
{
	m_piBitPtr = piBitStream;
	m_iCurBit = 0;

	//reorder stream (make 00ffh to ffh)
	//would have to do this between every single byte anyway - easier to do it all at once
	AllPtrType sptr;
	AllPtrType dptr;
	dptr.u8ptr = sptr.u8ptr = m_piBitPtr;
	while(*sptr.u16ptr != 0xd9ff) {
		if(*sptr.u16ptr == 0x00ff) *dptr.u8ptr++ = (uint8_t)*sptr.u16ptr++;
		//else if(*sptr.u16ptr == 0xd0ff) sptr.u16ptr++; //RST0
		//else if(*sptr.u16ptr == 0xd1ff) sptr.u16ptr++; //RST1
		//else if(*sptr.u16ptr == 0xd2ff) sptr.u16ptr++; //RST2
		//else if(*sptr.u16ptr == 0xd3ff) sptr.u16ptr++; //RST3
		//else if(*sptr.u16ptr == 0xd4ff) sptr.u16ptr++; //RST4
		//else if(*sptr.u16ptr == 0xd5ff) sptr.u16ptr++; //RST5
		//else if(*sptr.u16ptr == 0xd6ff) sptr.u16ptr++; //RST6
		//else if(*sptr.u16ptr == 0xd7ff) sptr.u16ptr++; //RST7
		else *dptr.u8ptr++ = *sptr.u8ptr++;
	}
	*dptr.u16ptr = *sptr.u16ptr;
}

//get numbits from the current position in the stream
uint32_t C_JpegBitStream::GetBits(int iNumBits) //works with maximum 24 bits
{
	uint32_t iResult;

	//get 32 bits in byte backwards order
	AllPtrType dptr;
	dptr.u32ptr = &iResult;
	dptr.u8ptr[3] = m_piBitPtr[0];
	dptr.u8ptr[2] = m_piBitPtr[1];
	dptr.u8ptr[1] = m_piBitPtr[2];
	dptr.u8ptr[0] = m_piBitPtr[3];

	iResult <<= m_iCurBit;     //ignore bits already processed
	iResult >>= (32-iNumBits); //make right justified

	return iResult;
}

//add numbits to the current position in the stream
void C_JpegBitStream::IncPos(int iNumBits) //works with many bits
{
	int iBits  = m_iCurBit + iNumBits;
	int iBytes = iBits >> 3;

	m_piBitPtr += iBytes;
	m_iCurBit  = iBits & 7;

	//if(numbits == 0) skip bits until even byte
	if(iNumBits==0 && m_iCurBit!=0) {
		m_piBitPtr++;
		m_iCurBit = 1;
	}
}
