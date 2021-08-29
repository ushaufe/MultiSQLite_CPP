
// MultiSQliteMFDlg.cpp : implementation file
//

#include "pch.h"
#include <afxinet.h>
#include "framework.h"
#include "MultiSQliteMF.h"
#include "MultiSQliteMFDlg.h"
#include "afxdialogex.h"
#include <thread>
#include <iostream>
#include <fileapi.h>
#include <atlconv.h> // for CT2A
#include "resource.h"		// main sy
//#include "GlobalFunctions.h"
// Connection pooling
// https://dev.yorhel.nl/doc/sqlaccess


using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The following static elements of the class must be redefined globally
// Since they are static elements of the class and need redefinition
CWnd* CMultiSQliteMFDlg::staticWnd;
CString CMultiSQliteMFDlg::strAppID;

bool CMultiSQliteMFDlg::bFlickerLock1;
bool CMultiSQliteMFDlg::bFlickerLock2;

CListBox* CMultiSQliteMFDlg::lb;
sqlite3* CMultiSQliteMFDlg::dbx;
sqlite3* CMultiSQliteMFDlg::db0;
sqlite3* CMultiSQliteMFDlg::db1;
sqlite3* CMultiSQliteMFDlg::db2;

long CMultiSQliteMFDlg::maxThreadID;

bool CMultiSQliteMFDlg::bPollock;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBtnClickedFlickerCount();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMultiSQliteMFDlg dialog



CMultiSQliteMFDlg::CMultiSQliteMFDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAINDIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// The static listbox in order to access the GUI-element 
	// from threads, which do not have access to an instance of the class
	lb = NULL;

	// The constructor sets all instances of connections to NULL
	// so that it can be checked against NULL if a connection already exists
	dbx = NULL;  
	db0 = NULL;
	db1 = NULL;
	db2 = NULL;

	maxThreadID = 0;

	bFlickerLock1 = false;
	bFlickerLock2 = false;

	bPollock = false;

#if defined _DEBUG
	bIsDebug = true;
#else
	bIsDebug = false;
#endif

}

void CMultiSQliteMFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CMultiSQliteMFDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CMultiSQliteMFDlg::OnBnClickedBtnConnect)
	ON_LBN_SELCHANGE(IDC_LIST, &CMultiSQliteMFDlg::OnLbnSelchangeList)	
	ON_BN_CLICKED(BTN_HammerInSQL, &CMultiSQliteMFDlg::OnBnClickedHammerinsql)
	ON_BN_CLICKED(BTN_ShowDBContents, &CMultiSQliteMFDlg::OnBnClickedShowdbcontents)
	ON_BN_CLICKED(IDC_SHOW_DB_COUNT_SINGLE, &CMultiSQliteMFDlg::OnBnClickedShowDbCountSingle)
	ON_BN_CLICKED(BTN_Hammer2SQL, &CMultiSQliteMFDlg::OnBnClickedHammer2sql)
	ON_BN_CLICKED(BTN_MultiConnect_Write, &CMultiSQliteMFDlg::OnBnClickedMulticonnectWrite)
	ON_BN_CLICKED(BTN_SQLMultiHammer, &CMultiSQliteMFDlg::OnBnClickedSqlmultihammer)
	ON_BN_CLICKED(BTN_MultiCount_Once, &CMultiSQliteMFDlg::OnBnClickedMulticountOnce)
	ON_BN_CLICKED(BTN_SingleInsert, &CMultiSQliteMFDlg::OnBnClickedSingleinsert)	
	ON_WM_TIMER()	
	ON_WM_CLOSE()
	ON_BN_CLICKED(BTN_StartThreadsSingleCon, &CMultiSQliteMFDlg::OnBnClickedStartthreadssinglecon)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(BTN_FLICKER_COUNT, &CMultiSQliteMFDlg::OnBnClickedFlickerCount)
	ON_BN_CLICKED(BTN_Update, &CMultiSQliteMFDlg::OnBnClickedUpdate)
END_MESSAGE_MAP()


// CMultiSQliteMFDlg message handlers

