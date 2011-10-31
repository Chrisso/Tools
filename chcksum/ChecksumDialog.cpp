#include "StdAfx.h"
#include "ChecksumDialog.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <md5.h>
#include <sha.h>
#include <ripemd.h>
#include <whrlpool.h>

///////////////////////////////////////////////////////////////////////////////
// Construction/ Destruction
///////////////////////////////////////////////////////////////////////////////

CChecksumDialog::CChecksumDialog() : m_nAlgorithmId(0)
{
	m_bClosing = false;
	m_bCentered = false;
}

CChecksumDialog::~CChecksumDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
// Method Implementation
///////////////////////////////////////////////////////////////////////////////

DWORD CChecksumDialog::CalculateChecksumProc(CChecksumDialog *pThis)
{
	CryptoPP::HashTransformation *pHash = NULL;

	switch (pThis->m_nAlgorithmId)
	{
		case 0: pHash = new CryptoPP::Weak::MD5();	break;
		case 1: pHash = new CryptoPP::SHA1();		break;
		case 2: pHash = new CryptoPP::SHA256();		break;
		case 3: pHash = new CryptoPP::RIPEMD160();	break;
		case 4: pHash = new CryptoPP::Whirlpool();	break;
	}

	if (!pHash)
	{
		pThis->PostMessage(WMU_ASYNCPROGRESS_INFO, ProgressInfo_EndError);
		return 1;
	}

	HANDLE hFile = ::CreateFile(pThis->m_sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		pThis->PostMessage(WMU_ASYNCPROGRESS_INFO, ProgressInfo_Inititialize);

		BYTE pBuffer[1024];
		DWORD dwBytesRead = 0;

		LARGE_INTEGER nTemp; ::GetFileSizeEx(hFile, &nTemp);
		__int64 nFileSize = nTemp.QuadPart; 
		__int64 nTotalRead = 0;
		__int64 nLastPercentage = 0;

		while (::ReadFile(hFile, pBuffer, 1024, &dwBytesRead, NULL) && dwBytesRead && !pThis->m_bClosing)
		{
			pHash->Update(pBuffer, dwBytesRead);
			nTotalRead += dwBytesRead;

			if (nLastPercentage != (nTotalRead * 100) / nFileSize)
			{
				nLastPercentage = (nTotalRead * 100) / nFileSize;
				pThis->PostMessage(WMU_ASYNCPROGRESS_INFO, ProgressInfo_Percentage, max(0, min(100, (int)nLastPercentage)));
			}
		}

		::CloseHandle(hFile);

		if (!pThis->m_bClosing)	// user cancel?
		{
			pHash->Final(pBuffer);	// HACK: Digest may not be longer than 1024
			pThis->m_sChecksum = _T("");
			for (unsigned int i=0; i<pHash->DigestSize(); i++)
				pThis->m_sChecksum.AppendFormat(_T("%02x"), pBuffer[i]);

			pThis->PostMessage(WMU_ASYNCPROGRESS_INFO, ProgressInfo_EndSuccess);
		}
	}
	else pThis->PostMessage(WMU_ASYNCPROGRESS_INFO, ProgressInfo_EndError);

	delete pHash;
	return 0;
}

void CChecksumDialog::OnSelectFile(UINT uCode, int nID, HWND hWndCtl)
{
	CFileDialog fd(TRUE);
	if (fd.DoModal() == IDOK)
	{
		m_sFileName = fd.m_szFileName;
		m_sChecksum = _T("");
		DoDataExchange(FALSE);
	}
}
	
void CChecksumDialog::OnCalculateChecksum(UINT uCode, int nID, HWND hWndCtl)
{
	DoDataExchange(TRUE);

	if (m_sFileName.IsEmpty() || !::PathFileExists(m_sFileName) || ::PathIsDirectory(m_sFileName))
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_NOFILE);
		return ;
	}

	m_nAlgorithmId = CComboBox(GetDlgItem(IDC_CBX_ALGORITHM)).GetCurSel();
	m_hAsyncOp = ::AtlCreateThread<CChecksumDialog>(CalculateChecksumProc, this);
}

void CChecksumDialog::OnCopyChecksum(UINT uCode, int nID, HWND hWndCtl)
{
	if (m_sChecksum.IsEmpty())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_NOCHECKSUM);
		return;
	}

	if (!OpenClipboard())
	{
		::AtlMessageBox(m_hWnd, IDS_ERR_WINDOWS);
		return ;
	}

	EmptyClipboard();

	size_t cbStr = (m_sChecksum.GetLength() + 1) * sizeof(TCHAR);
	HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, cbStr);
	memcpy_s(GlobalLock(hData), cbStr, m_sChecksum.LockBuffer(), cbStr);
	GlobalUnlock(hData);
	m_sChecksum.UnlockBuffer();

	UINT uiFormat = (sizeof(TCHAR) == sizeof(WCHAR)) ? CF_UNICODETEXT : CF_TEXT;
	if (SetClipboardData(uiFormat, hData))
		::AtlMessageBox(m_hWnd, IDS_CHECKSUMCOPIED, IDS_APP_TITLE);

	CloseClipboard();
}

