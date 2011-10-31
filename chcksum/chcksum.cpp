// chcksum.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "chcksum.h"
#include "ChecksumDialog.h"

CAppModule _Module;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);	

	::CoInitialize(NULL);
	::AtlInitCommonControls(ICC_WIN95_CLASSES);

	_Module.Init(NULL, hInstance);

	CChecksumDialog().DoModal();

	_Module.Term();

	::CoUninitialize();

	return 0;
}
