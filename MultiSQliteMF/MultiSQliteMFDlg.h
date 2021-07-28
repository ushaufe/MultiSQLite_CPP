
// MultiSQliteMFDlg.h : header file
//

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
	enum { IDD = IDD_MULTISQLITEMF_DIALOG };
#endif

	protected:
	static CListBox* lb;
	static sqlite3 *db;

	static sqlite3 *db0;
	static sqlite3 *db1;
	static sqlite3 *db2;

	// Some global pool of database connections
	//pool_t *db_pool;

	/*
	sqlite3 *get_database() {
		sqlite3 *db = pool_pop(db_pool);
		if (db)
			return db;
		if (sqlite3_open("database.sqlite3", &db))
			return NULL;
		return db;
	}
	*/

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	static void t1();
	static void t2();
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
	static int callback(void *NotUsed, int argc, char **argv, char **azColName);

	static int callback_flicker(void *NotUsed, int argc, char **argv, char **azColName);
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
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




