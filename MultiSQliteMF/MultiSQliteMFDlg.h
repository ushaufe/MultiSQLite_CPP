
// MultiSQliteMFDlg.h : header file
//

// This is a C++ project to demonstrage how well SQLite can handle Multithreading
// The documentation for this project and the accompanying C# projects can be found here:
// (the location may be moved by the webmaster of the wiki)
// https://mywiki.grp.haufemg.com/pages/viewpage.action?pageId=156088657

#pragma once
#include "sqlite/sqlite3.h"

class CMyObject : public CObject {

};


// CMultiSQliteMFDlg dialog
class CMultiSQliteMFDlg : public CDialogEx
{
// Construction
public:
	CMultiSQliteMFDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINDIALOG };
#endif

	protected:
	// A handle to the ListBox must be kept static
	// so that the different threads have access to the GUI
	// This must be defined static so that the threads can access it outside of 
	// the instance-contect
	static CListBox* lb;
	
	
	// This database-object is used for access without connection-pooling (multiple threads / one connection)
	static sqlite3 *db;

	// Different instances of the the database are used
	// For simultaneous multithreading with multiple connection-objects (=Pooling)
	// see => OnBnClickedMulticonnectWrite()
	static sqlite3 *db0;
	static sqlite3 *db1;
	static sqlite3 *db2;
	// The instances must be static to be accessible
	// from the threads and hence outside of the 
	// context of the class
	

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
	// Each thread has an own function
	static void t1();
	static void t2();
	
	// A Window-handle must be kept 
	// so that the different threads have access to the GUI
	// This must be defined static so that the threads can access it outside of 
	// the instance-contect
	static CWnd* staticWnd;

	CMyObject* pNewThreadObject;

	static UINT ThreadProc1(LPVOID pParam);
	static UINT ThreadProc2(LPVOID pParam);

	static UINT ThreadSQLHammerIn1(LPVOID pParam);
	static UINT ThreadSQLHammerIn2(LPVOID pParam);

	static UINT ThreadSQLFlicker1(LPVOID pParam);
	static UINT ThreadSQLFlicker2(LPVOID pParam);

	static UINT ThreadSQLMultiHammerIn1(LPVOID pParam);
	static UINT ThreadSQLMultiHammerIn2(LPVOID pParam);

// Implementation
protected:
	HICON m_hIcon;
	
	// This method is called automatically after a "select from" database operation
	static int callback(void *NotUsed, int argc, char **argv, char **azColName);

	// This method is called automatically after a "select from" database operation
	// It fills the listbox-object and is called simultaneously from multiple threads
	static int callback_flicker(void *NotUsed, int argc, char **argv, char **azColName);
	
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
	void Connect();
	boolean execQuery(CString strQuery);
	boolean setAppID();
	CString strDatabaseFile;
	CString strAppID;

	const CString DB_NAME = L"Demo_CPP.db";

public:
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnLbnSelchangeList();
	afx_msg void OnBnClickedMultithread();
	afx_msg void OnBnClickedMfcMultithread();
	afx_msg void OnBnClickedConnect4sqlite();
	afx_msg void OnBnClickedHammerinsql();
	afx_msg void OnBnClickedShowdbcontents();
	afx_msg void OnBnClickedShowDbCountSingle();
	afx_msg void OnBnClickedHammer2sql();
	//afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedMulticonnectWrite();
	afx_msg void OnBnClickedSqlmultihammer();
	afx_msg void OnBnClickedMulticountOnce();
	afx_msg void OnBnClickedFlickercount();
};




