
// MultiSQliteMFDlg.cpp : implementation file
//

#include "pch.h"
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
sqlite3* CMultiSQliteMFDlg::db;
sqlite3* CMultiSQliteMFDlg::db0;
sqlite3* CMultiSQliteMFDlg::db1;
sqlite3* CMultiSQliteMFDlg::db2;

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
	db = NULL;
	db0 = NULL;
	db1 = NULL;
	db2 = NULL;

	bFlickerLock1 = false;
	bFlickerLock2 = false;
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
	ON_BN_CLICKED(BTN_FlickerCount, &CMultiSQliteMFDlg::OnBnClickedFlickercount)	
	ON_WM_TIMER()	
END_MESSAGE_MAP()


// CMultiSQliteMFDlg message handlers

BOOL CMultiSQliteMFDlg::OnInitDialog()
{
	//CFont font;
	//font.CreatePointFont(16, _T("Arial"));
	
	CDialogEx::OnInitDialog();

	

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

	lb->AddString(CString( APP_NAME ));
	lb->AddString(CString("Version: ") + GetAppVersion(GetAppPath()));
	CStatic* lblVersion =  (CStatic*)GetDlgItem(IDC_VERSION);
	CString strVersion = CString("Version: ") + GetAppVersion(GetAppPath());
	lblVersion->SetWindowText( strVersion);	
	Connect();
	

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
	bmpBackground.LoadBitmap(IDB_BACKGROUND);
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
	rc = sqlite3_exec(db, szQuery, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		sqlite3_free(zErrMsg);
		return false;
	}
	return true;
}

