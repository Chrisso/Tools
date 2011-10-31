// LineCountApp.cpp : Definiert den Einstiegspunkt für die Anwendung.
// 04/2005 Christoph Stoepel (christoph-stoepel@web.de)

#include "stdafx.h"
#include "resource.h"
#include "LineCountGui.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	TCHAR szTitle[127];	 // Titelleistentext
	LoadString(hInstance, IDS_APP_TITLE, szTitle, 127);

	CLineCountGui gui;
	if (gui.Create(NULL, CWindow::rcDefault, szTitle) == NULL)
		return 1;

	gui.ShowWindow(nCmdShow);
	gui.UpdateWindow();

	// Hauptmeldungsschleife:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}