
//program to compile all files in the current directory (recursive) in to
// a special .res file to be used with the C_Resource class.
// this executable itself is excluded.

#include "common.h"
#include "fileresource.h"

#include <vector>

struct S_Item
{
	uint8_t  iNameLength;
	char     szName[256];
	uint32_t iFileStart; //not used
	uint32_t iFileLength;
};
typedef std::vector<S_Item> C_ItemList;


bool CreateResource(const char *szExclude, const char *szDest)
{
	bool bOK, bResult = false;
	S_Item stItem;
	C_ItemList *pclItems = new C_ItemList();
	C_Resource *pclFile = new C_Resource();
	uint32_t iFileStart, iHeaderLen = 4;
	int i, iSize;
	char szFilename[MAX_PATH];

	//collect data
	void *hSearch = C_Resource::FileSearchOpen((char*)"./", (char*)"*");
	while(C_Resource::FileSearchNext(hSearch, szFilename, sizeof(szFilename))) {
		int iLen = (int)strlen(szFilename);
		if(iLen>=(int)sizeof(stItem.szName)) {
			printf("warning: skipping file with too long name (%s)\n", szFilename);
			continue;
		}
		if(strcmp(szFilename, ".")==0) continue;
		if(strcmp(szFilename, "..")==0) continue;
		if(strcmp(szFilename, szExclude)==0) {
			//printf("warning: skipping excluded file (%s)\n", szFilename);
			continue;
		}
		if(strcmp(szFilename, szDest)==0) continue;
		stItem.iNameLength = (uint8_t)iLen;
		strncpy(stItem.szName, szFilename, sizeof(stItem.szName));
		stItem.iFileStart = 0; //not known yet
		if(!pclFile->SetFilename(szFilename)) continue; //skip directories (and files that cannot be opened)
		stItem.iFileLength = pclFile->GetSize();
		if(!stItem.iFileLength) {
			printf("warning: skipping empty file (%s)\n", szFilename);
			continue;
		}
		iHeaderLen += 1+stItem.iNameLength+sizeof(int)*2;
		pclItems->push_back(stItem);
	}
	C_Resource::FileSearchClose(hSearch);

	//write header
	pclFile->SetMode(false);
	if(!pclFile->SetFilename(szDest)) {
		printf("error: could not open destination (%s)\n", szDest);
		goto error;
	}
	iFileStart = iHeaderLen;
	iSize = (int)pclItems->size();
	bOK = pclFile->Write(&iSize, sizeof(iSize));
	for(i=0; i<iSize; i++) {
		S_Item *pstItem = &(*pclItems)[i];
		if(bOK) bOK = pclFile->Write(&pstItem->iNameLength, sizeof(pstItem->iNameLength));
		if(bOK) bOK = pclFile->Write(pstItem->szName, pstItem->iNameLength);
		if(bOK) bOK = pclFile->Write(&iFileStart, sizeof(iFileStart));
		if(bOK) bOK = pclFile->Write(&pstItem->iFileLength, sizeof(pstItem->iFileLength));
		iFileStart += pstItem->iFileLength;
	}
	if(!bOK) {
		printf("error: could not write to destination (%s)\n", szDest);
		goto error;
	}

	//write file entries
	for(i=0; i<iSize; i++) {
		S_Item *pstItem = &(*pclItems)[i];
		C_Resource *pclFileSrc = new C_Resource();
		pclFileSrc->SetFilename(pstItem->szName);
		uint8_t *pData = new uint8_t[pstItem->iFileLength];
		bOK = pclFileSrc->Read(pData, pstItem->iFileLength);
		delete pclFileSrc;
		if(!bOK) {
			printf("error: could not read source (%s)\n", pstItem->szName);
			delete[] pData;
			goto error;
		}
		bOK = pclFile->Write(pData, pstItem->iFileLength);
		delete[] pData;
		if(!bOK) {
			printf("error: could not write to destination (%s)\n", szDest);
			goto error;
		}
	}

	printf("\nresource created as: '%s'\n", szDest);
	bResult = true;
error:
	delete pclFile; //closes the file
	delete pclItems;
	return bResult;
}

int main(int argc, char *argv[])
{
	printf("res_comp v1.00\n");
	/*if(argc!=2) {
		printf("usage: 'res_comp output.res'\n compiles all files in the current directory\n"
			" except this exe into the given output file. not recursive.\n");
		return 1;
	}*/
	char *szExeName = strrchr(argv[0], '/');
	if(!szExeName) szExeName = strrchr(argv[0], '\\');
	return CreateResource(szExeName ? szExeName+1 : argv[0], argc==2 ? argv[1] : "resource.dat") ? 0 : 1;
}
