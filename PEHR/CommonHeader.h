#ifndef __COMMONHEADER_H__
#define __COMMONHEADER_H__

#include <string>
#include <fstream>
#include "windows.h"
#include "State.h"
#include "commctrl.h"

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES	16
#define IMAGE_SIZEOF_SHORT_NAME				8
#define ID_LISTBOX							100
#define PLATFORM_X86						10
#define PLATFORM_X64						20

using namespace std;

struct PEHR_IMAGE_DOS_HEADER
{
	unsigned short e_magic;
	unsigned short DosHeaderOffset[29];
	unsigned long e_lfanew;
};

struct PEHR_IMAGE_DATA_DIRECTORY
{
	unsigned long VirtualAddress;
	unsigned long SIze;
};

struct PEHR_IMAGE_OPTIONAL_HEADER32
{
	unsigned short Magic;
	unsigned char MajorLinkerVersion;
	unsigned char MinorLinkerVersion;
	unsigned long SizeOfCode;
	unsigned long SizeOfInitializedData;
	unsigned long SizeOfUninitializedData;
	unsigned long AddressOfEntryPoint;
	unsigned long BaseOfCode;
	unsigned long BaseOfData;
	unsigned long ImageBase;
	unsigned long SectionAlignment;
	unsigned long FileAlignment;
	unsigned short MajorOperatingSystemVersion;
	unsigned short MinorOperatingSystemVersion;
	unsigned short MajorImageVersion;
	unsigned short MinorImageVersion;
	unsigned short MajorSubsystemVersion;
	unsigned short MinorSubsystemVersion;
	unsigned long Win32VersionValue;
	unsigned long SizeOfImage;
	unsigned long SizeOfHeaders;
	unsigned long CheckSum;
	unsigned short Subsystem;
	unsigned short DllCharacteristics;
	unsigned long SizeOfStackReserve;
	unsigned long SizeOfStackCommit;
	unsigned long SizeOfHeapReserve;
	unsigned long SizeOfHeapCommit;
	unsigned long LoaderFlags;
	unsigned long NumberOfRvaAndSizes;	
	PEHR_IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

extern unsigned long r_BaseOfData;

struct PEHR_IMAGE_OPTIONAL_HEADER64
{
	unsigned short Magic;
	unsigned char MajorLinkerVersion;
	unsigned char MinorLinkerVersion;
	unsigned long SizeOfCode;
	unsigned long SizeOfInitializedData;
	unsigned long SizeOfUninitializedData;
	unsigned long AddressOfEntryPoint;
	unsigned long BaseOfCode;
	unsigned long long ImageBase;
	unsigned long SectionAlignment;
	unsigned long FileAlignment;
	unsigned short MajorOperatingSystemVersion;
	unsigned short MinorOperatingSystemVersion;
	unsigned short MajorImageVersion;
	unsigned short MinorImageVersion;
	unsigned short MajorSubsystemVersion;
	unsigned short MinorSubsystemVersion;
	unsigned long Win32VersionValue;
	unsigned long SizeOfImage;
	unsigned long SizeOfHeaders;
	unsigned long CheckSum;
	unsigned short Subsystem;
	unsigned short DllCharacteristics;
	unsigned long long SizeOfStackReserve;
	unsigned long long SizeOfStackCommit;
	unsigned long long SizeOfHeapReserve;
	unsigned long long SizeOfHeapCommit;
	unsigned long LoaderFlags;
	unsigned long NumberOfRvaAndSizes;
	PEHR_IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct PEHR_IMAGE_FILE_HEADER
{
	unsigned short Machine;
	unsigned short NumberOfSections;
	unsigned long TimeDateStamp;
	unsigned long PointerToSymbolTable;
	unsigned long NumberOfSymbols;
	unsigned short SizeOfOptionalHeader;
	unsigned short Characteristics;
};

struct PEHR_IMAGE_NT_HEADERS32
{
	unsigned long Signature;
	PEHR_IMAGE_FILE_HEADER FileHeader;
	PEHR_IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

struct PEHR_IMAGE_NT_HEADERS64
{
	unsigned long Signature;
	PEHR_IMAGE_FILE_HEADER FileHeader;
	PEHR_IMAGE_OPTIONAL_HEADER64 OptionalHeader;
};

struct PEHR_IMAGE_SECTION_HEADER
{
	unsigned char Name[IMAGE_SIZEOF_SHORT_NAME];
	unsigned long VirtualSize;
	unsigned long VirtualAddress;
	unsigned long SizeOfRawData;
	unsigned long PointerToRawData;
	unsigned long PointerToRelocations;
	unsigned long PointerToLinenumbers;
	unsigned short NumberOfRelocations;
	unsigned short NumberOfLinenumbers;
	unsigned long Characteristics;
};

struct PEHR_IMAGE_IMPORT_DESCRIPTOR
{
	unsigned long OriginalFirstThunk;
	unsigned long TimeDateStamp;
	unsigned long ForwarderChain;
	unsigned long Name;
	unsigned long FirstThunk;
};

struct PEHR_IMAGE_EXPORT_DIRECTORY
{
	unsigned long Characteristics;
	unsigned long TimeDateStamp;
	unsigned short MajorVersion;
	unsigned short MinorVersion;
	unsigned long Name;
	unsigned long Base;
	unsigned long NumberOfFunctions;
	unsigned long NumberOfNames;
	unsigned long AddressOfFunctions;
	unsigned long AddressOfNames;
	unsigned long AddressOfNameOrdinals;
};

struct STRUCT_MALLOC
{
	STRUCT_MALLOC() {memset(this, NULL, sizeof(STRUCT_MALLOC));}