BOOL CMultiSQliteMFDlg::OnInitDialog()
{
	//CFont font;
	//font.CreatePointFont(16, _T("Arial"));
	
	CDialogEx::OnInitDialog();
	CScrollBar* scrollBarNumberThreads = (CScrollBar*)GetDlgItem(IDC_SLIDER_NUMBER_THREADS);
	staticWnd = this;
	nNumberThreads = 1;
	scrollBarNumberThreads->SetScrollRange(1, 20, true);
	scrollBarNumberThreads->SetScrollPos(nNumberThreads);
	ShowNumberOfThreads();
	

	// https://www.geeksforgeeks.org/multithreading-in-cpp/

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CPaintDC   dc(this);
	dc.SetBkColor(RGB(255, 255, 255));

	SetWindowText(APP_NAME);
	

	
	CFont font;
	font.CreateFont(
		40,                        // nHeight
		40,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		TRUE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Microsoft Sans Serif"));                 // lpszFacename

	
	// https://forums.codeguru.com/showthread.php?63545-Static-text-font-size

	CRect rc;
	GetWindowRect(&rc); // getting dialog coordinates
	MoveWindow(rc.left, rc.top, 1200, 800);

	lb = (CListBox*)GetDlgItem(IDC_LIST);
	CString strVersionPrefix = bIsDebug ? L" (Debug)" : L" (Release)";
	CString strVersion = CString("Version: ") + GetAppVersion(GetAppPath()) + strVersionPrefix;
	lb->AddString(CString( APP_NAME ));
	lb->AddString(strVersion);
	CStatic* lblVersion =  (CStatic*)GetDlgItem(IDC_VERSION);	
	lblVersion->SetWindowText( strVersion);	
	
	/*
	DWORD processID = GetCurrentProcessId();
	const char* szProcessName = (ProcessIdToName(processID).c_str());
	CString strProcessName(szProcessName);
	APP_DB_NAME = strProcessName;
	MessageBox(APP_DB_NAME);
	*/

	SetTimer(tiStartupDelay, 2000, NULL); // one event every 1000 ms = 1 s	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMultiSQliteMFDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMultiSQliteMFDlg::OnPaint()
{
	
	CPaintDC   dc(this);
	CRect   rect;
	
	GetClientRect(&rect);
	CDC   dcMem;
	dcMem.CreateCompatibleDC(&dc);
	
	
	CBitmap   bmpBackground;
	bmpBackground.LoadBitmap(IDB_BACKGROUND2);
	// IDB_BITMAP corresponding ID is your own map   
	BITMAP   bitmap;
	bmpBackground.GetBitmap(&bitmap);
	CBitmap* pbmpOld = dcMem.SelectObject(&bmpBackground);
	dc.FillRect(rect, new CBrush());
	dc.StretchBlt(0, 100, rect.Width(), rect.Height()-100, &dcMem, 0, 0,
		bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	
	dc.SetBkColor(RGB(255, 255, 255));

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}


}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMultiSQliteMFDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


boolean CMultiSQliteMFDlg::execQuery(CString strQuery)
{
	CStringA strQueryA(strQuery);
	const char* szQuery = strQueryA;


	// The following cases handle access via multiple applications
	// (Separate EXEs or separate DLLs)
	// Only one application can use a SQlite-database exclusively for wriging
	// Whereas multiple applications can use it for reading 
	// even when locked from one app for wriging
	int rc;

	char* zErrMsg = 0;
	// our CSingleLock object. 
	CSingleLock dbLock(&mutexDB);
	dbLock.Lock(DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		rc = sqlite3_exec(dbx, szQuery, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			sqlite3_free(zErrMsg);
			return false;
		}
		return true;
	}
	dbLock.Unlock();
}


int CMultiSQliteMFDlg::getDBVersionNumber(CString strDBVersion)
{
	strDBVersion = strDBVersion.Trim();	
	strDBVersion.Remove('.');

	CT2A ascDBVersion(strDBVersion);
	char* p;
	int nDBVersion = strtol(ascDBVersion.m_psz, &p, 10);
	if (*p != 0) {
		return 0;
	}

	return nDBVersion;

}

CString CMultiSQliteMFDlg::getAppData()
{
	char* szAppData;
	size_t lenAppData;
	errno_t err = _dupenv_s(&szAppData, &lenAppData, "APPDATA");

	CString strAppDataHaufe(szAppData);

	if (strAppDataHaufe.GetLength() > 0) {
		if (strAppDataHaufe[strAppDataHaufe.GetLength() - 1] != '\\')
			strAppDataHaufe += '\\';
	}

	strAppDataHaufe += CString("Haufe") + CString("\\");

	CStringW strwAppDataHaufe(strAppDataHaufe);
	LPCWSTR ptrStrAppDataHaufe = strwAppDataHaufe;
	CreateDirectory(ptrStrAppDataHaufe, NULL);

	CString strAppData = strAppDataHaufe + CString("MultiSQLite") + CString("\\");
	CStringW strwAppData(strAppData);
	LPCWSTR ptrStrAppData = strwAppData;
	CreateDirectory(ptrStrAppData, NULL);

	return strAppData;
}

int CMultiSQliteMFDlg::getDBVersionNumber()
{
	CString strDBVersion;
	if (!getDBVersion(strDBVersion))
		return -1;

	return getDBVersionNumber(strDBVersion);
}

void CMultiSQliteMFDlg::Connect()
{		
	// This connection is not to be used by threads
	// It should simply simulate the easiest case (simple access to SQlite)
	// In order to rule out errors in the more complex cases
	
	if (dbx != NULL) {
		this->lb->AddString(L"Error: Already Connected!");
		return;
	}


		
	strDatabaseFile = getAppData() + DB_NAME;
	//const char* szDatabaseFile = (LPCTSTR)strDatabaseFile;
	
	//char* c = strDatabaseFile.GetBuffer(strDatabaseFile.GetLength());
	CStringA strDatabaseFileA(strDatabaseFile);
	const char* szDatabaseFile = strDatabaseFileA;
	
	
		// The following cases handle access via multiple applications
	// (Separate EXEs or separate DLLs)
	// Only one application can use a SQlite-database exclusively for wriging
	// Whereas multiple applications can use it for reading 
	// even when locked from one app for wriging
	int rc;

	/*
	// hence if a file already exists
	if (PathFileExists(strDatabaseFile)) {
		// only reading is possible
		rc = sqlite3_open_v2(szDatabaseFile, &db, SQLITE_OPEN_READONLY, NULL);
		this->lb->AddString(L"Mode: Read-Only");
	}
	else
	{
		// otherwise a second instance of the same application can be opened to read it
		rc = sqlite3_open_v2(szDatabaseFile, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		this->lb->AddString(L"Read + Write Access");
	}
	//DeleteFile(L"demo.db");
	*/

	bool bDatabaseExisted = PathFileExists(strDatabaseFile);
	if (!bDatabaseExisted)
	{
		this->lb->AddString(L"No database has been created yet.");
		this->lb->AddString(L"Create database.....");
	}

	rc = sqlite3_open_v2(szDatabaseFile, &dbx, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	this->lb->AddString(L"Read + Write Access");

	const int STATEMENTS = 8;
	char* zErrMsg = 0;
	//const char* pSQL[STATEMENTS];
	


	if (rc != SQLITE_OK)
	{
		dbx = NULL;
		this->lb->AddString(L"Error: Cant connect!");
	}
	else
	{
		if (!bDatabaseExisted)
		{
			this->lb->AddString(L"Database created.");
		}
		this->lb->AddString(L"Connected");
		this->lb->AddString(L"Database: " + strDatabaseFile);
	}


	bool bRecreateDB = false;
	int nDBVersionNumber = getDBVersionNumber();
	CString strDBVersion;
	getDBVersion(strDBVersion);
	if (bDatabaseExisted)
	{
		if (nDBVersionNumber <= 0)
		{
			this->lb->AddString(L"Error: Invalid database....");
			bRecreateDB = true;
		}
		else
		{
			this->lb->AddString(L"Database Version: " + strDBVersion);
		}
		if (nDBVersionNumber < DB_VERSION_MIN)
		{
			this->lb->AddString(L"Error: Old database....");
			bRecreateDB = true;
		}
		if (nDBVersionNumber > DB_VERSION_MAX)
		{
			this->lb->AddString(L"Error: The database file is newer than the application.");
			this->lb->AddString(L"       Please update your application or rename the database file.");
			this->lb->AddString(L"       -----------------------------------------------------------.");
			this->lb->AddString(L"       The file is located here:");
			this->lb->AddString(L"       -----------------------------------------------------------.");
			this->lb->AddString(L"       " + strDatabaseFile);
			return;
		}


		if (bRecreateDB)
		{
			this->lb->AddString(L"       Recreating database....");
			this->lb->AddString(L"       Disconnect....");
			sqlite3_close(dbx);

			{
				this->lb->AddString(L"       Deleting Database....");
				if (!DeleteFile(strDatabaseFile))
				{
					this->lb->AddString(L"Error: Old Database could not have been deleted.");
					dbx = NULL;
					return;
				}
			}
			this->lb->AddString(L"       Recreate database and connect....");
			rc = sqlite3_open_v2(szDatabaseFile, &dbx, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
			if (rc != SQLITE_OK)
			{
				dbx = NULL;
				this->lb->AddString(L"Error: Cant connect!");
				return;
			}
			else
			{
				this->lb->AddString(L"Connected");
				this->lb->AddString(L"Database: " + strDatabaseFile);
			}
		}

		if (dbx == NULL) {
			this->lb->AddString(L"Error: Database is not working");
			return;
		}
	}

	if (bDatabaseExisted)
	{		
		if (nDBVersionNumber < getDBVersionNumber(DB_VERSION_STR))
		{
			this->lb->AddString(L"Updating database....");
			this->lb->AddString(L"       Current version is: " + this->getDBVersionString(nDBVersionNumber));
			this->lb->AddString(L"       Update to: " + DB_VERSION_STR);
		}
	}
	
	if (nDBVersionNumber < 1000)
	{
		execQuery(CString("Create Table if NOT Exists version (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT) "));
		execQuery(CString("Create Table if NOT Exists testtable (id INTEGER PRIMARY KEY AUTOINCREMENT, text VARCHAR, threadID INTEGER, appID INTEGER, tsCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP) "));
		execQuery(CString("Create Table if NOT Exists apps(id INTEGER PRIMARY KEY AUTOINCREMENT, tsCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP, tsLastPoll TIMESTAMP DEFAULT CURRENT_TIMESTAMP, name TEXT, isActive INTEGER DEFAULT FALSE)"));
		execQuery(CString("Create Table if NOT Exists threads(id INTEGER PRIMARY KEY AUTOINCREMENT, threadID INTEGER, appID INTEGER, tsCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP, isActive INTEGER DEFAULT FALSE)"));
		execQuery(CString("update apps set isActive=0 where tsLastPoll is null "));

		//execQuery(CString("insert into apps (name, isActive) values ('") + APP_DB_NAME + CString("', true)"));
		nDBVersionNumber = 1000;
	}

	if (nDBVersionNumber < 1100)
	{
		execQuery(CString("Drop table if exists version"));
		execQuery(CString("Drop table if exists multisqlite_version"));
		execQuery(CString("Create Table if NOT Exists multisqlite_version (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, version TEXT) "));
		execQuery(CString("insert into multisqlite_version (id,name,version) values (0,'MULTISQLITE', '") + CString ("1.1.0.0")  + CString("')"));
		nDBVersionNumber = 1100;
	}

	if (nDBVersionNumber < 2000)
	{
		{
			execQuery(CString("Alter Table testtable RENAME TO multisqlite_entries "));
			execQuery(CString("Alter Table apps RENAME TO multisqlite_apps  "));
			execQuery(CString("Alter Table threads RENAME TO multisqlite_threads  "));
			execQuery(L"Update multisqlite_version set version='" + CString(DB_VERSION_STR) + CString("'"));			
			CString strCurrentDBVersion;
			nDBVersionNumber = getDBVersionNumber(DB_VERSION_STR);
		}
				
	}
	
	// Create a table that can hold text-data along with the thread-id of the thread that created the data
	/*
	CString strInsert;
	strInsert.Format(_T("insert into testtable (appID,threadid,text) values (%s,0,'%s')"), strAppID, CTime::GetCurrentTime().Format("%Y/%m/%d %H:%M")); // 
	execQuery(strInsert);
	execQuery(CString( "Delete from version") );
	execQuery(CString( "insert into version (id,name) values (0,") + _T("'1.0.0.0'") + CString(")"));
	
	*/

	//Create a table that can hold text-data along with the thread-id of the thread that created the data
	execQuery(L"update multisqlite_apps set tsLastPoll = CURRENT_TIMESTAMP where id=" + strAppID);
	execQuery(L"update multisqlite_apps set isActive=0 where tsLastPoll is null ");	
	execQuery(CString("insert into multisqlite_apps (name, isActive) values ('") + APP_DB_NAME + CString("', true)"));
	setAppID();
	CString strInsert;
	strInsert.Format(L"insert into multisqlite_entries (appID,threadid,text) values (%s,0,'%s')", strAppID, CTime::GetCurrentTime().Format("%Y/%m/%d %H:%M"));
	execQuery(strInsert);
	
	SetTimer(tiUpdateApps, 5000, NULL); // one event every 1000 ms = 1 s		
	
	CString appName = CString("MultiSQLite_CPP");
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.

	
}



CString CMultiSQliteMFDlg::getDBVersionString(int nDBVersion)
{
	int d = 0;
	int r = 0;

	CString str = _T("");

	while (nDBVersion > 0)
	{
		d = nDBVersion / 10;
		r = nDBVersion % 10;

		CString strR = _T("");

		if (str.GetLength() > 0)
			str = str + '.';
		strR.Format(_T("%d"), r);

		str += strR;
		nDBVersion = d;
	}
	str.MakeReverse();
	return str;
}


bool CMultiSQliteMFDlg::PeekAndPump()
{
	MSG msg;

	/*
	if (!bIsDebug)
		return false;
    */		

	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!AfxGetApp()->PumpMessage())
		{
			::PostQuitMessage(0);
			return false;			
		}
		//return true;		
		return false;
	}
}

bool  CMultiSQliteMFDlg::getDBVersion(CString& strDBVersion)
{
	// https://wang.yuxuan.org/blog/item/2007/05/simple-sqlite-test-c-program-without-callbacks
	//https://www.programmersought.com/article/22293267117/
	//https://stackoverflow.com/questions/14743061/sqlite3-exec-without-callback/26463522

	CString strSQL = _T("Select version from multisqlite_version order by id DESC LIMIT 1");
	CStringA strSQLA(strSQL);
	const char* szSQL = strSQLA;

	try
	{
		sqlite3_stmt* stmt = NULL;
		int rc = sqlite3_prepare_v2(dbx, szSQL, -1, &stmt, NULL);
		if (rc != SQLITE_OK)
			return false;

		int rowCount = 0;
		rc = sqlite3_step(stmt);

		while (rc != SQLITE_DONE && rc != SQLITE_OK)
		{
			rowCount++;
			int colCount = sqlite3_column_count(stmt);
			for (int colIndex = 0; colIndex < colCount; colIndex++)
			{
				int type = sqlite3_column_type(stmt, colIndex);
				const char* columnName = sqlite3_column_name(stmt, colIndex);
				if (type == SQLITE_TEXT)
				{
					strDBVersion = sqlite3_column_text(stmt, colIndex);
					rc = sqlite3_finalize(stmt);
					return true;
				}
				rc = sqlite3_finalize(stmt);
				return false;
			}
			rc = sqlite3_finalize(stmt);
			return false;
		}
		rc = sqlite3_finalize(stmt);
		return false;
	}
	catch (...)
	{
		return false;
	}
}



boolean CMultiSQliteMFDlg::setAppID() {
	// https://wang.yuxuan.org/blog/item/2007/05/simple-sqlite-test-c-program-without-callbacks
	//https://www.programmersought.com/article/22293267117/
	//https://stackoverflow.com/questions/14743061/sqlite3-exec-without-callback/26463522

	sqlite3_stmt* stmt = NULL;
	CSingleLock dbLock(&mutexDB);
	dbLock.Lock(DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		int rc = sqlite3_prepare_v2(dbx, "SELECT id FROM multisqlite_apps order by id DESC LIMIT 1", -1, &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			dbLock.Unlock();
			return rc;
		}

		int rowCount = 0;
		rc = sqlite3_step(stmt);
		while (rc != SQLITE_DONE && rc != SQLITE_OK)
		{
			rowCount++;
			int colCount = sqlite3_column_count(stmt);
			for (int colIndex = 0; colIndex < colCount; colIndex++)
			{
				int type = sqlite3_column_type(stmt, colIndex);
				const char* columnName = sqlite3_column_name(stmt, colIndex);
				if (type == SQLITE_INTEGER)
				{
					int valInt = sqlite3_column_int(stmt, colIndex);
					strAppID.Format(_T("%d"), valInt);
					SetWindowText(CString(_T("Haufe Multi-SQlite for C++ <ID: ")) + strAppID + CString(">"));
					rc = sqlite3_finalize(stmt);
					dbLock.Unlock();
					return true;
				}
				rc = sqlite3_finalize(stmt);
				dbLock.Unlock();
				return false;
			}
			rc = sqlite3_finalize(stmt);
			dbLock.Unlock();
			return false;
		}
		rc = sqlite3_finalize(stmt);
		dbLock.Unlock();
		return false;
	}
}


void CMultiSQliteMFDlg::OnBnClickedBtnConnect()
{			
	Connect();
}



// This callback function is used after
// the execution of "select from * " statements
// From different threads
int CMultiSQliteMFDlg::callback_flicker(void *NotUsed, int argc, char **argv, char **azColName)
{
	CListBox* list = (CListBox*)(staticWnd->GetDlgItem(IDC_LIST));
	list->ResetContent();
	int i;
	for (i = 0; i < argc; i++)
	{
		CString strCol(azColName[i]);
		CString strArgV(argv[i]);
		CString strArgC(" ");
		CString str = strCol + CString(" ") + strArgV + CString(" ") + strArgC;
		list->AddString(str);
		//cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
	}
	//bFlickerLock1 = false;
	//bFlickerLock2 = false;
	//cout << "\n";
	return 0;
}



int CMultiSQliteMFDlg::UpdateApplications(void* NotUsed, int argc, char** argv, char** azColName)
{
	CListBox* lbApplications = (CListBox*)(staticWnd->GetDlgItem(IDLB_APPLICATIONS));
	int i;
	CString strParam;

	CString str;	
	for (i = 0; i < argc; i++)
	{
		str = CString(argv[i]);
		lbApplications->AddString(str);
	}
	
	//cout << "\n";
	return 0;
}


int CMultiSQliteMFDlg::callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	CListBox* list = (CListBox*)(staticWnd->GetDlgItem(IDC_LIST));
	int i;
	for (i = 0; i < argc; i++)
	{
		CString strCol(azColName[i]);
		CString strArgV(argv[i]);
		CString strArgC(" ");
		CString str = strCol + CString(" ") + strArgV + CString(" ") + strArgC;
		list->AddString(str);
		//cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
	}
	//cout << "\n";
	list->SetCurSel(list->GetCount() - 1); // List box is zero based.
	return 0;
}


void CMultiSQliteMFDlg::OnLbnSelchangeList()
{
	// TODO: Add your control notification handler code here
}






void CMultiSQliteMFDlg::OnBnClickedHammerinsql()
{
	const int STATEMENTS = 8;
	char *zErrMsg = 0;
	const char *pSQL[STATEMENTS];
	staticWnd = this;
	int nNumberInserts = 100;	
	

	int rc;
	if (dbx == NULL) {
		this->lb->AddString(_T("DB NOT YET CONNECTED."));
		return;
	}
	
	CString strSQLInsert;
	for (int i = 0; i < nNumberInserts; i++) {
		strSQLInsert.Format(_T("insert into multisqlite_entries (threadid,text,appid) values (0,'T1: TID:0 / %d',%s)"), i, strAppID);
		CT2A szSQLInsert(strSQLInsert.GetString());
		CSingleLock dbLock(&mutexDB);
		dbLock.Lock(DB_LOCK_INTERVAL);    // Wait 100 ms...
		if (dbLock.IsLocked())
		rc = sqlite3_exec(dbx, szSQLInsert, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			return;
			dbLock.Unlock();
		}
		dbLock.Unlock();
	}
	
	CString strMessage;
	strMessage.Format(_T("%d Rows inserted into database"), nNumberInserts);
	this->lb->AddString(strMessage);
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
}


void CMultiSQliteMFDlg::OnBnClickedShowdbcontents()
{
	int rc;
	CSingleLock dbLock(&mutexDB);
	dbLock.Lock(DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		if (dbx == NULL)
		{
			this->lb->AddString(_T("DB NOT YET CONNECTED."));
			dbLock.Unlock();
			return;
		}
	}
	dbLock.Unlock();

	char *zErrMsg = 0;
	char* szSelect = "Select * from multisqlite_entries";
	
	dbLock.Lock(DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	rc = sqlite3_exec(dbx, szSelect, callback, 0, &zErrMsg);	
	if (rc != SQLITE_OK)
	{
		sqlite3_close(dbx);
		dbx = NULL;
		this->lb->AddString(_T("Couldn't read from database") + CString(zErrMsg));
		dbLock.Unlock();
		return;
	}
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
}


void CMultiSQliteMFDlg::OnBnClickedShowDbCountSingle()
{
	int rc;
	CSingleLock dbLock(&mutexDB);
	dbLock.Lock(DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		if (dbx == NULL) {
			this->lb->AddString(_T("DB NOT YET CONNECTED."));
			return;
		}
	}
	dbLock.Unlock();

	char *zErrMsg = 0;
	char* szSelect = "Select count(*) as Count_Thread_GUI from multisqlite_entries where threadID=0;Select count(*) as Count_Thread1 from multisqlite_entries where threadID=1;Select count(*) as Count_Thread2 from multisqlite_entries  where threadID=2";
	
	
	dbLock.Lock(DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		rc = sqlite3_exec(dbx, szSelect, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			sqlite3_close(dbx);
			dbx = NULL;
			this->lb->AddString(_T("Couldn't read from database") + CString(zErrMsg));
			dbLock.Unlock();
			return;
		}
	}
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
	dbLock.Unlock();
}


void CMultiSQliteMFDlg::OnBnClickedHammer2sql()
{
	lb = (CListBox*)GetDlgItem(IDC_LIST);

	pFlickerObject = new CFlickerObject();
	AfxBeginThread(ThreadSQLHammerIn, pFlickerObject);	
	AfxBeginThread(ThreadSQLHammerIn, pFlickerObject);
}

CString CMultiSQliteMFDlg::RandomString(int iLength, int iType)
{

	CString strReturn;

	CString strLocal;

	for (int i = 0; i < iLength; ++i)
	{
		int iNumber;

		// Seed the random-number generator with TickCount so that
		// the numbers will be different every time we run.
		srand((unsigned int)((i + 1) * iLength * GetTickCount()));

		switch (iType)
		{
		case 1:
			iNumber = rand() % 122;
			if (48 > iNumber)

				iNumber += 48;

			if ((57 < iNumber) &&
				(65 > iNumber))

				iNumber += 7;

			if ((90 < iNumber) &&
				(97 > iNumber))

				iNumber += 6;

			strReturn += (char)iNumber;

			break;

		case 2:

			iNumber = rand() % 122;

			if (65 > iNumber)

				iNumber = 65 + iNumber % 56;

			if ((90 < iNumber) &&
				(97 > iNumber))

				iNumber += 6;

			strReturn += (char)iNumber;

			break;

		case 3:

			strLocal.Format(L"%i", rand() % 9);

			strReturn += strLocal;

			break;

		default:

			strReturn += (char)rand();

			break;

		}

	}

	return strReturn;

}


UINT CMultiSQliteMFDlg::ThreadSQLHammerIn(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;

	int threadID = ++maxThreadID;	
	

	char* zErrMsg = 0;
	int rc;

	/*
	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
    */

	if (dbx == NULL) {
		return 1;
	}

	CString strMessage;
	strMessage.Format(_T("  Start Thread #%d"), threadID);
	((CMultiSQliteMFDlg*)staticWnd)->lb->AddString(strMessage);

	CString strStartThread;
	strStartThread.Format(_T("insert into multisqlite_threads (threadid,appID,isActive) values (%d,'%s',1)"), threadID, strAppID);
	CT2A szStartThread(strStartThread.GetString());
	CSingleLock dbLock(&((CMultiSQliteMFDlg*)staticWnd)->mutexDB);
	dbLock.Lock( ((CMultiSQliteMFDlg*)staticWnd)->DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		rc = sqlite3_exec(dbx, szStartThread, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			strMessage.Format(_T("  Error starting Thread #%d"), threadID);
			((CMultiSQliteMFDlg*)staticWnd)->lb->AddString(strMessage);
			dbLock.Unlock();
			return 1;
		}
	}
	dbLock.Unlock();

	
	CString strSQLInsert;
	//for (int i = 0; i < 1000; i++) {
	while (((CButton*)staticWnd->GetDlgItem(BTN_StartThreadsSingleCon))->GetCheck())
	{
			
		CString strRandomText = RandomString(1+rand()%255,1);

		strSQLInsert.Format(_T("insert into multisqlite_entries (threadid,text,appID) values (%d,'%s',%s)"), threadID, strRandomText, strAppID);
		CT2A szSQLInsert(strSQLInsert.GetString());
		CSingleLock dbLock(&((CMultiSQliteMFDlg*)staticWnd)->mutexDB);
		dbLock.Lock(((CMultiSQliteMFDlg*)staticWnd)->DB_LOCK_INTERVAL);    // Wait 100 ms...
		if (dbLock.IsLocked())
		{
			rc = sqlite3_exec(dbx, szSQLInsert, callback, 0, &zErrMsg);
			if (rc != SQLITE_OK)
			{
				strMessage.Format(_T("  Error during execution of Thread #%d"), threadID);
				((CMultiSQliteMFDlg*)staticWnd)->lb->AddString(strMessage);
				//return 1;
			}
		}
		dbLock.Unlock();
	}

	strMessage.Format(_T("  Stop Thread #%d"), threadID);
	((CMultiSQliteMFDlg*)staticWnd)->lb->AddString(strMessage);

	CString strStopThread;
	strStopThread.Format(_T("update multisqlite_threads set isActive=0 where threadID=%d and appID='%s' "), threadID, strAppID);
	CT2A szStopThread(strStopThread.GetString());
	
	dbLock.Lock(((CMultiSQliteMFDlg*)staticWnd)->DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		rc = sqlite3_exec(dbx, szStopThread, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			strMessage.Format(_T("  Error stopping Thread #%d"), threadID);
			((CMultiSQliteMFDlg*)staticWnd)->lb->AddString(strMessage);
			dbLock.Unlock();
			return 1;
		}
	}
	dbLock.Unlock();

	strMessage.Format(_T("  Thread #%d Terminated"), threadID);
	((CMultiSQliteMFDlg*)staticWnd)->lb->AddString(strMessage);

	return 0;
}





void CMultiSQliteMFDlg::OnBnClickedMulticonnectWrite()
{
	CString strOut = CString("");
	int rc;

	const int STATEMENTS = 8;
	char *zErrMsg = 0;
	const char *pSQL[STATEMENTS];
	staticWnd = this;

	CString strMultiConnectDBFile = CString("Multiple.db");
	CString strConnectionString;
	strConnectionString.Format(_T(" Data Source = ""%s""; Version = 3; Pooling = True; Max Pool Size = 100; "), strMultiConnectDBFile);
	CStringA strConnectionStringA(strConnectionString);
	const char* szConnectionString = strConnectionStringA;

	rc = sqlite3_open(szConnectionString, &db0);
	if (rc)
	{
		sqlite3_close(db0);
		db0 = NULL;
		strOut = strOut + CString(" ") + CString("Error: Cant connect (0)!");
	}
	else
	{
		strOut = strOut + CString(" ") + CString("Connected (0)!");
	}

	const char* szSQLCreateTable = "Create Table if NOT Exists Select * from multisqlite_entries (id INTEGER PRIMARY KEY AUTOINCREMENT, text VARCHAR, threadid INTEGER, tsCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP))";
	rc = sqlite3_exec(db0, szSQLCreateTable, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db0);
		db0 = NULL;
		AfxMessageBox(CString("Error Creating: ") + CString(zErrMsg));
		sqlite3_free(zErrMsg);		
	}

	//1
	rc = sqlite3_open(szConnectionString, &db1);
	if (rc)
	{
		sqlite3_close(db1);
		db1 = NULL;
		strOut = strOut + CString(" ") + CString("Error: Cant connect (1)!");		
	}
	else
	{
		strOut = strOut + CString(" ") + CString("Connected (1)!");
	}

	//2
	rc = sqlite3_open(szConnectionString, &db2);
	if (rc)
	{
		sqlite3_close(db2);
		db2 = NULL;
		strOut = strOut + CString(" ") + CString("Error: Cant connect (2)!");		
	}
	else
	{
		strOut = strOut + CString(" ") + CString("Connected (2)!");
	}

	this->lb->AddString(strOut);
	this->lb->SetCurSel(this->lb->GetCount() - 1);
}


UINT CMultiSQliteMFDlg::ThreadSQLMultiHammerIn1(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;
	
	/*
	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
		*/
	char *zErrMsg = 0;
	int rc;
	if (db1 == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into multisqlite_entries (threadid,text) values (1,'T1: TID:1 / %d')"), i);
		CT2A szSQLInsert(strSQLInsert.GetString());
		rc = sqlite3_exec(db1, szSQLInsert, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			return 1;
		}
	}

	return 0;
}

UINT CMultiSQliteMFDlg::ThreadSQLMultiHammerIn2(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;

	/*
	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
		*/

	char *zErrMsg = 0;
	int rc;
	if (db2 == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into multisqlite_entries (threadid,text) values (2,'T2: TID:2 / %d')"), i);
		CT2A szSQLInsert(strSQLInsert.GetString());
		rc = sqlite3_exec(db2, szSQLInsert, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			return 1;
		}
	}

	return 0;
}

void CMultiSQliteMFDlg::OnBnClickedSqlmultihammer()
{
	lb = (CListBox*)GetDlgItem(IDC_LIST);

	pFlickerObject = new CFlickerObject;
	AfxBeginThread(ThreadSQLMultiHammerIn1, pFlickerObject);
	AfxBeginThread(ThreadSQLMultiHammerIn2, pFlickerObject);
}

void CMultiSQliteMFDlg::OnBnClickedMulticountOnce()
{
	int rc;
	if (db0 == NULL) {
		AfxMessageBox(CString("DB NOT YET CONNECTED."));
		return;
	}

	char *zErrMsg = 0;
	char* szSelect = "Select count(*) as Count_Thread1 from multisqlite_entries where threadID=0;Select count(*) as Count_Thread2 from testtable  where threadID=1;Select count(*) as Count_Thread3 from multisqlite_entries  where threadID=2";
	rc = sqlite3_exec(db0, szSelect, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		sqlite3_close(db0);
		db0 = NULL;
		AfxMessageBox(CString("Couldn't read from database") + CString(zErrMsg));
		return;
	}
}

UINT CMultiSQliteMFDlg::ThreadSQLFlicker1(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;
	
	/*
	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
    */

	char *zErrMsg = 0;
	int rc;
	if (dbx == NULL) {
		return 1;
	}

	long lLastFlicker, lTimeNow;	
	lLastFlicker = GetTickCount64();
	long lDiff;
	CButton* btnFlickerCount = ((CButton*)staticWnd->GetDlgItem(BTN_FLICKER_COUNT));
	while (btnFlickerCount->GetCheck())
	{
		lTimeNow = GetTickCount64();
		lDiff =  lTimeNow - lLastFlicker;		
		if (lDiff>250)
			bFlickerLock1 = false;
		if (!bFlickerLock1)
		{
			lLastFlicker = GetTickCount64();
			bFlickerLock1 = true;
			char* szSelect = "Select count(*) as Count_Thread1_1 from multisqlite_entries where threadID=0; Select count(*) as Count_Thread2_1 from testtable  where threadID=1; Select count(*) as Count_Thread3_1 from multisqlite_entries  where threadID=2";
			
			CSingleLock dbLock(&((CMultiSQliteMFDlg*)staticWnd)->mutexDB);
			dbLock.Lock( ((CMultiSQliteMFDlg*)staticWnd)->DB_LOCK_INTERVAL);    // Wait 100 ms...
			if (dbLock.IsLocked())
			{
				rc = sqlite3_exec(dbx, szSelect, callback_flicker, 0, &zErrMsg);
			}
			dbLock.Unlock();
		}
	}

	return 0;
}


UINT CMultiSQliteMFDlg::ThreadSQLFlicker2(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;

	/*
	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
	*/

	char *zErrMsg = 0;
	int rc;
	if (dbx == NULL) {
		return 1;
	}
	
	long lLastFlicker, lTimeNow;
	lLastFlicker = GetTickCount64();
	long lDiff;
	while (true) 
	{
		lTimeNow = GetTickCount64();
		lDiff = lTimeNow - lLastFlicker;
		if (lDiff > 250)
			bFlickerLock1 = false;
		if (!bFlickerLock2)
		{
			bFlickerLock2 = true;
			char* szSelect = "Select count(*) as Count_Thread1_2 from multisqlite_entries where threadID=0;Select count(*) as Count_Thread2_2 from testtable  where threadID=1;Select count(*) as Count_Thread3_2 from multisqlite_entries  where threadID=2";
			CSingleLock dbLock(&((CMultiSQliteMFDlg*)staticWnd)->mutexDB);
			dbLock.Lock(((CMultiSQliteMFDlg*)staticWnd)->DB_LOCK_INTERVAL);    // Wait 100 ms...
			if (dbLock.IsLocked())
			{
				rc = sqlite3_exec(dbx, szSelect, callback_flicker, 0, &zErrMsg);
			}
			dbLock.Unlock();
		}
	}	
	return 0;
}





void CMultiSQliteMFDlg::OnBnClickedSingleinsert()
{
	int rc;
	char* zErrMsg = 0;

	CString strSQL;
	strSQL.Format(_T("insert into multisqlite_entries (appID,threadid,text) values (%s,0,'{0}')"), strAppID);
	CStringA strSQLA(strSQL);	
	const char* szInsertIntoTable = strSQLA;
	CSingleLock dbLock(&((CMultiSQliteMFDlg*)staticWnd)->mutexDB);
	dbLock.Lock(((CMultiSQliteMFDlg*)staticWnd)->DB_LOCK_INTERVAL);    // Wait 100 ms...
	if (dbLock.IsLocked())
	{
		rc = sqlite3_exec(dbx, szInsertIntoTable, NULL, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			sqlite3_close(dbx);
			dbx = NULL;
			AfxMessageBox(CString("Error inserting into table.") + CString(zErrMsg));
		}
		else
		{
			this->lb->AddString(L"Dataset inserted into Database");
		}
		this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
	}
	dbLock.Unlock();
}


void CMultiSQliteMFDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == tiStartupDelay)
	{
		KillTimer(tiStartupDelay);					
		if (IsDebuggerPresent())
		{
			Connect();
			return;
		}
		if (!UpdateApp())
			Connect();	
	}
	else if (nIDEvent == tiCloseAfterUpdate)
	{
		KillTimer(tiCloseAfterUpdate);
		PostMessage(WM_CLOSE);
		return;
	}
	else if (nIDEvent == tiRestartAfterUpdate)
	{
		CString sApp = GetAppPath();;
		this->lb->AddString(CString("Closing this application...."));
		ShellExecute(NULL, _T("open"), sApp.GetBuffer(sApp.GetLength()), NULL, NULL, SW_SHOW);
		KillTimer(tiRestartAfterUpdate);
		SetTimer(tiCloseAfterUpdate, 5000, NULL); // one event every 1000 ms = 1 s
	}
	else if (nIDEvent==tiUpdateApps)
	{	
		if (bPollock)
			return;

		bPollock = true;

		CListBox* lbApplications = (CListBox*)(staticWnd->GetDlgItem(IDLB_APPLICATIONS));
		lbApplications->ResetContent();
		int rc;
		if (dbx != NULL)
		{
			execQuery(CString(_T("update multisqlite_apps set isActive = 0 where strftime('%s', 'now') - strftime('%s', tsLastPoll) > 30")));

			execQuery(CString("update multisqlite_apps set tsLastPoll = CURRENT_TIMESTAMP, isActive=1 where id = ") + strAppID);

			char* zErrMsg = 0;
			char* szSelect = "Select name || ' <ID:' || id || '>' from multisqlite_apps where strftime('%s', 'now') - strftime('%s', tsLastPoll) < 30";

			CSingleLock dbLock(&((CMultiSQliteMFDlg*)staticWnd)->mutexDB);
			dbLock.Lock(((CMultiSQliteMFDlg*)staticWnd)->DB_LOCK_INTERVAL);    // Wait 100 ms...
			if (dbLock.IsLocked())
			{
				rc = sqlite3_exec(dbx, szSelect, UpdateApplications, 0, &zErrMsg);
			}
			dbLock.Unlock();
		}

		bPollock = false;

	}

	CDialogEx::OnTimer(nIDEvent);
}







