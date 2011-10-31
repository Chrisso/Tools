// LineCountGui.h : Schnittstelle der Fensterklasse für diese Anwendung.
// 04/2005 Christoph Stoepel (christoph-stoepel@web.de)

#pragma once

class CLineCountGui : public CWindowImpl<CLineCountGui, CWindow, CFrameWinTraits>
{
private:
	HWND m_wndStatusBar;
	HWND m_wndTreeView;

	int  ProcessFile(TCHAR *name);
	int  ProcessDirectory(TCHAR *name, HTREEITEM pTI = NULL);
	bool CheckFileExtension(TCHAR *name) const;
	BOOL UpdateItemText(TCHAR *name, int count, HTREEITEM item);
	HTREEITEM AddItemToTree(TCHAR *name, int count, HTREEITEM parent);

protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnLineCount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:

	CLineCountGui();

    DECLARE_WND_CLASS(_T("LineCountGui"))

	BEGIN_MSG_MAP(CLineCountGui)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)		
		COMMAND_ID_HANDLER(IDM_SELECTFOLDER, OnLineCount)
		COMMAND_ID_HANDLER(IDM_EXIT, OnExit)
		COMMAND_ID_HANDLER(IDM_ABOUT, OnAbout)
	END_MSG_MAP()
};
