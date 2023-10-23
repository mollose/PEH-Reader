#include "CommonHeader.h"
#include "resource.h"
#include "tchar.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LPWSTR lpszClass = L"PEH Reader";

HINSTANCE g_hInst;
HWND g_hWnd;
State * state;
State * substate;
State * directorystate;

DosHeaderState* State::dosheader = new DosHeaderState();
NTHeaderState* State::ntheader = new NTHeaderState();
FileHeaderState* State::fileheader = new FileHeaderState();
OptionalHeaderState* State::optionalheader = new OptionalHeaderState();
DataDirectoryState* State::datadirectory = new DataDirectoryState();
ExportDirectoryState* State::exportdirectory = new ExportDirectoryState();
ImportDirectoryState* State::importdirectory = new ImportDirectoryState();
SectionHeaderState* State::sectionheader = new SectionHeaderState();
SectionState* State::section = NULL;

int g_platform;
bool bIsNullState = true;
bool bHasIID = false;
bool bHasIED = false;
TCHAR FilePath[MAX_PATH];
STRUCT_MALLOC tempMAlloc;

int APIENTRY WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	HBRUSH BackBrush = CreateSolidBrush(RGB(236, 233, 216));

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = BackBrush;
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
		300, 15, 607, 700, NULL, (HMENU)NULL, hInstance, NULL);
	g_hWnd = hWnd;
	ShowWindow(hWnd, nCmdShow);

	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	while(GetMessage(&Message, 0, 0, 0))
	{
		if(!TranslateAccelerator(hWnd, hAccel, &Message))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;
	HDROP hDrop;
	TCHAR strFile[MAX_PATH] = L"";
	TCHAR strTemp[MAX_PATH];

	switch(iMessage)
	{
	case WM_CREATE :
		g_hWnd = hWnd;
		CreateCommonTab();
		CreateCommonList();
		state = State::dosheader;
		substate = NULL;
		directorystate = NULL;
		DragAcceptFiles(hWnd, TRUE);
		return 0;
	case WM_NOTIFY :
		switch(((LPNMHDR)lParam) -> code)
		{
		case TCN_SELCHANGE :
			{
				int cPage = TabCtrl_GetCurSel(commonmenu);
				switch(cPage)
				{
				case 0 :
					if(state != State::dosheader)
					{
						state -> Escape();
						state = State::dosheader;
						if(!bIsNullState) state -> Enter();
					}
					break;
				case 1 :
					if(state != State::ntheader)
					{
						state -> Escape();
						state = State::ntheader;
						if(!bIsNullState) state -> Enter();
					}
					break;
				case 2 :
					if(state != State::sectionheader)
					{
						state -> Escape();
						state = State::sectionheader;
						if(!bIsNullState) state -> Enter();
					}
					break;
				}

				if(state == State::ntheader)
				{
					int ntpage = TabCtrl_GetCurSel(ntmenu);
					switch(ntpage)
					{
					case 0 :
						if(substate != State::fileheader)
						{
							if(substate != NULL)
								substate -> Escape();
							substate = State::fileheader;
							if(!bIsNullState) substate -> Enter();
						}
						break;
					case 1 :
						if(substate != State::optionalheader)
						{
							if(substate != NULL)
								substate -> Escape();
							substate = State::optionalheader;
							if(!bIsNullState) substate -> Enter();
						}
						break;
					case 2 :
						if(substate != State::datadirectory)
						{
							if(substate != NULL)
								substate -> Escape();
							substate = State::datadirectory;
							if(!bIsNullState) substate -> Enter();
						}
						break;
					}
				}
				else if(state == State::sectionheader)
				{
					int sectionpage = TabCtrl_GetCurSel(sectionmenu);
					if(substate != &State::section[sectionpage] && sectionpage != -1)
					{
						if(substate != NULL)
							substate -> Escape();
						substate = &State::section[sectionpage];
						substate -> Enter();
					}
				}

				if(substate == State::datadirectory)
				{
					int directorypage = TabCtrl_GetCurSel(directorymenu);
					switch(directorypage)
					{
					case 0 :
						if(directorystate != State::exportdirectory)
						{
							if(directorystate != NULL)
								directorystate -> Escape();
							directorystate = State::exportdirectory;
							directorystate -> Enter();
						}
						break;
					case 1 :
						if(directorystate != State::importdirectory)
						{
							if(directorystate != NULL)
								directorystate -> Escape();
							directorystate = State::importdirectory;
							directorystate -> Enter();
						}
						break;
					}
				}
			}
		}
		return 0;
	case WM_COMMAND :
		switch(LOWORD(wParam))
		{
		case ID_LISTBOX :
			switch(HIWORD(wParam))
			{
			case LBN_SELCHANGE :
				ListView_DeleteAllItems(directorylist);

				if(directorystate == State::exportdirectory && bHasIED == true)
				{
					int nfunc = State::exportdirectory -> GetnFunc();
					int alpha = 0;

					for(int i = 0; i < nfunc; i++)
						alpha += InsertExportDirectoryItem(i, alpha);
				}
				else if(directorystate == State::importdirectory && bHasIID == true)
				{
					int idx = State::importdirectory -> SearchIdxTable(SendMessage(directorybox, LB_GETCURSEL, 0, 0));
					int nfunc = State::importdirectory -> SearchFuncTable(idx);
					int alpha = 0;

					for (int i = 0; i < nfunc; i++)
						alpha += InsertImportDirectoryItem(i, idx, alpha);
				}
				break;
			}
			break;
		case ID_FILE_OPEN :
			memset(&ofn, 0, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = g_hWnd;
			ofn.lpstrFilter = L"PE File(*.exe; *.scr; *.dll; *.ocx; *sys)\0*.exe;*.scr;*.dll;*.ocx;*.sys";
			ofn.lpstrFile = strFile;
			ofn.nMaxFile = MAX_PATH;

			if(GetOpenFileName(&ofn) != 0)
			{
				wsprintf(strTemp, L"%s", ofn.lpstrFile);
				_tcscpy(FilePath, strTemp);

				CheckMAllocObjects(&tempMAlloc);

				if(LoadBinaryFile(FilePath))

				{
					state -> Escape();

					if(!bIsNullState) delete[] State::section;
					State::section = new SectionState[NTH.FileHeader.NumberOfSections];

					for(int i = 0; i < NTH.FileHeader.NumberOfSections; i++)
						State::section[i].SetIdx(i);
		
					int cutidx = 0;
					TCHAR ws[MAX_PATH] = L"PEH Reader - ";
					TCHAR* pcut = _tcsrchr(strTemp, L'\\');
					_tcscpy(&ws[13], pcut + 1);
					_tcsset(strTemp, 0);
					_tcscpy(strTemp, ws);

					if (g_platform == PLATFORM_X86)
						_tcscpy(&strTemp[_tcslen(strTemp)], L" (32bit)");
					else if (g_platform == PLATFORM_X64)
						_tcscpy(&strTemp[_tcslen(strTemp)], L" (64bit)");

					SetWindowText(hWnd, strTemp);
					state = State::dosheader;
					TabCtrl_SetCurSel(commonmenu, 0);
					substate = NULL;
					directorystate = NULL;
					state -> Enter();

					DeleteMAllocObjects(&tempMAlloc);
					bIsNullState = false;
				}
			}
			break;
		}
		return 0;
	case WM_DROPFILES : 
		hDrop = (HDROP)wParam;
		DragQueryFile(hDrop, 0, strTemp, MAX_PATH);
		_tcscpy(FilePath, strTemp);

		CheckMAllocObjects(&tempMAlloc);

		if (LoadBinaryFile(FilePath))
		{
			state->Escape();

			if (!bIsNullState) delete[] State::section;
			State::section = new SectionState[NTH.FileHeader.NumberOfSections];

			for (int i = 0; i < NTH.FileHeader.NumberOfSections; i++)
				State::section[i].SetIdx(i);

			int cutidx = 0;
			TCHAR ws[MAX_PATH] = L"PEH Reader - ";
			TCHAR* pcut = _tcsrchr(strTemp, L'\\');
			_tcscpy(&ws[13], pcut + 1);
			_tcsset(strTemp, 0);
			_tcscpy(strTemp, ws);

			if (g_platform == PLATFORM_X86)
				_tcscpy(&strTemp[_tcslen(strTemp)], L" (32bit)");
			else if (g_platform == PLATFORM_X64)
				_tcscpy(&strTemp[_tcslen(strTemp)], L" (64bit)");

			SetWindowText(hWnd, strTemp);
			state = State::dosheader;
			TabCtrl_SetCurSel(commonmenu, 0);
			substate = NULL;
			directorystate = NULL;
			state->Enter();

			DeleteMAllocObjects(&tempMAlloc);
			bIsNullState = false;
		}
		return 0;
	case WM_DESTROY :
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}