void CMultiSQliteMFDlg::OnClose()
{
	// TODO: Fügen Sie hier Ihren Meldungshandlercode ein, und/oder benutzen Sie den Standard.

	int rc;
	if (dbx != NULL)
	{
	
		execQuery(CString("update multisqlite_apps set isActive = 0  where id = ") + strAppID);
	}

	CDialogEx::OnClose();
}


void CMultiSQliteMFDlg::OnBnClickedStartthreadssinglecon()
{	
	if (dbx == NULL)
	{
		CButton* btn = (CButton*)staticWnd->GetDlgItem(BTN_StartThreadsSingleCon);
		lb->AddString(_T("Error: DB not connected"));
		btn->SetCheck(false);
		return;
	}
	lb = (CListBox*)GetDlgItem(IDC_LIST);

	if (!((CButton*)staticWnd->GetDlgItem(BTN_StartThreadsSingleCon))->GetCheck())
	{
		lb->AddString(_T("Terminate all Threads..."));
		return;
	}

	pFlickerObject = new CFlickerObject();
	CString strStartThreads;
	strStartThreads.Format(_T("Start %d threads...."),nNumberThreads);
	lb->AddString(strStartThreads);
	for (int i=0;i<nNumberThreads;i++)
		AfxBeginThread(ThreadSQLHammerIn, pFlickerObject);	
	strStartThreads.Format(_T("...%d threads started"), nNumberThreads);
	lb->AddString(strStartThreads);
}


