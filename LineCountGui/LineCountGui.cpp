// LineCountGui.cpp : Implementation der Fensterklasse
// 04/2005 Christoph Stoepel (christoph-stoepel@web.de)

#include "stdafx.h"
#include "resource.h"
#include "LineCountGui.h"

#define MAX_DISPLAY_LENGTH 50

//////////////////////////////////////////////////////////////////////
// Konstruktoren/ Destruktoren
//////////////////////////////////////////////////////////////////////

CLineCountGui::CLineCountGui() : m_wndStatusBar(NULL), m_wndTreeView(NULL)
{
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccx.dwICC  = ICC_BAR_CLASSES | ICC_TREEVIEW_CLASSES;
	InitCommonControlsEx(&iccx);

	rcDefault.top    = 100;
	rcDefault.left   = 100;
	rcDefault.bottom = 500;
	rcDefault.right  = 550;
}

//////////////////////////////////////////////////////////////////////
// Windows Ereignisbehandlung
//////////////////////////////////////////////////////////////////////

LRESULT CLineCountGui::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_wndStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE, 
		0, 0, 0, 0, m_hWnd, (HMENU)IDC_STATUSBAR, _AtlBaseModule.GetModuleInstance(), NULL);

	m_wndTreeView = CreateWindowEx(0, WC_TREEVIEW, NULL, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | WS_BORDER | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, m_hWnd, (HMENU)IDC_LISTVIEW, _AtlBaseModule.GetModuleInstance(), NULL);

	TCHAR szStatus[127]; 
	LoadString(_AtlBaseModule.GetModuleInstance(), IDS_MSG_READY, szStatus, 127);
	SendMessage(m_wndStatusBar, SB_SETTEXT, 0, (LPARAM)szStatus);

	HMENU mainMenu = LoadMenu(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDC_LINECOUNTGUI));
	SetMenu(mainMenu);
	return 0;
}
	
LRESULT CLineCountGui::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DestroyWindow();
	return 0;
}
	
LRESULT CLineCountGui::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT CLineCountGui::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rct;
	::SendMessage(m_wndStatusBar, WM_SIZE, wParam, lParam);
	::GetWindowRect(m_wndStatusBar, &rct);
	::MoveWindow(m_wndTreeView, 0, 0, LOWORD(lParam), HIWORD(lParam) - (rct.bottom - rct.top), TRUE);
	return 0;
}

LRESULT CLineCountGui::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR szAbout[255];	LoadString(_AtlBaseModule.GetModuleInstance(), IDS_ABOUT, szAbout, 255);
	TCHAR szTitle[64];	LoadString(_AtlBaseModule.GetModuleInstance(), IDS_APP_TITLE, szTitle, 64);
	MessageBox(szAbout, szTitle, MB_ICONINFORMATION | MB_OK);
	return 0;
}

LRESULT CLineCountGui::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DestroyWindow();
	return 0;
}

