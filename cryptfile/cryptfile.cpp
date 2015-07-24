// cryptfile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <sha.h>
#include <pwdbased.h>
#include <salsa.h>
#include <osrng.h>

#define CHUNK_SIZE (16*1024)

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		_tprintf(_T("usage: cryptfile [-d] [-o <outfile>] <password> <file>\n"));
		_tprintf(_T("where\n"));
		_tprintf(_T("\t-o <outfile> filename for non inplace encryption\n"));
		_tprintf(_T("\t-d decrypt file\n"));
		return 1;
	}

	bool bDecrypt = false;
	bool bUseTempFile = true;
	TCHAR szSourceFile[MAX_PATH];
	TCHAR szTargetFile[MAX_PATH];

	for (int i = 1; i < argc - 2; i++)
	{
		if (_tcscmp(_T("-d"), argv[i]) == 0)
			bDecrypt = true;
		if (_tcscmp(_T("-o"), argv[i]) == 0)
		{
			_tcscpy_s(szTargetFile, MAX_PATH, argv[i + 1]);
			bUseTempFile = false;
		}
	}

	if (bUseTempFile)
	{
		_tcscpy_s(szTargetFile, MAX_PATH, argv[argc - 1]);
		_tcscpy_s(szSourceFile, MAX_PATH, argv[argc - 1]);
		_tcscat_s(szSourceFile, MAX_PATH, _T(".tmp"));
		if (!::MoveFile(szTargetFile, szSourceFile))
		{
			_tprintf(_T("File access denied: %s\n"), szTargetFile);
			return 1;
		}
	}
	else
	{
		_tcscpy_s(szSourceFile, MAX_PATH, argv[argc - 1]);
	}

	unsigned char pKey[32];
	unsigned char pSalt[4]; // unique for this application
	pSalt[0] = 'C';
	pSalt[1] = 'S';
	pSalt[2] = 'X';
	pSalt[3] = '1';

	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA1> pbkdf2;
	pbkdf2.DeriveKey(
		pKey, sizeof(pKey), 0,
		reinterpret_cast<const unsigned char*>(argv[argc - 2]), _tcslen(argv[argc - 2]) * sizeof(TCHAR),
		pSalt, sizeof(pSalt), 1);

	HANDLE hSourceFile = ::CreateFile(szSourceFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSourceFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("File access denied: %s\n"), szSourceFile);
		return 1;
	}

	HANDLE hTargetFile = ::CreateFile(szTargetFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTargetFile == INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hSourceFile);
		_tprintf(_T("File access denied: %s\n"), szTargetFile);
		return 1;
	}

	BYTE *pData = new BYTE[CHUNK_SIZE];
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;

	unsigned char iv[8];

	if (bDecrypt)
	{
		// load file unique number for decryption
		::ReadFile(hSourceFile, iv, sizeof(iv), &dwBytesRead, NULL);
	}
	else
	{
		// generate number to used once (unique for each file)
		CryptoPP::AutoSeededRandomPool().GenerateBlock(iv, sizeof(iv));
		::WriteFile(hTargetFile, iv, sizeof(iv), &dwBytesWritten, NULL);
	}

	CryptoPP::Salsa20::Encryption salsa;
	salsa.SetKeyWithIV(pKey, sizeof(pKey), iv);
	
	while (::ReadFile(hSourceFile, pData, CHUNK_SIZE, &dwBytesRead, NULL) && dwBytesRead > 0)
	{
		salsa.ProcessData(pData, pData, dwBytesRead);  // inplace
		::WriteFile(hTargetFile, pData, dwBytesRead, &dwBytesWritten, NULL);
	}

	delete[] pData;

	::CloseHandle(hTargetFile);
	::CloseHandle(hSourceFile);

	if (bUseTempFile)
		::DeleteFile(szSourceFile);

#ifdef _DEBUG
	_tsystem(_T("pause"));
#endif 
	return 0;
}