void CMultiSQliteMFDlg::ShowNumberOfThreads()
{
	CStatic* lblScrollBar = (CStatic*)GetDlgItem(IDC_LBL_NUMBER_THREADS);
	CString strValue;
	strValue.Format(_T("Number of Threads: %d"), nNumberThreads);
	lblScrollBar->SetWindowTextW(strValue);
}

void CMultiSQliteMFDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Fügen Sie hier Ihren Meldungshandlercode ein, und/oder benutzen Sie den Standard.
	// https://programming.vip/docs/an-example-of-window-scrolling-using-scrollbars-windows-mobile-mfc.html

	CScrollBar* scrollBarNumberThreads = (CScrollBar*)GetDlgItem(IDC_SLIDER_NUMBER_THREADS);

	if (pScrollBar == scrollBarNumberThreads) {		
		
		switch (nSBCode)
		{
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
			case SB_BOTTOM:
			case SB_TOP:
			case SB_LINEUP:
			case SB_LINEDOWN :
			case SB_PAGEUP:
			case SB_PAGEDOWN:			
			{
				// TODO: Add your message handler code here and/or call default				
				SCROLLINFO scrollinfo;
				GetScrollInfo(SB_HORZ, &scrollinfo);
				int& nPosFrom = scrollinfo.nPos;
				int nPosTo = nPos;	
				if (nPosTo <= 0)
					nPosTo = 1;
				nNumberThreads = nPosTo;
				ShowNumberOfThreads();
			}	
			default:
				break;
		}
			
		CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}

