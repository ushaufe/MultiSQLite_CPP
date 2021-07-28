
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


// Connection pooling
// https://dev.yorhel.nl/doc/sqlaccess


using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The following static elements of the class must be redefined globally
// Since they are static elements of the class and need redefinition
CWnd* CMultiSQliteMFDlg::staticWnd;

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
	: CDialogEx(IDD_MULTISQLITEMF_DIALOG, pParent)
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
	ON_BN_CLICKED(IDC_Multithread, &CMultiSQliteMFDlg::OnBnClickedMultithread)
	ON_BN_CLICKED(BTN_MFC_MULTITHREAD, &CMultiSQliteMFDlg::OnBnClickedMfcMultithread)
	ON_BN_CLICKED(BTN_Connect4SQLite, &CMultiSQliteMFDlg::OnBnClickedConnect4sqlite)
	ON_BN_CLICKED(BTN_HammerInSQL, &CMultiSQliteMFDlg::OnBnClickedHammerinsql)
	ON_BN_CLICKED(BTN_ShowDBContents, &CMultiSQliteMFDlg::OnBnClickedShowdbcontents)
	ON_BN_CLICKED(IDC_SHOW_DB_COUNT_SINGLE, &CMultiSQliteMFDlg::OnBnClickedShowDbCountSingle)
	ON_BN_CLICKED(BTN_Hammer2SQL, &CMultiSQliteMFDlg::OnBnClickedHammer2sql)
	ON_BN_CLICKED(BTN_MultiConnect_Write, &CMultiSQliteMFDlg::OnBnClickedMulticonnectWrite)
	ON_BN_CLICKED(BTN_SQLMultiHammer, &CMultiSQliteMFDlg::OnBnClickedSqlmultihammer)
	ON_BN_CLICKED(BTN_MultiCount_Once, &CMultiSQliteMFDlg::OnBnClickedMulticountOnce)
	ON_BN_CLICKED(BTN_FlickerCount, &CMultiSQliteMFDlg::OnBnClickedFlickercount)
END_MESSAGE_MAP()


// CMultiSQliteMFDlg message handlers

