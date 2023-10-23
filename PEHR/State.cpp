#include "CommonHeader.h"
#include "time.h"

HWND commonmenu, ntmenu, sectionmenu, directorymenu;
HWND commonlist;
HWND directorybox, directorylist;

LVITEM LI;
LVCOLUMN COL;
wchar_t buffer[255] = {0};

int InsertExportDirectoryItem(int idx, int alpha)
{
	unsigned short ordinal;
	memcpy(&ordinal, &PEFile[ConvertRVAToOffset(IED.AddressOfNameOrdinals) 
		+ (idx * sizeof(unsigned short))], sizeof(unsigned short));

	unsigned long nameadr;
	memcpy(&nameadr, &PEFile[ConvertRVAToOffset(IED.AddressOfNames)
		+ (idx * sizeof(unsigned long))], sizeof(unsigned long));

	unsigned long filesize = pSH[NTH.FileHeader.NumberOfSections - 1].PointerToRawData + 
		pSH[NTH.FileHeader.NumberOfSections - 1].SizeOfRawData;
	if(ConvertRVAToOffset(nameadr) > filesize) return 1;

	unsigned long funcadr;
	memcpy(&funcadr, &PEFile[ConvertRVAToOffset(IED.AddressOfFunctions) 
		+ (ordinal * sizeof(unsigned long))], sizeof(unsigned long));

	LI.iItem = idx - alpha;
	LI.iSubItem = 0;	
	memset(buffer, 0, sizeof(wchar_t) * 255);
	swprintf(buffer, L"%x", funcadr);
	LI.pszText = buffer;
	ListView_InsertItem(directorylist, &LI);

	memset(buffer, 0, sizeof(wchar_t) * 255);
	GetStringFromFile(nameadr, buffer);

	LI.iSubItem = 1;
	LI.pszText = buffer;
	ListView_SetItem(directorylist, &LI);

	return 0;
}

int InsertImportDirectoryItem(int idx, int libidx, int alpha)
{
	unsigned long funcadr;
	int typesize;

	if (g_platform == PLATFORM_X86)
		typesize = 4;
	else if (g_platform == PLATFORM_X64)
		typesize = 8;
	
	memcpy(&funcadr, &PEFile[ConvertRVAToOffset(pIID[libidx].OriginalFirstThunk) + typesize * idx], sizeof(unsigned long));

	unsigned long filesize = pSH[NTH.FileHeader.NumberOfSections - 1].PointerToRawData + 
		pSH[NTH.FileHeader.NumberOfSections - 1].SizeOfRawData;
	if(ConvertRVAToOffset(funcadr) > filesize || ConvertRVAToOffset(funcadr) < NTH.OptionalHeader.SizeOfHeaders) return 1;

	unsigned short hint;
	memcpy(&hint, &PEFile[ConvertRVAToOffset(funcadr)], sizeof(unsigned short));

	LI.iItem = idx - alpha;
	LI.iSubItem = 0;	
	memset(buffer, 0, sizeof(wchar_t) * 255);
	swprintf(buffer, L"%x", hint);
	LI.pszText = buffer;
	ListView_InsertItem(directorylist, &LI);

	memset(buffer, 0, sizeof(wchar_t) * 255);
	GetStringFromFile(funcadr + 2, buffer);

	LI.iSubItem = 1;
	LI.pszText = buffer;
	ListView_SetItem(directorylist, &LI);

	return 0;
}

void CreateCommonTab()
{
	commonmenu = CreateWindow(L"SysTabControl32", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
		0, 0, 600, 680, g_hWnd, (HMENU)NULL, g_hInst, NULL); 

	TCITEM citem;
    citem.mask = TCIF_TEXT; 
    citem.pszText = L"DOS Header"; 
    TabCtrl_InsertItem(commonmenu, 0, &citem);
 
    citem.pszText = L"NT Header"; 
    TabCtrl_InsertItem(commonmenu, 1, &citem);

	citem.pszText = L"Section Header"; 
    TabCtrl_InsertItem(commonmenu, 2, &citem);

	TabCtrl_SetCurSel(commonmenu, 0);
    ShowWindow(commonmenu, SW_SHOW);
    UpdateWindow(commonmenu);
}