LRESULT CLineCountGui::OnLineCount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR szSelectedFolder[MAX_PATH];

	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner       = m_hWnd;
	bi.pszDisplayName  = szSelectedFolder;
	bi.ulFlags         = BIF_RETURNONLYFSDIRS;
	LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl)
	{		
		SHGetPathFromIDList(pidl, szSelectedFolder);
		IMalloc *pShellMem = NULL;
		if (SUCCEEDED(SHGetMalloc(&pShellMem)) && pShellMem)
		{
			pShellMem->Free((void*)pidl);
			pShellMem->Release();
		}
		TreeView_DeleteAllItems(m_wndTreeView);
		int Count = ProcessDirectory(szSelectedFolder);	
		TCHAR szLoader[127]; LoadString(_AtlBaseModule.GetModuleInstance(), IDS_MSG_RESULT, szLoader, 127);
		TCHAR szStatus[127]; _sntprintf(szStatus, 127, szLoader, Count);
		SendMessage(m_wndStatusBar, SB_SETTEXT, 0, (LPARAM)szStatus);
		SendMessage(m_wndStatusBar, WM_PAINT, 0, 0);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Methoden
//////////////////////////////////////////////////////////////////////

HTREEITEM CLineCountGui::AddItemToTree(TCHAR *name, int count, HTREEITEM parent)
{	
	ATLASSERT(::IsWindow(m_hWnd));
	ATLASSERT(::IsWindow(m_wndTreeView));
	ATLASSERT(name != NULL);

	TCHAR szCaption[MAX_PATH];
	_sntprintf(szCaption, MAX_PATH, _T("%s - %d"), PathFindFileName(name), count);

	TVINSERTSTRUCT tvi;
	tvi.hParent      = parent;
	tvi.hInsertAfter = TVI_SORT;
	tvi.item.mask    = TVIF_TEXT;
	tvi.item.pszText = szCaption;
	return TreeView_InsertItem(m_wndTreeView, &tvi);
}

BOOL CLineCountGui::UpdateItemText(TCHAR *name, int count, HTREEITEM item)
{
	ATLASSERT(::IsWindow(m_hWnd));
	ATLASSERT(::IsWindow(m_wndTreeView));
	ATLASSERT(name != NULL);

	TCHAR szCaption[MAX_PATH];
	_sntprintf(szCaption, MAX_PATH, _T("%s - %d"), PathFindFileName(name), count);

	TVITEM tv;
	tv.mask    = TVIF_HANDLE | TVIF_TEXT;
	tv.hItem   = item;
	tv.pszText = szCaption;
	return TreeView_SetItem(m_wndTreeView, &tv);
}

int CLineCountGui::ProcessDirectory(TCHAR *name, HTREEITEM pTI)
{
	ATLASSERT(name != NULL);
	ATLTRACE(_T("Enter Directory %s...\n"), name);

	int DirectoryCount = 0;
	TCHAR mask[MAX_PATH]; _stprintf(mask, _T("%s\\*"), name);
	WIN32_FIND_DATA fd;
	HANDLE hFindFile = FindFirstFile(mask, &fd);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			_tcscmp(fd.cFileName, _T("."))  != 0 && 
			_tcscmp(fd.cFileName, _T("..")) != 0)
			{
				TCHAR *SubDir  = new TCHAR[MAX_PATH];
				PathCombine(SubDir, name, fd.cFileName);
				HTREEITEM item = AddItemToTree(SubDir, 0, pTI);
				int dircount   = ProcessDirectory(SubDir, item);
				if (dircount)
					UpdateItemText(SubDir, dircount, item);
				else
					TreeView_DeleteItem(m_wndTreeView, item);
				delete[] SubDir;
				DirectoryCount += dircount;
			}
			else if (CheckFileExtension(fd.cFileName))
			{
				TCHAR FileName[MAX_PATH]; 
				PathCombine(FileName, name, fd.cFileName);
				int LineCount = ProcessFile(FileName);
				AddItemToTree(FileName, LineCount, pTI);
				DirectoryCount += LineCount;
			}
		}
		while (FindNextFile(hFindFile, &fd));

		FindClose(hFindFile);
	}

	return DirectoryCount;
}

int CLineCountGui::ProcessFile(TCHAR *name)
{
	ATLASSERT(name != NULL);

	int LineCount = 0;
	HANDLE hFile = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, 
							  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		BYTE buffer[1024];
		DWORD dwBytesRead;
		if (ReadFile(hFile, (void*)buffer, 1024, &dwBytesRead, NULL))
		{
			while (dwBytesRead > 0)
			{
				for (DWORD i=0; i<dwBytesRead; i++)
					if (buffer[i] == 10)
						LineCount++;
				ReadFile(hFile, (void*)buffer, 1024, &dwBytesRead, NULL);
			}
		}
		CloseHandle(hFile);
	}
	return LineCount + 1;
}

bool CLineCountGui::CheckFileExtension(TCHAR *name) const
{
	ATLASSERT(name != NULL);

	TCHAR *ext = PathFindExtension(name);
	if (_tcsicmp(ext, _T(".cpp"))    == 0) return true;
	if (_tcsicmp(ext, _T(".h"))      == 0) return true;
	if (_tcsicmp(ext, _T(".cs"))     == 0) return true;
	if (_tcsicmp(ext, _T(".vb"))     == 0) return true;
	if (_tcsicmp(ext, _T(".sql"))    == 0) return true;
	if (_tcsicmp(ext, _T(".java"))   == 0) return true;
	if (_tcsicmp(ext, _T(".aspx"))   == 0) return true;
	if (_tcsicmp(ext, _T(".ascx"))   == 0) return true;
	if (_tcsicmp(ext, _T(".php"))    == 0) return true;
	if (_tcsicmp(ext, _T(".jsp"))    == 0) return true;
	if (_tcsicmp(ext, _T(".css"))    == 0) return true;
	if (_tcsicmp(ext, _T(".config")) == 0) return true;
	return false;
}