	PEHR_IMAGE_SECTION_HEADER* tpSH;
	PEHR_IMAGE_IMPORT_DESCRIPTOR* tpIID;
	char* tPEFile;
	int* tIdxTable;
	int* tFuncTable;
};

extern PEHR_IMAGE_DOS_HEADER DosH;
extern PEHR_IMAGE_SECTION_HEADER* pSH;
extern PEHR_IMAGE_IMPORT_DESCRIPTOR* pIID;
extern PEHR_IMAGE_EXPORT_DIRECTORY IED;
extern PEHR_IMAGE_NT_HEADERS64 NTH;
extern char PEHeader[0x500];
extern char* PEFile;

extern HINSTANCE g_hInst;
extern HWND g_hWnd;
extern bool bHasIID;
extern bool bHasIED;
extern bool bIsNullState;
extern int g_platform;

extern State * state;
extern State * substate;
extern State * directorystate;

extern HWND commonmenu, ntmenu, sectionmenu, directorymenu;
extern HWND directorybox, directorylist;

extern LVITEM LI;
extern wchar_t buffer[255];
int InsertExportDirectoryItem(int idx, int alpha);
int InsertImportDirectoryItem(int idx, int libidx, int alpha);

bool LoadBinaryFile(TCHAR* adr);
void CheckMAllocObjects(STRUCT_MALLOC* m);
void DeleteMAllocObjects(STRUCT_MALLOC* m);
void CreateCommonTab();
void CreateCommonList();
unsigned long SwapDwordEndian(unsigned long value);
unsigned long ConvertRVAToOffset(unsigned long rva);
void GetStringFromFile(unsigned long rva, wchar_t* buffer);

template <class T>
void LoadPEHeaderValue(T* value, wchar_t* buffer)
{
	swprintf(buffer, L"%x", *value);
}

template <class T>
void InsertItem(int idx, LPWSTR text, T* value)
{
	LI.iItem = idx;
	LI.iSubItem = 0;
	LI.pszText = text;
	ListView_InsertItem(commonlist, &LI);

	LI.iSubItem = 1;
	memset(buffer, 0, sizeof(wchar_t) * 255);
	LoadPEHeaderValue<T>(value, buffer);
	LI.pszText = buffer;
	ListView_SetItem(commonlist, &LI);
}

template <class T>
int CountFileElemTillEnd(unsigned long rva)
{
	unsigned long offset = ConvertRVAToOffset(rva);
	int count = 0;
	T nullElem;
	memset(&nullElem, 0, sizeof(T));
	T container;
	
	while(1)
	{
		memcpy(&container, &PEFile[offset + (count * sizeof(T))], sizeof(T));
		if(memcmp(&container, &nullElem, sizeof(T)) == 0)
			break;
		count++;
	}

	return count;
}

#endif