void CreateCommonList()
{
	commonlist = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | 
		LVS_SHOWSELALWAYS, 10, 60, 570, 570, g_hWnd, (HMENU)NULL, g_hInst, NULL);

	wchar_t* lpszCols[3] = {L"Member", L"Value", L"Explanation"};
    int nColWidth[3] = {210, 90, 268};

	COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    COL.fmt=LVCFMT_LEFT;
    for(int i = 0; i < 3; i++)
    {
        COL.cx = nColWidth[i];
        COL.pszText = lpszCols[i];
        ListView_InsertColumn(commonlist, i, &COL);
    }

	LI.mask = LVIF_TEXT;
	LI.state = 0;
	LI.stateMask = 0;
}

void CreateNTHeaderTab()
{
	ntmenu = CreateWindow(L"SysTabControl32", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
		0, 23, 600, 657, g_hWnd, (HMENU)NULL, g_hInst, NULL); 

	TCITEM ntitem;
    ntitem.mask = TCIF_TEXT; 
    ntitem.pszText = L"File Header"; 
    TabCtrl_InsertItem(ntmenu, 0, &ntitem);
 
    ntitem.pszText = L"Optional Header"; 
    TabCtrl_InsertItem(ntmenu, 1, &ntitem);

    ntitem.pszText = L"Data Directory"; 
    TabCtrl_InsertItem(ntmenu, 2, &ntitem);

	TabCtrl_SetCurSel(ntmenu, 0);
    ShowWindow(ntmenu, SW_SHOW);
    UpdateWindow(ntmenu);
}

void CreateDataDirectoryTab()
{
	directorymenu = CreateWindow(L"SysTabControl32", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
		0, 46, 600, 634, g_hWnd, (HMENU)NULL, g_hInst, NULL); 

	TCITEM directoryitem;
    directoryitem.mask = TCIF_TEXT;  
    directoryitem.pszText = L"Export"; 
    TabCtrl_InsertItem(directorymenu, 0, &directoryitem);
 
    directoryitem.pszText = L"Import"; 
    TabCtrl_InsertItem(directorymenu, 1, &directoryitem);
   
	TabCtrl_SetCurSel(directorymenu, 0);
    ShowWindow(directorymenu, SW_SHOW);
    UpdateWindow(directorymenu);
}

void CreateDataDirectoryList()
{
	directorybox = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
		10, 83, 150, 557, g_hWnd, (HMENU)ID_LISTBOX, g_hInst, NULL);

	directorylist = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | 
		LVS_SHOWSELALWAYS, 170, 83, 410, 543, g_hWnd, (HMENU)NULL, g_hInst, NULL);
}

void CreateImportDirectoryColumn()
{
	wchar_t* lpszCols[2] = {L"Ordinal", L"Function Name"};
    int nColWidth[2] = {90, 318};

    for(int i = 0; i < 2; i++)
    {
        COL.cx = nColWidth[i];
        COL.pszText = lpszCols[i];
        ListView_InsertColumn(directorylist, i, &COL);
    }
}

void CreateExportDirectoryColumn()
{
	wchar_t* lpszCols[2] = {L"Address", L"Function Name"};
    int nColWidth[2] = {90, 318};

    for(int i = 0; i < 2; i++)
    {
        COL.cx = nColWidth[i];
        COL.pszText = lpszCols[i];
        ListView_InsertColumn(directorylist, i, &COL);
    }
}

void CreateSectionHeaderTab()
{
	sectionmenu = CreateWindow(L"SysTabControl32", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
		0, 23, 600, 657, g_hWnd, (HMENU)NULL, g_hInst, NULL); 

	TCITEM sectionitem;
    sectionitem.mask = TCIF_TEXT; 

	for(int i = 0; i < NTH.FileHeader.NumberOfSections; i++)
	{
		memset(buffer, 0, sizeof(wchar_t) * 255);
		
		for(int j = 0; j < 8; j++)
			buffer[j] = pSH[i].Name[j];

		sectionitem.pszText = buffer; 
		TabCtrl_InsertItem(sectionmenu, i, &sectionitem);
	}
   
	TabCtrl_SetCurSel(sectionmenu, 0);
    ShowWindow(sectionmenu, SW_SHOW);
    UpdateWindow(sectionmenu);
}

void State::Escape()
{
	ListView_DeleteAllItems(commonlist);
}

