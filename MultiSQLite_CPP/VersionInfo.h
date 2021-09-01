//#include "stdafx.h"
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

CString GetAppVersion(CString sApplication);
CString GetAppPath();
CString GetAppDir();
CString GetContainingFolder(CString& file);

/*
class CGlobalFunctions  : public CObject
{
public:
    CGlobalFunctions();
    virtual ~CGlobalFunctions();
public:
    static CString GetFileVersionX();
    static CString GetProductVersionX();
    static CString GetVersionInfo(HMODULE hLib, CString csEntry);
    static CString FormatVersion(CString cs);

private:
    static CString m_csFileVersion;
    static CString m_csProductVersion;
};
*/