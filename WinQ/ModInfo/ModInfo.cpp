
#include "stdafx.h"

#include "ModInfo.h"

BOOL GetModVerInfo(const char * pszModFilePath, MOD_VER_INFO & verInfo)
{
	BOOL bRet = TRUE;

	wchar_t * pVerInfoBuf = NULL;	
	DWORD dwLastError = 0;

	DWORD dwSize = GetFileVersionInfoSizeA(pszModFilePath, 0);
	if (dwSize > 0)
	{
		pVerInfoBuf = new wchar_t[dwSize + 1];
		if (NULL != pVerInfoBuf)
		{
			if (GetFileVersionInfoA(pszModFilePath, 0, dwSize, pVerInfoBuf))
			{
				mod_GetModFileVer(pVerInfoBuf, verInfo.wszFileVer);
				mod_GetModProductVer(pVerInfoBuf, verInfo.wszProductVer);

				mod_GetModNameValue(pVerInfoBuf, L"ProductName", verInfo.wszProductName);
				mod_GetModNameValue(pVerInfoBuf, L"FileDescription", verInfo.wszFileDesc);
				mod_GetModNameValue(pVerInfoBuf, L"LegalCopyright", verInfo.wszCopyrightNotice);
			}
			else
			{
				bRet = FALSE;
			}
		}
	}
	else
	{
		bRet = FALSE;
	}

	if (NULL != pVerInfoBuf)
	{
		delete[] pVerInfoBuf;
		pVerInfoBuf = NULL;
	}

	return bRet;
}

BOOL mod_GetModNameValue(const wchar_t * pVerData, const wchar_t * pwszName, wchar_t * pwszNameValue)
{
	LPTRANSLATE  pTranslate;
	UINT uTranslateLen;

	if (VerQueryValueW(pVerData, L"\\VarFileInfo\\Translation", (LPVOID*)&pTranslate, &uTranslateLen))
	{
		wchar_t szProuNameIndex[512] = { 0 };
		wchar_t * pszValue = NULL;

		swprintf_s(szProuNameIndex, 512, L"\\StringFileInfo\\%04x%04x\\%s",
			pTranslate->wLanguage, pTranslate->wCodePage,pwszName);

		if (VerQueryValueW(pVerData, szProuNameIndex, (LPVOID*)&pszValue, &uTranslateLen))
		{
			wcscpy(pwszNameValue, pszValue);
			
			return TRUE;
		}	
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

BOOL mod_GetModFileVer(const wchar_t * pVerData, wchar_t * pwszFileVer)
{
	unsigned   int   iFileInfoSize = sizeof(VS_FIXEDFILEINFO);
	VS_FIXEDFILEINFO   *pVsInfo = NULL;

	if(VerQueryValueW(pVerData,L"\\",(void**)&pVsInfo,&iFileInfoSize))
	{
		swprintf_s(pwszFileVer,64,L"%d.%d.%d.%d",HIWORD(pVsInfo->dwFileVersionMS),LOWORD(pVsInfo->dwFileVersionMS),
			HIWORD(pVsInfo->dwFileVersionLS),LOWORD(pVsInfo->dwFileVersionLS));

		return TRUE;
	}	
	else
	{
		return FALSE;
	}

	return FALSE;
}

BOOL mod_GetModProductVer(const wchar_t * pVerData, wchar_t * pwszFileProductVer)
{
	unsigned   int   iFileInfoSize = sizeof(VS_FIXEDFILEINFO);
	VS_FIXEDFILEINFO   *pVsInfo = NULL;

	if (VerQueryValueW(pVerData, L"\\", (void**)&pVsInfo, &iFileInfoSize))
	{
		swprintf_s(pwszFileProductVer, 64, L"%d.%d.%d.%d", HIWORD(pVsInfo->dwProductVersionMS), LOWORD(pVsInfo->dwProductVersionMS),
			HIWORD(pVsInfo->dwProductVersionLS), LOWORD(pVsInfo->dwProductVersionLS));

		return TRUE;
	}
	else
	{
		return FALSE;
	}

	return FALSE;
}

BOOL GetModSignInfo(const char * pszModFilePath, MOD_VER_INFO & verInfo)
{
	BOOL bRet = TRUE;

	return bRet;
}