void DosHeaderState::Enter()
{
	InsertItem<unsigned short>(0, L"Dos Signature", &DosH.e_magic);
	LI.iSubItem = 2;
	memset(buffer, 0, sizeof(wchar_t) * 255);
	unsigned long magic = SwapDwordEndian(DosH.e_magic);
	buffer[0] = (magic & 0xff000000) >> 24;
	buffer[1] = (magic & 0x00ff0000) >> 16;
	buffer[2] = (magic & 0x0000ff00) >> 8;
	buffer[3] = magic & 0x000000ff;
	LI.pszText = buffer;
	ListView_SetItem(commonlist, &LI);
	InsertItem<unsigned long>(1, L"offset to NTHeader", &DosH.e_lfanew);
}

void NTHeaderState::Enter()
{
	DestroyWindow(commonmenu);
	CreateNTHeaderTab();
	CreateCommonTab();
	TabCtrl_SetCurSel(commonmenu, 1);

	if(substate != NULL)
		substate -> Escape();
	substate = State::fileheader;
	substate -> Enter();
}

void NTHeaderState::Escape()
{
	DestroyWindow(ntmenu);

	if(substate != NULL)
		substate -> Escape();
	substate = NULL;
}

void FileHeaderState::Enter()
{
	InsertItem<unsigned short>(0, L"Machine Number", &NTH.FileHeader.Machine);
	LI.iSubItem = 2;
	switch(NTH.FileHeader.Machine)
	{
	case 0 : LI.pszText = L"Unknown"; break;
	case 0x014c : LI.pszText = L"Intel 386"; break;
	case 0x0160 : LI.pszText = L"MIPS big-endian"; break;
	case 0x0162 :
	case 0x0166 :
	case 0x0168 : LI.pszText = L"MIPS little-endian"; break;
	case 0x0169 : LI.pszText = L"MIPS little-endian WCE v2"; break;
	case 0x0184 : LI.pszText = L"Alpha_AXP"; break;
	case 0x01a2 : LI.pszText = L"SH3 little-endian"; break;
	case 0x01a4 : LI.pszText = L"SH3E little-endian"; break;
	case 0x01a6 : LI.pszText = L"SH4 little-endian"; break;
	case 0x01a8 : LI.pszText = L"SH5"; break;
	case 0x01c0 : LI.pszText = L"ARM little-endian"; break;
	case 0x01c2 : LI.pszText = L"ARM Thumb/Thumb-2 Little-Endian"; break;
	case 0x01c4 : LI.pszText = L"ARM Thumb-2 Little-Endian"; break;
	case 0x01f0: LI.pszText = L"IBM PowerPC Little-Endian"; break;
	case 0x0200 : LI.pszText = L"Intel 64"; break;
	case 0x0266 : 
	case 0x0366 : 
	case 0x0466 : LI.pszText = L"MIPS"; break;
	case 0x0284 : LI.pszText = L"ALPHA64"; break;
	case 0x0520 : LI.pszText = L"Infineon"; break;
	case 0x0EBC : LI.pszText = L"EFI Byte Code"; break;
	case 0x8664 : LI.pszText = L"AMD64 (K8)"; break;
	case 0x9041 : LI.pszText = L"M32R little-endian"; break;
	case 0xAA64 : LI.pszText = L"ARM64 Little-Endian"; break;
	}
	ListView_SetItem(commonlist, &LI);
	InsertItem<unsigned short>(1, L"Number of sections", &NTH.FileHeader.NumberOfSections);
	InsertItem<unsigned long>(2, L"File creation time", &NTH.FileHeader.TimeDateStamp);
	LI.iSubItem = 2;
	time_t t;
	time(&t);
	memcpy(&t, &NTH.FileHeader.TimeDateStamp, sizeof(unsigned long));
	char * s = ctime(&t);
	memset(buffer, 0, sizeof(wchar_t) * 255);
	
	for(int i = 0; i < 255; i++)
	{
		if(s[i] == 10) 
			break;
		buffer[i] = s[i];
	}

	LI.pszText = buffer;
	ListView_SetItem(commonlist, &LI);
	InsertItem<unsigned long>(3, L"Pointer to symbol table", &NTH.FileHeader.PointerToSymbolTable);
	InsertItem<unsigned long>(4, L"Number of symbols", &NTH.FileHeader.NumberOfSymbols);
	InsertItem<unsigned short>(5, L"Size of Optional header", &NTH.FileHeader.SizeOfOptionalHeader);
	InsertItem<unsigned short>(6, L"Characteristics of the image", &NTH.FileHeader.Characteristics);
	LI.iSubItem = 2;
	string str = "";
	unsigned short character = NTH.FileHeader.Characteristics;
	memset(buffer, 0, sizeof(wchar_t) * 255);
	if((character & 0x8000) == 0x8000) {if(str != "") str += " / "; str += "Bytes reversed(high)";}
	if((character & 0x4000) == 0x4000) {if(str != "") str += " / "; str += "UP system only";}
	if((character & 0x2000) == 0x2000) {if(str != "") str += " / "; str += "DLL file";}
	if((character & 0x1000) == 0x1000) {if(str != "") str += " / "; str += "System file";}
	if((character & 0x0800) == 0x0800) {if(str != "") str += " / "; str += "Net run from swap";}
	if((character & 0x0400) == 0x0400) {if(str != "") str += " / "; str += "Removable run from swap";}
	if((character & 0x0200) == 0x0200) {if(str != "") str += " / "; str += "Debug stripped";}
	if((character & 0x0100) == 0x0100) {if(str != "") str += " / "; str += "32bit machine";}
	if((character & 0x0080) == 0x0080) {if(str != "") str += " / "; str += "Bytes reversed(low)";}
	if((character & 0x0020) == 0x0020) {if(str != "") str += " / "; str += "Large address aware";}
	if((character & 0x0010) == 0x0010) {if(str != "") str += " / "; str += "Aggressive working set trim";}
	if((character & 0x0008) == 0x0008) {if(str != "") str += " / "; str += "Local symbols stripped";}
	if((character & 0x0004) == 0x0004) {if(str != "") str += " / "; str += "Line numbers stripped";}
	if((character & 0x0002) == 0x0002) {if(str != "") str += " / "; str += "Executable file";}
	if((character & 0x0001) == 0x0001) {if(str != "") str += " / "; str += "Relocation info stripped";}

	for(int i = 0; i < str.size(); i++)
		buffer[i] = str[i];

	LI.pszText = buffer;
	ListView_SetItem(commonlist, &LI);
}