BOOL CMultiSQliteMFDlg::OnInitDialog()
{
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



void CMultiSQliteMFDlg::OnBnClickedBtnConnect()
{	
	// This connection is not to be used by threads
	// It should simply simulate the easiest case (simple access to SQlite)
	// In order to rule out errors in the more complex cases
	
	if (db != NULL) {
		AfxMessageBox(L"Error: Already Connected!");
		return;
	}


	// The following cases handle access via multiple applications
	// (Separate EXEs or separate DLLs)
	// Only one application can use a SQlite-database exclusively for wriging
	// Whereas multiple applications can use it for reading 
	// even when locked from one app for wriging
	int rc;
	
	// hence if a file already exists
	if (PathFileExists(L"demo.db")) {
		// only reading is possible
		rc = sqlite3_open_v2("demo.db", &db, SQLITE_OPEN_READONLY, NULL);
		SetWindowText(L"Read-Only");
	}
	else
	{
		// otherwise a second instance of the same application can be opened to read it
		rc = sqlite3_open_v2("demo.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		SetWindowText(L"Read + Write Access");
	}
	//DeleteFile(L"demo.db");
	
	const int STATEMENTS = 8;
	char *zErrMsg = 0;
	const char *pSQL[STATEMENTS];
	staticWnd = this;

	

	
	//rc = sqlite3_open_v2("demo.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	//rc = sqlite3_open("Data Source = demo.db; Read Only=True;", &db);
	//rc = sqlite3_open("Data Source=demo.db;Version=3;Pooling=True;Max Pool Size=100;Read Only=True;", &db0);

	if (rc)
	{
		AfxMessageBox(L"Error: Cant connect!");
	}
	else
	{
		AfxMessageBox(L"Connected");
	}

	
	//...Creating tables and inserting some data
	pSQL[0] = "create table myTable (FirstName varchar(30), LastName varchar(30), Age smallint, Hometown varchar(30), Job varchar(30))";

	pSQL[1] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Peter', 'Griffin', 41, 'Quahog', 'Brewery')";

	pSQL[2] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Lois', 'Griffin', 40, 'Newport', 'Piano Teacher')";

	pSQL[3] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Joseph', 'Swanson', 39, 'Quahog', 'Police Officer')";

	pSQL[4] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Glenn', 'Quagmire', 41, 'Quahog', 'Pilot')";

	pSQL[5] = "select * from myTable";

	pSQL[6] = "delete from myTable";

	pSQL[7] = "drop table myTable";

	for (int i = 0; i < STATEMENTS; i++)
	{
		rc = sqlite3_exec(db, pSQL[i], callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{			
			sqlite3_free(zErrMsg);
			break;
		}
	}

	sqlite3_close(db);
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
	return 0;
}


void CMultiSQliteMFDlg::OnLbnSelchangeList()
{
	// TODO: Add your control notification handler code here
}



void CMultiSQliteMFDlg::t1()
{
	CTime dtFrom = GetCurrentTime();
	CTime dtTo = GetCurrentTime();
	do
	{
		dtTo = GetCurrentTime();
		CString str;
		str.Format( CString("%d"), dtTo.GetTickCount());
		lb->AddString(CString("#1 ") + str);
		CString strTimeDiff;
		strTimeDiff.Format(CString("%d"), dtTo.GetSecond() - dtFrom.GetSecond());
		lb->AddString(strTimeDiff);
	} while ((dtTo.GetSecond() - dtFrom.GetSecond()) < 3);
}

void CMultiSQliteMFDlg::t2()
{
	CTime dtFrom = GetCurrentTime();
	CTime dtTo = GetCurrentTime();
	do
	{
		dtTo = GetCurrentTime();
		CString str;
		str.Format(CString("%d"), dtTo.GetTickCount());
		lb->AddString(CString("#2 ") + str);
		CString strTimeDiff;
		strTimeDiff.Format(CString("%d"), dtTo.GetSecond() - dtFrom.GetSecond());
		lb->AddString(strTimeDiff);
	} while ((dtTo.GetSecond() - dtFrom.GetSecond()) < 3);
}

void f1() 
{}

void f2()
{}


UINT CMultiSQliteMFDlg::ThreadProc1(LPVOID pParam)
{
	CMyObject* pObject = (CMyObject*)pParam;

	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
		return 1;   // if pObject is not valid

	CTime dtFrom = GetCurrentTime();
	CTime dtTo = GetCurrentTime();	
	//do
	for (int i = 0; i < 1000; i++)
	{
		for (int l = 0; l < 10; l++) {
			dtTo = GetCurrentTime();
			CString str;
			str.Format(CString("%d"), dtTo.GetTickCount());
			CString strTimeDiff;
			strTimeDiff.Format(CString("%d"), dtTo.GetTickCount() - dtFrom.GetTickCount());
			lb->AddString(CString("#TP1 ") + str + CString("Diff: ") + strTimeDiff);
		}
	} //while ((dtTo.GetSecond() - dtFrom.GetSecond()) < 3);

	return 0;   // thread completed successfully
}



UINT CMultiSQliteMFDlg::ThreadProc2(LPVOID pParam)
{
	CMyObject* pObject = (CMyObject*)pParam;

	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
		return 1;   // if pObject is not valid

	CTime dtFrom = GetCurrentTime();
	CTime dtTo = GetCurrentTime();
	//do
	for (int i = 0; i < 1000; i++)
	{
		dtTo = GetCurrentTime();
		CString str;
		str.Format(CString("%d"), dtTo.GetTickCount());
		CString strTimeDiff;
		strTimeDiff.Format(CString("%d"), dtTo.GetTickCount() - dtFrom.GetTickCount());
		lb->AddString(CString("#TP2 ") + str + CString("Diff: ") + strTimeDiff);						
	} //while ((dtTo.GetSecond() - dtFrom.GetSecond()) < 3);

	return 0;   // thread completed successfully
}




void CMultiSQliteMFDlg::OnBnClickedMultithread()
{

	lb  = (CListBox*)GetDlgItem(IDC_LIST);
	
	// TODO: Add your control notification handler code here
	thread th1(t1);
	thread th2(t2);

	// Wait for threads to finish
	th2.detach();
	th1.detach();

}



void CMultiSQliteMFDlg::OnBnClickedMfcMultithread()
{
	lb = (CListBox*)GetDlgItem(IDC_LIST);

	pNewThreadObject = new CMyObject;
	AfxBeginThread(ThreadProc1, pNewThreadObject);	
	AfxBeginThread(ThreadProc2, pNewThreadObject);
}


void CMultiSQliteMFDlg::OnBnClickedConnect4sqlite()
{
	//CListBox* list = (CListBox*)GetDlgItem(IDC_LIST);

	if (db != NULL) {
		AfxMessageBox(L"Error: Already Connected!");
		return;
	}

	DeleteFile(L"multi.db");

	const int STATEMENTS = 8;
	char *zErrMsg = 0;
	const char *pSQL[STATEMENTS];
	staticWnd = this;

	int rc;

	char* zVfs;
	int openFlags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
	//int openFlags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX;
	//openFlags |= SQLITE_OPEN_MEMORY;
	zVfs = 0;

	rc = sqlite3_open_v2("main.db", &db, openFlags, zVfs);

	//rc = sqlite3_open_v2("multi.db", &db, SQLITE_OPEN_NOMUTEX, SQLITE_OPEN_MAIN_DB);
	//rc = sqlite3_open_v2("multi.db", &db, SQLITE_OPEN_FULLMUTEX, "");
	//rc = sqlite3_open("multi.db", &db);

	if (rc)
	{
		sqlite3_close(db);
		db = NULL;
		AfxMessageBox(L"Error: Cant connect!");
		return;
	}
	else
	{
		AfxMessageBox(L"Connected");
	}
	
	const char* szSQLCreateTable = "Create Table if NOT Exists testtable(id INTEGER PRIMARY KEY AUTOINCREMENT, text VARCHAR, threadid INTEGER)";
	rc = sqlite3_exec(db, szSQLCreateTable, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		AfxMessageBox(CString("Error Creating: ") + CString(zErrMsg));
		sqlite3_free(zErrMsg);
		return;
	}
	else
	{
		AfxMessageBox(CString("DB Created."));
	}

	char* szSQLDeleteFromTable = "Delete from testtable";	
	rc = sqlite3_exec(db, szSQLDeleteFromTable, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		AfxMessageBox(CString("Error deleting from table.") + CString(zErrMsg));
		return;
	}
	
	char* szInsertIntoTable = "insert into testtable (threadid,text) values (0,'{0}')";
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		AfxMessageBox(CString("Error inserting into table.") + CString(zErrMsg));
		return;
	}
}

/*
char* conv2CS(CString const & strParam)
{
	CStringA cstraParam(strParam);
	size_t len = cstraParam.GetLength() + 1;
	char *ncharStr = new char[len];
	strcpy_s(ncharStr, len, strParam);
	return ncharStr;
}
*/

void CMultiSQliteMFDlg::OnBnClickedHammerinsql()
{
	const int STATEMENTS = 8;
	char *zErrMsg = 0;
	const char *pSQL[STATEMENTS];
	staticWnd = this;

	int rc;
	if (db == NULL) {
		AfxMessageBox(CString("DB NOT YET CONNECTED."));
		return;
	}
	
	CString strSQLInsert;
	for (int i = 0; i < 100; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text) values (0,'T1: TID:0 / %d')"), i);
		CT2A szSQLInsert(strSQLInsert.GetString());
		rc = sqlite3_exec(db, szSQLInsert, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			return;
		}
	}
	
	
	//char* szInsert = conv2CS(strInsert);
	//char* szInsert = ";
}


void CMultiSQliteMFDlg::OnBnClickedShowdbcontents()
{
	int rc;
	if (db == NULL) {
		AfxMessageBox(CString("DB NOT YET CONNECTED."));
		return;
	}

	char *zErrMsg = 0;
	char* szSelect = "Select * from testtable";
	rc = sqlite3_exec(db, szSelect, callback, 0, &zErrMsg);
	
	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		AfxMessageBox(CString("Couldn't read from database") + CString(zErrMsg));
		return;
	}
}