void CMultiSQliteMFDlg::Connect()
{		
	char* szAppData;
	size_t lenAppData;
	errno_t err = _dupenv_s(&szAppData, &lenAppData, "APPDATA");

	CString strAppData(szAppData);

	// This connection is not to be used by threads
	// It should simply simulate the easiest case (simple access to SQlite)
	// In order to rule out errors in the more complex cases

	if (db != NULL) {
		this->lb->AddString(L"Error: Already Connected!");
		return;
	}

	if (strAppData.GetLength() > 0) {
		if (strAppData[strAppData.GetLength() - 1] != '\\')
			strAppData += '\\';		
	}

	strAppData += CString("MultiSQlite") + CString("\\");

	CStringW strwAppData(strAppData);
	LPCWSTR ptrStrAppData = strwAppData;
	CreateDirectory(ptrStrAppData, NULL);
		
	strDatabaseFile = strAppData + + DB_NAME;
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
	rc = sqlite3_open_v2(szDatabaseFile, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	this->lb->AddString(L"Read + Write Access");

	const int STATEMENTS = 8;
	char* zErrMsg = 0;
	//const char* pSQL[STATEMENTS];
	staticWnd = this;


	if (rc != SQLITE_OK)
	{
		db = NULL;
		this->lb->AddString(L"Error: Cant connect!");
	}
	else
	{
		this->lb->AddString(L"Connected");
		this->lb->AddString(L"Database: " + strDatabaseFile);
	}

	CString strRevision;
	strRevision.Format(_T("%d"), DB_REVISION);
	int nDBRevision = -1;
	if (getSQLInt(_T("Select revision from version  order by id DESC LIMIT 1"), nDBRevision))
	{
		if (nDBRevision < DB_REVISION)
		{
			this->lb->AddString(L"Error: Old database....");
			this->lb->AddString(L"       Deleting tables....");
			execQuery(CString("drop table if exists version"));
			execQuery(CString("drop table if exists testtable"));
			execQuery(CString("drop table if exists apps"));
			this->lb->AddString(L"       Disconnect....");
			this->lb->AddString(L"       Deleting Database....");
			sqlite3_close(db);
			DeleteFile(strDatabaseFile);
			this->lb->AddString(L"       Recreate database and connect....");
			rc = sqlite3_open_v2(szDatabaseFile, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
			if (rc != SQLITE_OK)
			{
				db = NULL;
				this->lb->AddString(L"Error: Cant connect!");
			}
			else
			{
				this->lb->AddString(L"Connected");
				this->lb->AddString(L"Database: " + strDatabaseFile);
			}
		}
	}

	if (db != NULL) {
		UINT_PTR myTimer = SetTimer(1, 5000, NULL); // one event every 1000 ms = 1 s
	}

	CString appName = CString("MultiSQLite_CPP");
	
	
	execQuery( CString( "Create Table if NOT Exists version (id INTEGER PRIMARY KEY AUTOINCREMENT, revision INTEGER) ") );
	execQuery( CString( "Create Table if NOT Exists testtable (id INTEGER PRIMARY KEY AUTOINCREMENT, text VARCHAR, threadID INTEGER, appID INTEGER, tsCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP) " ) );
	execQuery( CString( "Create Table if NOT Exists apps (id INTEGER PRIMARY KEY AUTOINCREMENT, tsCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  tsLastPoll TIMESTAMP DEFAULT CURRENT_TIMESTAMP, name TEXT) ") );
	execQuery( CString( "insert into apps (name) values ('") + appName + CString( "')") ) ;
	setAppID();
	
	// Create a table that can hold text-data along with the thread-id of the thread that created the data
	CString strInsert;
	strInsert.Format(_T("insert into testtable (appID,threadid,text) values (%s,0,'%s')"), strAppID, CTime::GetCurrentTime().Format("%Y/%m/%d %H:%M")); // 
	execQuery(strInsert);
	execQuery(CString( "Delete from version") );
	execQuery(CString( "insert into version (id,revision) values (0,") + strRevision + CString(")"));

	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
}

bool  CMultiSQliteMFDlg::getSQLInt(CString strSQL, int& nInt)
{
	// https://wang.yuxuan.org/blog/item/2007/05/simple-sqlite-test-c-program-without-callbacks
	//https://www.programmersought.com/article/22293267117/
	//https://stackoverflow.com/questions/14743061/sqlite3-exec-without-callback/26463522

	CStringA strSQLA(strSQL);
	const char* szSQL = strSQLA;

	sqlite3_stmt* stmt = NULL;
	int rc = sqlite3_prepare_v2(db, szSQL, -1, &stmt, NULL);
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
			if (type == SQLITE_INTEGER)
			{
				nInt = sqlite3_column_int(stmt, colIndex);			
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


boolean CMultiSQliteMFDlg::setAppID() {
	// https://wang.yuxuan.org/blog/item/2007/05/simple-sqlite-test-c-program-without-callbacks
	//https://www.programmersought.com/article/22293267117/
	//https://stackoverflow.com/questions/14743061/sqlite3-exec-without-callback/26463522

	sqlite3_stmt* stmt = NULL;
	int rc = sqlite3_prepare_v2(db, "SELECT id FROM apps order by id DESC LIMIT 1", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return rc;

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
				SetWindowText(  CString(_T("Haufe Multi-SQlite for C++ <ID: ") ) + strAppID + CString(">") ) ;
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
	if (db == NULL) {
		this->lb->AddString(_T("DB NOT YET CONNECTED."));
		return;
	}
	
	CString strSQLInsert;
	for (int i = 0; i < nNumberInserts; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text,appid) values (0,'T1: TID:0 / %d',%s)"), i, strAppID);
		CT2A szSQLInsert(strSQLInsert.GetString());
		rc = sqlite3_exec(db, szSQLInsert, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			return;
		}
	}
	
	CString strMessage;
	strMessage.Format(_T("%d Rows inserted into database"), nNumberInserts);
	this->lb->AddString(strMessage);
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
}


void CMultiSQliteMFDlg::OnBnClickedShowdbcontents()
{
	int rc;
	if (db == NULL) {
		this->lb->AddString(_T("DB NOT YET CONNECTED."));
		return;
	}

	char *zErrMsg = 0;
	char* szSelect = "Select * from testtable";
	rc = sqlite3_exec(db, szSelect, callback, 0, &zErrMsg);
	
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		this->lb->AddString(_T("Couldn't read from database") + CString(zErrMsg));
		return;
	}
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
}


void CMultiSQliteMFDlg::OnBnClickedShowDbCountSingle()
{
	int rc;
	if (db == NULL) {
		this->lb->AddString(_T("DB NOT YET CONNECTED."));
		return;
	}

	char *zErrMsg = 0;
	char* szSelect = "Select count(*) as Count_Thread_GUI from testtable where threadID=0;Select count(*) as Count_Thread1 from testtable  where threadID=1;Select count(*) as Count_Thread2 from testtable  where threadID=2";
	rc = sqlite3_exec(db, szSelect, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		this->lb->AddString(_T("Couldn't read from database") + CString(zErrMsg));
		return;
	}
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
}


void CMultiSQliteMFDlg::OnBnClickedHammer2sql()
{
	lb = (CListBox*)GetDlgItem(IDC_LIST);

	pFlickerObject = new CFlickerObject();
	AfxBeginThread(ThreadSQLHammerIn1, pFlickerObject);
	AfxBeginThread(ThreadSQLHammerIn2, pFlickerObject);
}


UINT CMultiSQliteMFDlg::ThreadSQLHammerIn1(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;

	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text,appID) values (1,'T1: TID:1 / %d',%s)"), i, strAppID);
		CT2A szSQLInsert(strSQLInsert.GetString());
		rc = sqlite3_exec(db, szSQLInsert, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			return 1;
		}
	}

	return 0;
}

UINT CMultiSQliteMFDlg::ThreadSQLHammerIn2(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;

	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text,appID) values (2,'T2: TID:2 / %d',%s)"), i, strAppID);
		CT2A szSQLInsert(strSQLInsert.GetString());
		rc = sqlite3_exec(db, szSQLInsert, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			return 1;
		}
	}

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

	const char* szSQLCreateTable = "Create Table if NOT Exists testtable(id INTEGER PRIMARY KEY AUTOINCREMENT, text VARCHAR, threadid INTEGER)";
	rc = sqlite3_exec(db0, szSQLCreateTable, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db0);
		db = NULL;
		AfxMessageBox(CString("Error Creating: ") + CString(zErrMsg));
		sqlite3_free(zErrMsg);		
	}

	//1
	rc = sqlite3_open(szConnectionString, &db1);
	if (rc)
	{
		sqlite3_close(db1);
		db = NULL;
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
		db = NULL;
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

	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db1 == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text) values (1,'T1: TID:1 / %d')"), i);
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

	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db2 == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text) values (2,'T2: TID:2 / %d')"), i);
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
	char* szSelect = "Select count(*) as Count_Thread1 from testtable where threadID=0;Select count(*) as Count_Thread2 from testtable  where threadID=1;Select count(*) as Count_Thread3 from testtable  where threadID=2";
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
	
	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid

	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
		return 1;
	}

	long lLastFlicker, lTimeNow;	
	lLastFlicker = GetTickCount64();
	long lDiff;
	while (true) 
	{
		lTimeNow = GetTickCount64();
		lDiff =  lTimeNow - lLastFlicker;		
		if (lDiff>250)
			bFlickerLock1 = false;
		if (!bFlickerLock1)
		{
			lLastFlicker = GetTickCount64();
			bFlickerLock1 = true;
			char* szSelect = "Select count(*) as Count_Thread1_1 from testtable where threadID=0; Select count(*) as Count_Thread2_1 from testtable  where threadID=1; Select count(*) as Count_Thread3_1 from testtable  where threadID=2";
			rc = sqlite3_exec(db, szSelect, callback_flicker, 0, &zErrMsg);
		}
	}

	return 0;
}


