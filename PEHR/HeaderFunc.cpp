#include "CommonHeader.h"

PEHR_IMAGE_DOS_HEADER DosH;
PEHR_IMAGE_SECTION_HEADER* pSH = NULL;
PEHR_IMAGE_IMPORT_DESCRIPTOR* pIID = NULL;
PEHR_IMAGE_EXPORT_DIRECTORY IED;
PEHR_IMAGE_NT_HEADERS64 NTH = {0, };

char PEHeader[0x500];
char* PEFile = NULL;
unsigned long r_BaseOfData = 0;

unsigned long ConvertRVAToOffset(unsigned long rva);

void CheckMAllocObjects(STRUCT_MALLOC* m)
{
	if(!bIsNullState) 
	{
		m -> tpSH = pSH;
		m -> tPEFile = PEFile;

		if(bHasIID)
		{
			m -> tpIID = pIID;
			m -> tIdxTable = State::importdirectory -> GetIdxTable();
			m -> tFuncTable = State::importdirectory -> GetFuncTable();
		}
	}
}

void DeleteMAllocObjects(STRUCT_MALLOC* m)
{
	if(m -> tpSH != NULL)
	{
		delete[] m -> tpSH;
		delete[] m -> tPEFile;

		if(m -> tpIID != NULL)
		{
			delete[] m -> tpIID;
			delete[] m -> tIdxTable;
			delete[] m -> tFuncTable;
		}
	}

	memset(m, NULL, sizeof(STRUCT_MALLOC));
}

bool LoadBinaryFile(TCHAR* adr)
{
	ifstream input(adr, ios::in | ios::binary);

	if(!input || input.fail())
	{
		MessageBox(NULL, L"Can't open file", L"Error", MB_OK);
		return false;
	}

	for(int i = 0; i < sizeof(PEHeader); i++)
		PEHeader[i] = input.get();

	memcpy(&DosH, PEHeader, sizeof(DosH));

	unsigned short* optmagic = (unsigned short*)(PEHeader + DosH.e_lfanew + 24);

	if (*optmagic == 0x010B)
	{
		PEHR_IMAGE_NT_HEADERS32 NTH32;
		memcpy(&NTH32, PEHeader + DosH.e_lfanew, sizeof(PEHR_IMAGE_NT_HEADERS32));
		memcpy(&NTH, &NTH32, 48);
		r_BaseOfData = NTH32.OptionalHeader.BaseOfData;
		NTH.OptionalHeader.ImageBase = NTH32.OptionalHeader.ImageBase;
		memcpy((char*)&NTH.OptionalHeader.ImageBase + 8, (char*)&NTH32.OptionalHeader.ImageBase + 4, 40);
		NTH.OptionalHeader.SizeOfStackReserve = NTH32.OptionalHeader.SizeOfStackReserve;
		NTH.OptionalHeader.SizeOfStackCommit = NTH32.OptionalHeader.SizeOfStackCommit;
		NTH.OptionalHeader.SizeOfHeapReserve = NTH32.OptionalHeader.SizeOfHeapReserve;
		NTH.OptionalHeader.SizeOfHeapCommit = NTH32.OptionalHeader.SizeOfHeapCommit;
		memcpy((char*)&NTH.OptionalHeader.SizeOfHeapCommit + 8, (char*)&NTH32.OptionalHeader.SizeOfHeapCommit + 4, 136);

		pSH = new PEHR_IMAGE_SECTION_HEADER[NTH.FileHeader.NumberOfSections];
		memcpy(pSH, PEHeader + DosH.e_lfanew + sizeof(PEHR_IMAGE_NT_HEADERS32), NTH.FileHeader.NumberOfSections * 40);
		g_platform = PLATFORM_X86;
	}
	else if (*optmagic == 0x020B)
	{
		memcpy(&NTH, PEHeader + DosH.e_lfanew, sizeof(PEHR_IMAGE_NT_HEADERS64));

		pSH = new PEHR_IMAGE_SECTION_HEADER[NTH.FileHeader.NumberOfSections];
		memcpy(pSH, PEHeader + DosH.e_lfanew + sizeof(PEHR_IMAGE_NT_HEADERS64), NTH.FileHeader.NumberOfSections * 40);
		g_platform = PLATFORM_X64;
	}

	int sizeofFile = pSH[NTH.FileHeader.NumberOfSections - 1].PointerToRawData + 
		pSH[NTH.FileHeader.NumberOfSections - 1].SizeOfRawData;
	PEFile = new char[sizeofFile];
	input.seekg(ios_base::beg);

	for(int i = 0; i < sizeofFile; i++)
		PEFile[i] = input.get();

	input.close();

	int beginoffset = ConvertRVAToOffset(NTH.OptionalHeader.DataDirectory[1].VirtualAddress);
	if(beginoffset != 0)
	{
		int sizeofIID = CountFileElemTillEnd<PEHR_IMAGE_IMPORT_DESCRIPTOR>
			(NTH.OptionalHeader.DataDirectory[1].VirtualAddress);
		pIID = new PEHR_IMAGE_IMPORT_DESCRIPTOR[sizeofIID];
		memcpy(pIID, &PEFile[beginoffset], sizeofIID * sizeof(PEHR_IMAGE_IMPORT_DESCRIPTOR));
	
		State::importdirectory -> SetSize(sizeofIID);
		State::importdirectory -> SetIdxTable();
		State::importdirectory -> SetFuncTable();
		bHasIID = true;
	}
	else bHasIID = false;

	beginoffset = ConvertRVAToOffset(NTH.OptionalHeader.DataDirectory[0].VirtualAddress);
	if(beginoffset != 0) 
	{
		memcpy(&IED, &PEFile[beginoffset], sizeof(IED));
		State::exportdirectory -> SetnFunc(IED.NumberOfNames);
		bHasIED = true;
	}
	else bHasIED = false;

	return true;
}

unsigned long SwapDwordEndian(unsigned long value)
{
	return ((value & 0x000000ff) << 24)
		| ((value & 0x0000ff00) << 8)
		| ((value & 0x00ff0000) >> 8)
		| ((value & 0xff000000) >> 24);
}

unsigned long ConvertRVAToOffset(unsigned long rva)
{
	int nos = NTH.FileHeader.NumberOfSections;

	for(int i = nos - 1; i >= 0; i--)
	{
		if(rva >= pSH[i].VirtualAddress)
			return (rva - pSH[i].VirtualAddress) + pSH[i].PointerToRawData;
	}

	return 0;
}

void GetStringFromFile(unsigned long rva, wchar_t* buffer)
{
	unsigned long offset = ConvertRVAToOffset(rva);
	int nElem = CountFileElemTillEnd<char>(rva);
	memset(buffer, 0, sizeof(wchar_t) * 255);
	
	for(int i = 0; i < nElem; i++)
		buffer[i] = PEFile[offset + i];
}