void CMultiSQliteMFDlg::OnBnClickedShowDbCountSingle()
{
	int rc;
	if (db == NULL) {
		AfxMessageBox(CString("DB NOT YET CONNECTED."));
		return;
	}

	char *zErrMsg = 0;
	char* szSelect = "Select count(*) as Count_Thread1 from testtable where threadID=0;Select count(*) as Count_Thread2 from testtable  where threadID=1;Select count(*) as Count_Thread3 from testtable  where threadID=2";
	rc = sqlite3_exec(db, szSelect, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		sqlite3_close(db);
		db = NULL;
		AfxMessageBox(CString("Couldn't read from database") + CString(zErrMsg));
		return;
	}
}


void CMultiSQliteMFDlg::OnBnClickedHammer2sql()
{
	lb = (CListBox*)GetDlgItem(IDC_LIST);

	pNewThreadObject = new CMyObject;
	AfxBeginThread(ThreadSQLHammerIn1, pNewThreadObject);
	AfxBeginThread(ThreadSQLHammerIn2, pNewThreadObject);
}


UINT CMultiSQliteMFDlg::ThreadSQLHammerIn1(LPVOID pParam) {
	CMyObject* pObject = (CMyObject*)pParam;

	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text) values (1,'T1: TID:1 / %d')"), i);
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
	CMyObject* pObject = (CMyObject*)pParam;

	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
		return 1;
	}

	CString strSQLInsert;
	for (int i = 0; i < 1000; i++) {
		strSQLInsert.Format(_T("insert into testtable (threadid,text) values (2,'T2: TID:2 / %d')"), i);
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

	const int STATEMENTS = 8;
	char *zErrMsg = 0;
	const char *pSQL[STATEMENTS];
	staticWnd = this;

	int rc;
	//0
	//rc = sqlite3_open("multi.db", &db0);
	rc = sqlite3_open("Data Source=multi.db;Version=3;Pooling=True;Max Pool Size=100;", &db0);
	if (rc)
	{
		sqlite3_close(db0);
		db = NULL;
		strOut = strOut + CString(" ") + CString("Error: Cant connect (0)!");
		return;
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
		return;
	}

	//1
	rc = sqlite3_open("Data Source=multi.db;Version=3;Pooling=True;Max Pool Size=100;", &db1);
	if (rc)
	{
		sqlite3_close(db1);
		db = NULL;
		strOut = strOut + CString(" ") + CString("Error: Cant connect (1)!");
		return;
	}
	else
	{
		strOut = strOut + CString(" ") + CString("Connected (1)!");
	}

	//2
	rc = sqlite3_open("Data Source=multi.db;Version=3;Pooling=True;Max Pool Size=100;", &db2);
	if (rc)
	{
		sqlite3_close(db2);
		db = NULL;
		strOut = strOut + CString(" ") + CString("Error: Cant connect (2)!");
		return;
	}
	else
	{
		strOut = strOut + CString(" ") + CString("Connected (2)!");
	}

	AfxMessageBox(strOut);
}


