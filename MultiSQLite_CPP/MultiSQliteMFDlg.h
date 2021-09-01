
// MultiSQliteMFDlg.h : header file
//

// This is a C++ project to demonstrage how well SQLite can handle Multithreading
// The documentation for this project and the accompanying C# projects can be found here:
// (the location may be moved by the webmaster of the wiki)
// https://mywiki.grp.haufemg.com/pages/viewpage.action?pageId=156088657

#pragma once
#include "sqlite/sqlite3.h"
#include "VersionInfo.h"


class CFlickerObject : public CObject {

};

class CAboutDlg;

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

	static bool bFlickerLock1;
	static bool bFlickerLock2;

	static bool bPollock;

	static long maxThreadID;

	const int DB_LOCK_INTERVAL = 100;

	const int DB_VERSION_MIN = 1000;
	const int DB_VERSION_MAX = 2000;
	const CString DB_VERSION_STR = L"2.0.0.0";

	CMenu *menu;

	protected:
	// A handle to the ListBox must be kept static
	// so that the different threads have access to the GUI
	// This must be defined static so that the threads can access it outside of 
	// the instance-contect
	static CListBox* lb;
	
	
	// This database-object is used for access without connection-pooling (multiple threads / one connection)
	static sqlite3 *dbx;
	CMutex mutexDB;

	// Different instances of the the database are used
	// For simultaneous multithreading with multiple connection-objects (=Pooling)
	// see => OnBnClickedMulticonnectWrite()
	static sqlite3 *db0;
	static sqlite3 *db1;
	static sqlite3 *db2;
	// The instances must be static to be accessible
	// from the threads and hence outside of the 
	// context of the class
	
	CAboutDlg* aboutDlg;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
	
	// A Window-handle must be kept 
	// so that the different threads have access to the GUI
	// This must be defined static so that the threads can access it outside of 
	// the instance-contect
	static CWnd* staticWnd;
	bool bIsDebug = false;
	CFlickerObject* pFlickerObject;
	int nNumberThreads;

	static CString CMultiSQliteMFDlg::RandomString(int len, int iType);


	static UINT ThreadSQLHammerIn(LPVOID pParam);	

	static UINT ThreadSQLFlicker1(LPVOID pParam);
	static UINT ThreadSQLFlicker2(LPVOID pParam);

	static UINT ThreadSQLMultiHammerIn1(LPVOID pParam);
	static UINT ThreadSQLMultiHammerIn2(LPVOID pParam);

// Implementation
protected:
	HICON m_hIcon;
	
	void ShowNumberOfThreads();

	// This method is called automatically after a "select from" database operation
	static int callback(void *NotUsed, int argc, char **argv, char **azColName);


	static int UpdateApplications(void* NotUsed, int argc, char** argv, char** azColName);

	// This method is called automatically after a "select from" database operation
	// It fills the listbox-object and is called simultaneously from multiple threads
	static int callback_flicker(void *NotUsed, int argc, char **argv, char **azColName);

	bool UpdateApp();
	CString getAppData();
	bool PeekAndPump();
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
	void Connect();
	void Disconnect();
	boolean execQuery(CString strQuery);
	void UpdateMenuItems();
	boolean setAppID();
	bool getSQLInt(CString strSQL,int &nInt);
	bool getDBVersion(CString& strDBVersion);
	int getDBVersionNumber(CString strDBVersion);
	CString getDBVersionString(int nDBVersion);
	int getDBVersionNumber();
	CString strDatabaseFile;
	static CString strAppID;
	UINT_PTR tiUpdateApps = 1;
	UINT_PTR tiRestartAfterUpdate = 2;
	UINT_PTR tiCloseAfterUpdate = 3;
	UINT_PTR tiStartupDelay = 4;

	const CString DB_NAME = L"MultiSQLite.db";
	const CString APP_NAME = L"Haufe Multi - SQlite for C++";
	CString APP_DB_NAME = L"MultiSQlite_CPP";
	std::string ProcessIdToName(DWORD processId);
	const int DB_REVISION = 2;

public:
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnLbnSelchangeList();		
	
	afx_msg void OnBnClickedHammerinsql();
	afx_msg void OnBnClickedShowdbcontents();
	afx_msg void OnBnClickedShowDbCountSingle();
	afx_msg void OnBnClickedHammer2sql();
	//afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedMulticonnectWrite();
	afx_msg void OnBnClickedSqlmultihammer();
	afx_msg void OnBnClickedMulticountOnce();
	afx_msg void OnBnClickedSingleinsert();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBnClickedStartthreadssinglecon();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedFlickerCount();
	afx_msg void OnBnClickedUpdate();
	afx_msg void OnFileExit();	
	afx_msg void OnFileDisconnect();	
	afx_msg void OnUpdateWindowNew(CCmdUI* pCmdUI);
	afx_msg void OnHelpAbout();
	afx_msg void OnHelpUpdate();
	afx_msg void OnActionsStartthreads();
	afx_msg void OnActionsStopthreads();
	afx_msg void OnFileConnect();
	afx_msg void OnHelpShowgithubprojectforc();
	afx_msg void OnHelpShowgithubprojectforcs();
	afx_msg void OnActionsStartmultisqliteforCs();
};