///////////////////////////////////////////////////////////////////////////////
// Windows Message Handling
///////////////////////////////////////////////////////////////////////////////

LRESULT CChecksumDialog::OnInitDialog(HWND hwndFocus, LPARAM lParam) 
{
	CComboBox cbx(GetDlgItem(IDC_CBX_ALGORITHM));
	if (cbx.IsWindow())
	{
		cbx.AddString(CString((LPCTSTR)IDS_ALGORITHM_MD5));
		cbx.AddString(CString((LPCTSTR)IDS_ALGORITHM_SHA1));
		cbx.AddString(CString((LPCTSTR)IDS_ALGORITHM_SHA256));
		cbx.AddString(CString((LPCTSTR)IDS_ALGORITHM_RIPEMD160));
		cbx.AddString(CString((LPCTSTR)IDS_ALGORITHM_WHIRLPOOL));
		cbx.SetCurSel(0);
	}

#ifdef UNICODE
	int nArgs;
	LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);;
	if (szArglist)
	{
		if (nArgs == 2)
		{
			m_sFileName = szArglist[1];
			DoDataExchange(FALSE);
			OnCalculateChecksum(0, 0, NULL);
		}
		::LocalFree(szArglist);
	}	
#endif

	return 0;
}

void CChecksumDialog::OnShowWindow(BOOL bShowing, int nReason)
{
	if (bShowing && !m_bCentered)
	{
		CenterWindow();
		m_bCentered = true;
	}
}

void CChecksumDialog::OnDropFiles(HDROP hDropInfo)
{
	if (DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0) == 1)
	{
		TCHAR szFile[MAX_PATH+1];
		if (DragQueryFile(hDropInfo, 0, szFile, MAX_PATH))
		{
			m_sFileName = szFile;
			m_sChecksum = _T("");
			DoDataExchange(FALSE);
		}
	}
}

void CChecksumDialog::OnClose()
{
	if (m_hAsyncOp)
	{
		ATLTRACE(_T("[CChecksumDialog] ::OnClose - waiting for pending calculation...\n"));
		
		m_bClosing = true;	// fire cancellation signal
		if (::WaitForSingleObject(m_hAsyncOp, 1000) == WAIT_OBJECT_0)
		{
			ATLTRACE(_T("[CChecksumDialog] ::OnClose - Calculation cancelled.\n"));
		}
	}

	EndDialog(IDOK);
}

LRESULT CChecksumDialog::OnAsyncProgress(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ProgressInfo info = static_cast<ProgressInfo>(wParam);

	switch (info)
	{
		case ProgressInfo_Inititialize:
		{
			ATLTRACE(_T("[CChecksumDialog] ::OnAsyncProgress - Calculation started...\n"));
			GetDlgItem(IDC_BTN_SELECTFILE).EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_CALCULATECHECKSUM).EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_COPYCHECKSUM).EnableWindow(FALSE);
			GetDlgItem(IDC_PGS_CALCULATION).ShowWindow(SW_SHOWNORMAL);
		} break;

		case ProgressInfo_Percentage:
		{
			GetDlgItem(IDC_PGS_CALCULATION).SendMessage(PBM_SETPOS, lParam);
		} break;

		case ProgressInfo_EndError:
		{
			ATLTRACE(_T("[CChecksumDialog] ::OnAsyncProgress - Calculation finished with error!\n"));
			GetDlgItem(IDC_PGS_CALCULATION).ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_SELECTFILE).EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_CALCULATECHECKSUM).EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_COPYCHECKSUM).EnableWindow(TRUE);

			m_hAsyncOp = NULL;
			m_sChecksum = _T("");
			DoDataExchange(FALSE);

			::AtlMessageBox(m_hWnd, IDS_ERR_WINDOWS);
		} break;

		case ProgressInfo_EndSuccess:
		{
			ATLTRACE(_T("[CChecksumDialog] ::OnAsyncProgress - Calculation finished.\n"));
			GetDlgItem(IDC_PGS_CALCULATION).ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_SELECTFILE).EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_CALCULATECHECKSUM).EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_COPYCHECKSUM).EnableWindow(TRUE);

			m_hAsyncOp = NULL;
			DoDataExchange(FALSE);
		} break;
	}

	return 0;
}