void OptionalHeaderState::Enter()
{
	int anum = 7;
	InsertItem<unsigned short>(0, L"The state of image file(magic)", &NTH.OptionalHeader.Magic);
	LI.iSubItem = 2;
	switch(NTH.OptionalHeader.Magic)
	{
	case 0x010b : LI.pszText = L"IMAGE_OPTIONAL_HEADER32"; break;
	case 0x020b : LI.pszText = L"IMAGE_OPTIONAL_HEADER64"; break;
	}
	ListView_SetItem(commonlist, &LI);
	InsertItem<unsigned char>(1, L"Major linker version", &NTH.OptionalHeader.MajorLinkerVersion);
	InsertItem<unsigned char>(2, L"Minor linker version", &NTH.OptionalHeader.MinorLinkerVersion);
	InsertItem<unsigned long>(3, L"Size of code", &NTH.OptionalHeader.SizeOfCode);
	InsertItem<unsigned long>(4, L"Size of initialized data", &NTH.OptionalHeader.SizeOfInitializedData);
	InsertItem<unsigned long>(5, L"Size of uninitialized data", &NTH.OptionalHeader.SizeOfUninitializedData);
	InsertItem<unsigned long>(6, L"Address of entry point", &NTH.OptionalHeader.AddressOfEntryPoint);
	InsertItem<unsigned long>(7, L"Base of code", &NTH.OptionalHeader.BaseOfCode);
	
	if(g_platform == PLATFORM_X86)
	{
		InsertItem<unsigned long>(8, L"Base of data", &r_BaseOfData);
		anum = 8;
	}

	InsertItem<unsigned long long>(anum + 1, L"Image base", &NTH.OptionalHeader.ImageBase);
	InsertItem<unsigned long>(anum + 2, L"SectionAlignment", &NTH.OptionalHeader.SectionAlignment);
	InsertItem<unsigned long>(anum + 3, L"FileAlignment", &NTH.OptionalHeader.FileAlignment);
	InsertItem<unsigned short>(anum + 4, L"Major operating system version", &NTH.OptionalHeader.MajorOperatingSystemVersion);
	InsertItem<unsigned short>(anum + 5, L"Minor operating system version", &NTH.OptionalHeader.MinorOperatingSystemVersion);
	InsertItem<unsigned short>(anum + 6, L"Major image version", &NTH.OptionalHeader.MajorImageVersion);
	InsertItem<unsigned short>(anum + 7, L"Minor image version", &NTH.OptionalHeader.MinorImageVersion);
	InsertItem<unsigned short>(anum + 8, L"Major subsystem version", &NTH.OptionalHeader.MajorSubsystemVersion);
	InsertItem<unsigned short>(anum + 9, L"Minor subsystem version", &NTH.OptionalHeader.MinorSubsystemVersion);
	InsertItem<unsigned long>(anum + 10, L"Win32 version value", &NTH.OptionalHeader.Win32VersionValue);
	InsertItem<unsigned long>(anum + 11, L"Size of image", &NTH.OptionalHeader.SizeOfImage);
	InsertItem<unsigned long>(anum + 12, L"Size of headers", &NTH.OptionalHeader.SizeOfHeaders);
	InsertItem<unsigned long>(anum + 13, L"Check sum", &NTH.OptionalHeader.CheckSum);
	InsertItem<unsigned short>(anum + 14, L"Subsystem", &NTH.OptionalHeader.Subsystem);
	LI.iSubItem = 2;
	switch(NTH.OptionalHeader.Subsystem)
	{
	case 1 : LI.pszText = L"Driver file"; break;
	case 2 : LI.pszText = L"GUI file"; break;
	case 3 : LI.pszText = L"CUI file"; break;
	}
	ListView_SetItem(commonlist, &LI);
	InsertItem<unsigned short>(anum + 15, L"DLL characteristics", &NTH.OptionalHeader.DllCharacteristics);
	LI.iSubItem = 2;
	string str = "";
	unsigned long character = NTH.OptionalHeader.DllCharacteristics;
	if ((character & 0x0040) == 0x0040) { if (str != "") str += " / "; str += "Dynamic base"; }
	if ((character & 0x0080) == 0x0080) { if (str != "") str += " / "; str += "Force intergrity"; }
	if ((character & 0x0100) == 0x0100) { if (str != "") str += " / "; str += "Compatible with DEP"; }
	if ((character & 0x0200) == 0x0200) { if (str != "") str += " / "; str += "No isolation"; }
	if ((character & 0x0400) == 0x0400) { if (str != "") str += " / "; str += "No SEH"; }
	if ((character & 0x0800) == 0x0800) { if (str != "") str += " / "; str += "No bind"; }
	if ((character & 0x2000) == 0x2000) { if (str != "") str += " / "; str += "WDM driver"; }
	if ((character & 0x8000) == 0x8000) { if (str != "") str += " / "; str += "Terminal server aware"; }
	memset(buffer, 0, sizeof(wchar_t) * 255);

	for (int i = 0; i < str.size(); i++)
		buffer[i] = str[i];

	LI.pszText = buffer;
	ListView_SetItem(commonlist, &LI);
	InsertItem<unsigned long long>(anum + 16, L"Size of stack reserve", &NTH.OptionalHeader.SizeOfStackReserve);
	InsertItem<unsigned long long>(anum + 17, L"Size of stack commit", &NTH.OptionalHeader.SizeOfStackCommit);
	InsertItem<unsigned long long>(anum + 18, L"Size of heap reserve", &NTH.OptionalHeader.SizeOfHeapReserve);
	InsertItem<unsigned long long>(anum + 19, L"Size of heap commit", &NTH.OptionalHeader.SizeOfHeapCommit);
	InsertItem<unsigned long>(anum + 20, L"Loader flags", &NTH.OptionalHeader.LoaderFlags);
}