std::string CMultiSQliteMFDlg::ProcessIdToName(DWORD processId)
{
	std::string ret;
	HANDLE handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		processId /* This is the PID, you can find one from windows task manager */
	);
	if (handle)
	{
		DWORD buffSize = 1024;
		CHAR buffer[1024];
		if (QueryFullProcessImageNameA(handle, 0, buffer, &buffSize))
		{
			ret = buffer;
		}
		else
		{
			printf("Error GetModuleBaseNameA : %lu", GetLastError());
		}
		CloseHandle(handle);
	}
	else
	{
		printf("Error OpenProcess : %lu", GetLastError());
	}
	return ret;
}








void CMultiSQliteMFDlg::OnBnClickedFlickerCount()
{
	pFlickerObject = new CFlickerObject;

	int rc;
	if (dbx == NULL) {
		CButton* btn = (CButton*)staticWnd->GetDlgItem(BTN_FLICKER_COUNT);
		this->lb->AddString(CString("Error: DB NOT YET CONNECTED."));
		btn->SetCheck(false);
		return;
	}

	AfxBeginThread(ThreadSQLFlicker1, pFlickerObject);
	//AfxBeginThread(ThreadSQLFlicker2, pFlickerObject);
}

/*
int CMultiSQliteMFDlg::getVersionNumber(CString strVersion)
{
	strVersion.Remove('.');
	CT2A ascVersion(strVersion);
	char* p;
	int x = strtol(ascVersion, &p, 10);
	if (*p != 0) {
		return 0;
	}
	return x;
}
*/