UINT CMultiSQliteMFDlg::ThreadSQLFlicker2(LPVOID pParam) {
	CFlickerObject* pFlickerObject = (CFlickerObject*)pParam;

	if (pFlickerObject == NULL ||
		!pFlickerObject->IsKindOf(RUNTIME_CLASS(CFlickerObject)))
		return 1;   // if pObject is not valid

	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
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
			char* szSelect = "Select count(*) as Count_Thread1_2 from testtable where threadID=0;Select count(*) as Count_Thread2_2 from testtable  where threadID=1;Select count(*) as Count_Thread3_2 from testtable  where threadID=2";
			rc = sqlite3_exec(db, szSelect, callback_flicker, 0, &zErrMsg);
		}
	}	
	return 0;
}


void CMultiSQliteMFDlg::OnBnClickedFlickercount()
{
	pFlickerObject = new CFlickerObject;

	int rc;
	if (db == NULL) {
		this->lb->AddString(CString("Error: DB NOT YET CONNECTED."));
		return;
	}

	AfxBeginThread(ThreadSQLFlicker1, pFlickerObject);
	//AfxBeginThread(ThreadSQLFlicker2, pFlickerObject);
}


void CMultiSQliteMFDlg::OnBnClickedSingleinsert()
{
	int rc;
	char* zErrMsg = 0;

	CString strSQL;
	strSQL.Format(_T("insert into testtable (appID,threadid,text) values (%s,0,'{0}')"), strAppID);
	CStringA strSQLA(strSQL);	
	const char* szInsertIntoTable = strSQLA;
	rc = sqlite3_exec(db, szInsertIntoTable, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		AfxMessageBox(CString("Error inserting into table.") + CString(zErrMsg));
	}
	else
	{
		this->lb->AddString(L"Dataset inserted into Database");
	}
	this->lb->SetCurSel(this->lb->GetCount() - 1); // List box is zero based.
}


void CMultiSQliteMFDlg::OnTimer(UINT_PTR nIDEvent)
{
	CListBox* lbApplications = (CListBox*)(staticWnd->GetDlgItem(IDLB_APPLICATIONS));
	lbApplications->ResetContent();	
	int rc;
	if (db != NULL)
	{
		execQuery(CString(_T("update apps set tsLastPoll = CURRENT_TIMESTAMP where id = ")) + strAppID);

		char* zErrMsg = 0;
		char* szSelect = "Select name || ' <ID:' || id || '>' from apps where strftime('%s', 'now') - strftime('%s', tsLastPoll) < 30";
	
		rc = sqlite3_exec(db, szSelect, UpdateApplications, 0, &zErrMsg);		
	}
	
	CDialogEx::OnTimer(nIDEvent);
}