void DataDirectoryState::Enter()
{
	DestroyWindow(commonlist);

	DestroyWindow(commonmenu);
	DestroyWindow(ntmenu);
	CreateDataDirectoryTab();
	CreateNTHeaderTab();
	CreateCommonTab();
	TabCtrl_SetCurSel(ntmenu, 2);
	TabCtrl_SetCurSel(commonmenu, 1);

	if(directorystate != NULL)
		directorystate -> Escape();
	directorystate = State::exportdirectory;
	directorystate -> Enter();
}

void DataDirectoryState::Escape()
{
	CreateCommonList();
	DestroyWindow(directorymenu);
	
	if(directorystate != NULL)
		directorystate -> Escape();
	directorystate = NULL;
}

void ExportDirectoryState::Enter()
{
	CreateDataDirectoryList();
	CreateExportDirectoryColumn();

	if(bHasIED == true)
	{
		GetStringFromFile(IED.Name, buffer);
		SendMessage(directorybox, LB_ADDSTRING, 0, (LPARAM)buffer);
	}
}

void ExportDirectoryState::Escape()
{
	DestroyWindow(directorylist);
	DestroyWindow(directorybox);

	if(bHasIED == true)
	{
		SendMessage(directorybox, LB_RESETCONTENT, 0, 0);
		ListView_DeleteAllItems(directorylist);
	}
}

