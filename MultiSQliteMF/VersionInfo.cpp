#include "VersionInfo.h"


#include <Windows.h>
#pragma comment(lib,"version")


CString GetContainingFolder(CString &file)
{
    CFileFind fileFind;
    fileFind.FindFile(file);
    fileFind.FindNextFile();
    return fileFind.GetRoot();
}


CString GetAppPath()
{
	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	CString sPath(szPath);
	
	return sPath;
}

CString GetAppDir()
{
	return GetContainingFolder( GetAppPath() );	
}



CString GetAppVersion(CString sApplication)
{

	LPCWSTR szVersionFile =  CT2CW(sApplication); //_T("C:\\Software\\sqliteadmin\\sqliteadmin.exe");
	//TCHAR szVersionFile[512];
	CString sVersion;
	DWORD  verHandle = NULL;
	UINT   size      = 0;
	LPBYTE lpBuffer  = NULL;
	DWORD  verSize   = GetFileVersionInfoSize( szVersionFile, &verHandle);

	if (verSize != NULL)
	{
		LPSTR verData = new char[verSize];

		if (GetFileVersionInfo( szVersionFile, verHandle, verSize, verData))
		{
			if (VerQueryValue(verData,_T("\\"),(VOID FAR* FAR*)&lpBuffer,&size))
			{
				if (size)
				{
					VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
					if (verInfo->dwSignature == 0xfeef04bd)
					{
                   
						// Doesn't matter if you are on 32 bit or 64 bit,
						// DWORD is always 32 bits, so first two revision numbers
						// come from dwFileVersionMS, last two come from dwFileVersionLS						
						sVersion.Format(_T( "%d.%d.%d.%d\n"),
						( verInfo->dwFileVersionMS >> 16 ) & 0xffff,
						( verInfo->dwFileVersionMS >>  0 ) & 0xffff,
						( verInfo->dwFileVersionLS >> 16 ) & 0xffff,
						( verInfo->dwFileVersionLS >>  0 ) & 0xffff
						);
					}
				}
			}
		}
		delete[] verData;
	}
	return sVersion;
}

/*
CString CGlobalFunctions::m_csFileVersion;
CString CGlobalFunctions::m_csProductVersion;

CGlobalFunctions::CGlobalFunctions()
{

}

CGlobalFunctions::~CGlobalFunctions()
{

}
// This is the key method
CString CGlobalFunctions::GetVersionInfo(HMODULE hLib, CString csEntry)
{
  CString csRet;

  if (hLib == NULL)
    hLib = AfxGetResourceHandle();
  
  HRSRC hVersion = FindResource( hLib, 
    MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION );
  if (hVersion != NULL)
  {
    HGLOBAL hGlobal = LoadResource( hLib, hVersion ); 
    if ( hGlobal != NULL)  
    {  
  
      LPVOID versionInfo  = LockResource(hGlobal);  
      if (versionInfo != NULL)
      {
        DWORD vLen,langD;
        BOOL retVal;    
    
        LPVOID retbuf=NULL;
    
        static char fileEntry[256];

        sprintf(fileEntry,_T("\\VarFileInfo\\Translation"));
        retVal = VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen);
        if (retVal && vLen==4) 
        {
          memcpy(&langD,retbuf,4);            
          sprintf(fileEntry, "\\StringFileInfo\\%02X%02X%02X%02X\\%s",
                  (langD & 0xff00)>>8,langD & 0xff,(langD & 0xff000000)>>24, 
                  (langD & 0xff0000)>>16, csEntry);            
        }
        else 
          sprintf(fileEntry, "\\StringFileInfo\\%04X04B0\\%s", 
            GetUserDefaultLangID(), csEntry);

        if (VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen)) 
          csRet = (char*)retbuf;
      }
    }

    UnlockResource( hGlobal );  
    FreeResource( hGlobal );  
  }

  return csRet;
}

// Re-formats a string formatted as "m,n,o,p" to format as "m.nop"
CString CGlobalFunctions::FormatVersion(CString cs)
{
  CString csRet;
  if (!cs.IsEmpty())
  {
    cs.TrimRight();
    int iPos = cs.Find(',');
    if (iPos == -1)
      return _T("");
    cs.TrimLeft();
    cs.TrimRight();
    csRet.Format(_T("%s."), cs.Left(iPos));

    while (1)
    {
      cs = cs.Mid(iPos + 1);
      cs.TrimLeft();
      iPos = cs.Find(',');
      if (iPos == -1)
      {
        csRet +=cs;
        break;
      }
      csRet += cs.Left(iPos);
    }
  }

  return csRet;
}

// Loads "FileVersion" from resource formats it and keeps it in mind
CString CGlobalFunctions::GetFileVersionX()
{
  if (m_csFileVersion.IsEmpty())
  {
    CString csVersion = FormatVersion(GetVersionInfo(NULL, _T("FileVersion")));
    m_csFileVersion.Format(_T("Version %s (Build %s)"), 
      csVersion, GetVersionInfo(NULL, _T(_T("SpecialBuild"))));
  }

  return m_csFileVersion;
}

// Loads "ProductVersion" from resource formats it and keeps it in mind
CString CGlobalFunctions::GetProductVersionX()
{
  if (m_csProductVersion.IsEmpty())
    m_csProductVersion = FormatVersion(GetVersionInfo(NULL, _T("ProductVersion")));

  return m_csProductVersion;
}
*/