UINT CMultiSQliteMFDlg::ThreadSQLMultiHammerIn1(LPVOID pParam) {
	CMyObject* pObject = (CMyObject*)pParam;

	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
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
	CMyObject* pObject = (CMyObject*)pParam;

	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
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

	pNewThreadObject = new CMyObject;
	AfxBeginThread(ThreadSQLMultiHammerIn1, pNewThreadObject);
	AfxBeginThread(ThreadSQLMultiHammerIn2, pNewThreadObject);
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
	CMyObject* pObject = (CMyObject*)pParam;
	
	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
		return 1;
	}

	while (true) {
		char* szSelect = "Select count(*) as Count_Thread1_1 from testtable where threadID=0; Select count(*) as Count_Thread2_1 from testtable  where threadID=1; Select count(*) as Count_Thread3_1 from testtable  where threadID=2";
		rc = sqlite3_exec(db, szSelect, callback_flicker, 0, &zErrMsg);
	}

	return 0;
}


UINT CMultiSQliteMFDlg::ThreadSQLFlicker2(LPVOID pParam) {
	CMyObject* pObject = (CMyObject*)pParam;

	if (pObject == NULL ||
		!pObject->IsKindOf(RUNTIME_CLASS(CMyObject)))
		return 1;   // if pObject is not valid
	char *zErrMsg = 0;
	int rc;
	if (db == NULL) {
		return 1;
	}
	
	
	while (true) {
		char* szSelect = "Select count(*) as Count_Thread1_2 from testtable where threadID=0;Select count(*) as Count_Thread2_2 from testtable  where threadID=1;Select count(*) as Count_Thread3_2 from testtable  where threadID=2";
		rc = sqlite3_exec(db, szSelect, callback_flicker, 0, &zErrMsg);
	}
	

	return 0;
}


void CMultiSQliteMFDlg::OnBnClickedFlickercount()
{
	int rc;
	if (db == NULL) {
		AfxMessageBox(CString("DB NOT YET CONNECTED."));
		return;
	}

	AfxBeginThread(ThreadSQLFlicker1, pNewThreadObject);
	AfxBeginThread(ThreadSQLFlicker2, pNewThreadObject);
}
