
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

BOOL GetModSignInfo(const char * pszModFilePath, MOD_SIGN_INFO & signInfo)
{
	BOOL bRet = FALSE;

	wchar_t wszFilePath[MAX_PATH] = { 0 };

	HCERTSTORE hStore = NULL;
	HCRYPTMSG hMsg = NULL;
	PCCERT_CONTEXT pCertContext = NULL;
	BOOL fResult;
	DWORD dwEncoding, dwContentType, dwFormatType;
	PCMSG_SIGNER_INFO pSignerInfo = NULL;	
	DWORD dwSignerInfo;
	CERT_INFO CertInfo;

	mbstowcs(wszFilePath, pszModFilePath, MAX_PATH);

	// Get message handle and store handle from the signed file.
	fResult = CryptQueryObject(CERT_QUERY_OBJECT_FILE,
		wszFilePath,
		CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
		CERT_QUERY_FORMAT_FLAG_BINARY,
		0,
		&dwEncoding,
		&dwContentType,
		&dwFormatType,
		&hStore,
		&hMsg,
		NULL);
	if (!fResult)
	{
		//_tprintf(_T("CryptQueryObject failed with %x\n"), GetLastError());		
	}
	
	// Get signer information size.
	fResult = CryptMsgGetParam(hMsg,
		CMSG_SIGNER_INFO_PARAM,
		0,
		NULL,
		&dwSignerInfo);
	if (!fResult)
	{
		_tprintf(_T("CryptMsgGetParam failed with %x\n"), GetLastError());

		goto THE_END;
	}

	// Allocate memory for signer information.
	pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);
	if (!pSignerInfo)
	{
		_tprintf(_T("Unable to allocate memory for Signer Info.\n"));	

		goto THE_END;
	}

	// Get Signer Information.
	fResult = CryptMsgGetParam(hMsg,
		CMSG_SIGNER_INFO_PARAM,
		0,
		(PVOID)pSignerInfo,
		&dwSignerInfo);
	if (!fResult)
	{
		_tprintf(_T("CryptMsgGetParam failed with %x\n"), GetLastError());		

		goto THE_END;
	}

	CertInfo.Issuer = pSignerInfo->Issuer;
	CertInfo.SerialNumber = pSignerInfo->SerialNumber;

	pCertContext = CertFindCertificateInStore(hStore,
		ENCODING,
		0,
		CERT_FIND_SUBJECT_CERT,
		(PVOID)&CertInfo,
		NULL);
	if (!pCertContext)
	{
		_tprintf(_T("CryptMsgGetParam failed with %x\n"), GetLastError());

		goto THE_END;
	}

	bRet = mod_GetModSignInfo(pCertContext, signInfo);

THE_END:
	if (pSignerInfo != NULL)
	{
		LocalFree(pSignerInfo);
	}	

	if (pCertContext != NULL)
	{
		CertFreeCertificateContext(pCertContext);
	}

	if (hStore != NULL)
	{
		CertCloseStore(hStore, 0);
	}

	if (hMsg != NULL)
	{
		CryptMsgClose(hMsg);
	}

	return bRet;
}

BOOL mod_GetModSignInfo(PCCERT_CONTEXT pCertContext, MOD_SIGN_INFO & signInfo)
{
	BOOL bRet = TRUE;

	LPTSTR szName = NULL;
	DWORD dwData;

	FILETIME fTime = { 0 };

	//NotBefore
	fTime.dwHighDateTime = pCertContext->pCertInfo->NotBefore.dwHighDateTime;
	fTime.dwLowDateTime = pCertContext->pCertInfo->NotBefore.dwLowDateTime;
	FileTimeToSystemTime(&fTime,&signInfo.timeStart);

	//NotAfter
	fTime.dwHighDateTime = pCertContext->pCertInfo->NotAfter.dwHighDateTime;
	fTime.dwLowDateTime = pCertContext->pCertInfo->NotAfter.dwLowDateTime;
	FileTimeToSystemTime(&fTime, &signInfo.timeEnd);

	__try
	{
		//Serial Number.		
		dwData = pCertContext->pCertInfo->SerialNumber.cbData;
		for (DWORD n = 0; n < dwData; n++)
		{
			wchar_t wszTemp[4] = { 0 };

			wsprintf(wszTemp, L"%02x", pCertContext->pCertInfo->SerialNumber.pbData[dwData - (n + 1)]);	

			wcscat(signInfo.wszSerialNumber, wszTemp);
		}
		
		// Get Issuer name size.
		if (!(dwData = CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			CERT_NAME_ISSUER_FLAG,
			NULL,
			NULL,
			0)))
		{
			_tprintf(_T("CertGetNameString failed.\n"));
		}
		// Allocate memory for Issuer name.
		szName = (LPTSTR)LocalAlloc(LPTR, dwData * sizeof(TCHAR));
		if (!szName)
		{
			_tprintf(_T("Unable to allocate memory for issuer name.\n"));
		}

		// Get Issuer name.
		if (!(CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			CERT_NAME_ISSUER_FLAG,
			NULL,
			szName,
			dwData)))
		{
			_tprintf(_T("CertGetNameString failed.\n"));
		}
		
		wcscpy(signInfo.wszIssuer, szName);

		LocalFree(szName);
		szName = NULL;

		// Get Subject name size.
		if (!(dwData = CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			0,
			NULL,
			NULL,
			0)))
		{
			_tprintf(_T("CertGetNameString failed.\n"));
		}
		// Allocate memory for subject name.
		szName = (LPTSTR)LocalAlloc(LPTR, dwData * sizeof(TCHAR));
		if (!szName)
		{
			_tprintf(_T("Unable to allocate memory for subject name.\n"));
		}
		// Get subject name.
		if (!(CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			0,
			NULL,
			szName,
			dwData)))
		{
			_tprintf(_T("CertGetNameString failed.\n"));
		}

		wcscpy(signInfo.wszUser, szName);
	}
	__finally
	{
		if (szName != NULL) LocalFree(szName);
	}
	
	return bRet;
}