bool CMultiSQliteMFDlg::UpdateApp()
{

	this->lb->AddString(_T("Trying to update Application ...."));		
	while (PeekAndPump()) {};
	
	CString strDebug = getAppData() +  "VersionTest.Debug";
	CString strRelease = getAppData() + "VersionTest.Release";
	try
	{
		DeleteFile(strDebug);
	}
	catch (...) {}
	try
	{
		DeleteFile(strRelease);
	}
	catch (...) {}

	TCHAR urlRelease[] = TEXT("https://github.com/ushaufe/SQlite4CPP/raw/master/Release/MultiSQLite_CPP.exe");
	TCHAR urlDebug[] = TEXT("https://github.com/ushaufe/SQlite4CPP/raw/master/Debug/MultiSQLite_CPP.exe");
	CString strVersionRelease, strVersionDebug;
	CString strVersion = GetAppVersion(GetAppPath());
	lb->AddString(CString("   Installed Version: ") + strVersion);
	try
	{
		int index = this->lb->AddString(_T("   Checking Debug..."));		
		while (PeekAndPump()) {};
		CInternetSession connection;
		CStdioFile* stream = connection.OpenURL(urlDebug);

		//const int capacity = stream->GetLength();
		const int capacity = stream->SeekToEnd();
		stream->SeekToBegin();
		char* buffer = new char[capacity];
		int bytes_read = stream->Read(buffer, capacity);

		CFile of(strDebug, CFile::modeCreate | CFile::modeWrite);
		of.Write(buffer, bytes_read);
		of.Close();
		this->lb->DeleteString(index);
		strVersionDebug = GetAppVersion(strDebug);
		this->lb->AddString(_T("   Checking Debug... ") + strVersionDebug);
	}
	catch (...) { }

	try
	{
		int index = this->lb->AddString(_T("   Checking Release..."));
		while (PeekAndPump()) {};
		CInternetSession connection;
		CStdioFile* stream = connection.OpenURL(urlRelease);

		//const int capacity = stream->GetLength();
		const int capacity = stream->SeekToEnd();
		stream->SeekToBegin();
		char* buffer = new char[capacity];
		int bytes_read = stream->Read(buffer, capacity);

		CFile of(strRelease, CFile::modeCreate | CFile::modeWrite);
		of.Write(buffer, bytes_read);
		of.Close();
		this->lb->DeleteString(index);
		strVersionRelease = GetAppVersion(strRelease);
		this->lb->AddString(_T("   Checking Release... ") + strVersionRelease);
	}
	catch (...) { }

	int nReleaseVersion = getDBVersionNumber(strVersionRelease);
	int nDebugVersion = getDBVersionNumber(strVersionDebug);
	int nVersion = getDBVersionNumber(strVersion);
	int nNewVersion = 0;
	CString strNewFile;
	if (nDebugVersion > nReleaseVersion)
	{
		nNewVersion = nDebugVersion;
		strNewFile = strDebug;
	}
	else
	{
		nNewVersion = nReleaseVersion;
		strNewFile = strRelease;
	}
	if (nNewVersion > nVersion)
	{
		CString strCurrentApp = GetAppPath();
		lb->AddString(CString("Updating to version: ") + GetAppVersion(strNewFile));
		try { DeleteFile(getAppData() + "Temp.file"); }
		catch (...) {}
		try { MoveFile(strCurrentApp, getAppData() + "Temp.file"); }
		catch (...) {}
		try { MoveFile(strNewFile, strCurrentApp); }
		catch (...) {}
		lb->AddString(CString("Restarting Application..."));
		SetTimer(tiRestartAfterUpdate, 5000, NULL); // one event every 1000 ms = 1 s
		return true;
	}
	else
	{
		lb->AddString(CString("This is the newest version. No Update required."));
	}				
	return false;
}

void CMultiSQliteMFDlg::OnBnClickedUpdate()
{
	UpdateApp();	
}
