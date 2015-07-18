// cryptfile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define SODIUM_STATIC
#include <sodium.h>

#define CHUNK_SIZE (16*1024)

int _tmain(int argc, _TCHAR* argv[])
{
	if (sodium_init() == -1)
	{
		_tprintf(_T("Could not initialize cryptography subsystem!\n"));
		return 1;
	}

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

	unsigned char pKey[crypto_stream_KEYBYTES];
	unsigned char pNonce[crypto_stream_NONCEBYTES];
	unsigned char pSalt[crypto_pwhash_scryptsalsa208sha256_SALTBYTES];

	// generate simple password salt (unique for this application)
	for (size_t i = 2; i < crypto_pwhash_scryptsalsa208sha256_SALTBYTES; i++)
		pSalt[i] = (i * 2) & 0xFF;
	pSalt[0] = 'C';
	pSalt[1] = 'S';

	// generate key (derived from password)
	crypto_pwhash_scryptsalsa208sha256(
		pKey, sizeof(pKey),
		reinterpret_cast<const char*>(argv[argc - 2]), _tcslen(argv[argc - 2]) * sizeof(TCHAR),
		pSalt,
		crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE,
		crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE);

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

	if (bDecrypt)
	{
		// load file unique number for decryption
		::ReadFile(hSourceFile, pNonce, sizeof(pNonce), &dwBytesRead, NULL);
	}
	else
	{
		// generate number to used once (unique for each file)
		randombytes_buf(pNonce, sizeof(pNonce));
		::WriteFile(hTargetFile, pNonce, sizeof(pNonce), &dwBytesWritten, NULL);
	}
	
	while (::ReadFile(hSourceFile, pData, CHUNK_SIZE, &dwBytesRead, NULL) && dwBytesRead > 0)
	{
		crypto_stream_xor(pData, pData, dwBytesRead, pNonce, pKey); // inplace
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

