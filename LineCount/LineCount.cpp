// LineCount.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
// (c) 2005 Christoph Stöpel

#include "stdafx.h"

int g_LineCount; // globale Variable, wegen Zählung in Rekursion

// Liest die Anzahl der Zeilen in einer Datei
int ProcessFile(TCHAR *name)
{
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

// Prüft, ob Datei dem Filter entspricht
bool CheckFileExtension(TCHAR *name)
{
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

// durchläuft alle Dateien eines Verzeichnisse und rekursiv 
// auch alle Unterverzeichnisse
void ProcessDirectory(TCHAR *name)
{
	_tprintf(_T("Enter Directory %s...\n"), name);
	TCHAR mask[MAX_PATH]; _stprintf(mask, _T("%s\\*"), name);
	WIN32_FIND_DATA fd;
	HANDLE hFindFile = FindFirstFile(mask, &fd);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			_tcscmp(fd.cFileName, _T("."))  != 0 && 
			_tcscmp(fd.cFileName, _T("..")) != 0)
		{
			TCHAR *SubDir = new TCHAR[MAX_PATH];
			PathCombine(SubDir, name, fd.cFileName);
			ProcessDirectory(SubDir);
			delete[] SubDir;
		}
		if (CheckFileExtension(fd.cFileName))
		{
			TCHAR FileName[MAX_PATH]; PathCombine(FileName, name, fd.cFileName);
			int LineCount = ProcessFile(FileName);
			TCHAR compact[40];
			PathCompactPathEx(compact, FileName, 40, 0);
			_tprintf(_T("   %-45s %8d\n"), compact, LineCount);
			g_LineCount += LineCount;
		}
		while (FindNextFile(hFindFile, &fd))
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			_tcscmp(fd.cFileName, _T("."))  != 0 && 
			_tcscmp(fd.cFileName, _T("..")) != 0)
			{
				TCHAR *SubDir = new TCHAR[MAX_PATH];
				PathCombine(SubDir, name, fd.cFileName);
				ProcessDirectory(SubDir);
				delete[] SubDir;
			}
			if (CheckFileExtension(fd.cFileName))
			{
				TCHAR FileName[MAX_PATH]; PathCombine(FileName, name, fd.cFileName);
				int LineCount = ProcessFile(FileName);
				TCHAR compact[40];
				PathCompactPathEx(compact, FileName, 40, 0);
				_tprintf(_T("   %-45s %8d\n"), compact, LineCount);
				g_LineCount += LineCount;
			}
		}
		FindClose(hFindFile);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	g_LineCount = 0;
	#ifdef _DEBUG
		_tprintf(_T("Application startup with %d Arguments\n"), argc);
	#endif
	if (argc == 1)
	{
		_tprintf(_T("Counts Lines of Code in all Files and Directories of your Project\n"));
		_tprintf(_T("Usage: LineCount.exe <Directory>\n"));
		return 0;
	}
	ProcessDirectory(argv[1]);
	_tprintf(_T("\nTotal Lines of Code: %d\n"), g_LineCount);
	#ifdef _DEBUG
		_tprintf(_T("Application End. Press [Enter].\n"));
		_gettchar();
	#endif
	return 0;
}