void ImportDirectoryState::Enter()
{
	CreateDataDirectoryList();
	CreateImportDirectoryColumn();
	
	if(bHasIID == true)
	{
		for(int i = 0; i < sizeofIID; i++)
		{
			GetStringFromFile(pIID[i].Name, buffer);
			IdxTable[i] = SendMessage(directorybox, LB_ADDSTRING, 0, (LPARAM)buffer);
		}
	}
}

void ImportDirectoryState::Escape()
{
	DestroyWindow(directorylist);
	DestroyWindow(directorybox);

	if(bHasIID == true)
	{
		SendMessage(directorybox, LB_RESETCONTENT, 0, 0);
		ListView_DeleteAllItems(directorylist);
	}
}

void ImportDirectoryState::SetFuncTable()
{
	FuncTable = new int[sizeofIID];

	if(g_platform == PLATFORM_X86)
		for(int i = 0; i < sizeofIID; i++)
			FuncTable[i] = CountFileElemTillEnd<unsigned int>(pIID[i].OriginalFirstThunk);
	else if(g_platform == PLATFORM_X64)
		for (int i = 0; i < sizeofIID; i++)
			FuncTable[i] = CountFileElemTillEnd<unsigned long long>(pIID[i].OriginalFirstThunk);
}

void SectionHeaderState::Enter()
{
	DestroyWindow(commonmenu);
	if(!bIsNullState) CreateSectionHeaderTab();
	CreateCommonTab();
	TabCtrl_SetCurSel(commonmenu, 2);

	if(substate != NULL)
		substate -> Escape();
	substate = &State::section[0];
	if(substate != NULL) 
		substate -> Enter();
}

void SectionHeaderState::Escape()
{
	DestroyWindow(sectionmenu);

	if(substate != NULL)
		substate -> Escape();
	substate = NULL;
}

void SectionState::Enter()
{
	InsertItem<unsigned long>(0, L"Virtual size", &pSH[SectionIdx].VirtualSize);
	InsertItem<unsigned long>(1, L"Virtual address", &pSH[SectionIdx].VirtualAddress);
	InsertItem<unsigned long>(2, L"Size of raw data", &pSH[SectionIdx].SizeOfRawData);
	InsertItem<unsigned long>(3, L"Pointer to raw data(offset)", &pSH[SectionIdx].PointerToRawData);
	InsertItem<unsigned long>(4, L"Pointer to relocations", &pSH[SectionIdx].PointerToRelocations);
	InsertItem<unsigned long>(5, L"Pointer to line numbers", &pSH[SectionIdx].PointerToLinenumbers);
	InsertItem<unsigned short>(6, L"Number of reloactions", &pSH[SectionIdx].NumberOfRelocations);
	InsertItem<unsigned short>(7, L"Number of line numbers", &pSH[SectionIdx].NumberOfLinenumbers);
	InsertItem<unsigned long>(8, L"Characteristics", &pSH[SectionIdx].Characteristics);
	LI.iSubItem = 2;
	string str = "";
	unsigned long character = pSH[SectionIdx].Characteristics;
	if((character & 0x80000000) == 0x80000000) {if(str != "") str += " / "; str += "Writable";}
	if((character & 0x40000000) == 0x40000000) {if(str != "") str += " / "; str += "Readable";}
	if((character & 0x20000000) == 0x20000000) {if(str != "") str += " / "; str += "Executable";}
	if((character & 0x00000080) == 0x00000080) {if(str != "") str += " / "; str += "Uninitialized data";}
	if((character & 0x00000040) == 0x00000040) {if(str != "") str += " / "; str += "Initialized data";}
	if((character & 0x00000020) == 0x00000020) {if(str != "") str += " / "; str += "Code";}
	memset(buffer, 0, sizeof(wchar_t) * 255);

	for(int i = 0; i < str.size(); i++)
		buffer[i] = str[i];

	LI.pszText = buffer;
	ListView_SetItem(commonlist, &LI);
}