#pragma once

#include "Resource.h"

class CChecksumDialog : public CDialogImpl<CChecksumDialog>, 
						public CWinDataExchange<CChecksumDialog>
{
private:
	bool    m_bCentered;
	int     m_nAlgorithmId;
	CString m_sFileName;
	CString m_sChecksum;

	volatile bool   m_bClosing;
	volatile HANDLE m_hAsyncOp;

	static DWORD WINAPI CalculateChecksumProc(CChecksumDialog *pThis);

protected:
	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam);
	LRESULT OnAsyncProgress(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnShowWindow(BOOL bShowing, int nReason);
	void OnDropFiles(HDROP hDropInfo);
	void OnSelectFile(UINT uCode, int nID, HWND hWndCtl);
	void OnCalculateChecksum(UINT uCode, int nID, HWND hWndCtl);
	void OnCopyChecksum(UINT uCode, int nID, HWND hWndCtl);
	void OnClose();

public:
	CChecksumDialog();
	~CChecksumDialog();

	enum { IDD = IDD_CHECKSUM };

	BEGIN_MSG_MAP(CStartupDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_DROPFILES(OnDropFiles)
		COMMAND_ID_HANDLER_EX(IDC_BTN_SELECTFILE, OnSelectFile)
		COMMAND_ID_HANDLER_EX(IDC_BTN_CALCULATECHECKSUM, OnCalculateChecksum)	
		COMMAND_ID_HANDLER_EX(IDC_BTN_COPYCHECKSUM, OnCopyChecksum)
		MESSAGE_HANDLER_EX(WMU_ASYNCPROGRESS_INFO, OnAsyncProgress)
		MSG_WM_CLOSE(OnClose)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CChecksumDialog)
		DDX_TEXT(IDC_EDT_FILENAME, m_sFileName)
		DDX_TEXT(IDC_EDT_CHECKSUM, m_sChecksum)
	END_DDX_MAP()
};
