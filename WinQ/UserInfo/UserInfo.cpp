
#include "stdafx.h"

#include "UserInfo.h"

CUserInfo::CUserInfo(const wchar_t * pDomainName)
{
	memset(m_wszDomainName, 0, MAX_PATH);

	if (NULL != pDomainName)
	{
		wcscpy(m_wszDomainName, pDomainName);
	}
}

CUserInfo::~CUserInfo()
{

}

BOOL CUserInfo::GetUsersList(ARRAY_USER &userList)
{
	BOOL bRet = TRUE;

	LPUSER_INFO_0 pBuf = NULL;
	LPUSER_INFO_0 pTmpBuf;
	DWORD dwLevel = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;

	do
	{
		nStatus = NetUserEnum(wcslen(m_wszDomainName) > 1 ? m_wszDomainName : NULL,
			dwLevel,
			FILTER_NORMAL_ACCOUNT,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (int i = 0; i < dwEntriesRead; i++)
				{
					if (pTmpBuf == NULL)
					{
						break;
					}

					USER_INFO uInfo = { 0 };
					wcscpy(uInfo.wszName, pTmpBuf->usri0_name);
					if (GetUserInfo(uInfo.dwUserID, uInfo.dwGroupID, uInfo.wszName))
					{
						userList.push_back(uInfo);
					}

					pTmpBuf++;
				}
			}
		}

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	} while (nStatus == ERROR_MORE_DATA);

	return bRet;
}

BOOL CUserInfo::GetUserInfo(DWORD &dwUid, DWORD &dwGid, const wchar_t * pName)
{
	BOOL bRet = FALSE;

	wchar_t wszUserName[200] = { 0 };
	DWORD dwUserNameLen = 200;

	LPUSER_INFO_3 lpUserInfo3 = NULL;

	if (NULL == pName)
	{
		GetUserNameW(wszUserName, &dwUserNameLen);
	}
	else
	{
		wcscpy(wszUserName, pName);
	}

	NET_API_STATUS ntStatus = NetUserGetInfo(wcslen(m_wszDomainName) > 1 ? m_wszDomainName : NULL,
		wszUserName, 3, (LPBYTE*)&lpUserInfo3);
	if (NERR_Success == ntStatus)
	{
		dwUid = lpUserInfo3->usri3_user_id;
		dwGid = lpUserInfo3->usri3_primary_group_id;

		NetApiBufferFree(lpUserInfo3);
		lpUserInfo3 = NULL;

		bRet = TRUE;
	}

	return bRet;
}

BOOL CUserInfo::GetUserPrivilege(DWORD &dwPri,const wchar_t * pName)
{
	BOOL bRet = FALSE;

	wchar_t wszUserName[200] = { 0 };
	DWORD dwUserNameLen = 200;

	LPUSER_INFO_3 lpUserInfo3 = NULL;

	if (NULL == pName)
	{
		GetUserNameW(wszUserName, &dwUserNameLen);
	}
	else
	{
		wcscpy(wszUserName, pName);
	}

	NET_API_STATUS ntStatus = NetUserGetInfo(wcslen(m_wszDomainName) > 1 ? m_wszDomainName : NULL,
		wszUserName, 3, (LPBYTE*)&lpUserInfo3);
	if (NERR_Success == ntStatus)
	{		
		dwPri = lpUserInfo3->usri3_priv;

		NetApiBufferFree(lpUserInfo3);
		lpUserInfo3 = NULL;

		bRet = TRUE;
	}

	return bRet;
}

BOOL CUserInfo::GetUserLastLogon(SYSTEMTIME &locTime, const wchar_t * pName)
{
	BOOL bRet = FALSE;

	DWORD dwLastTime = 0;
	struct tm * timeinfo;

	wchar_t wszUserName[200] = { 0 };
	DWORD dwUserNameLen = 200;

	LPUSER_INFO_3 lpUserInfo3 = NULL;

	if (NULL == pName)
	{
		GetUserNameW(wszUserName, &dwUserNameLen);
	}
	else
	{
		wcscpy(wszUserName, pName);
	}

	NET_API_STATUS ntStatus = NetUserGetInfo(wcslen(m_wszDomainName) > 1 ? m_wszDomainName : NULL,
		wszUserName, 3, (LPBYTE*)&lpUserInfo3);
	if (NERR_Success == ntStatus)
	{
		dwLastTime = lpUserInfo3->usri3_last_logon;

		NetApiBufferFree(lpUserInfo3);
		lpUserInfo3 = NULL;

		bRet = TRUE;
	}
	
	time_t time_sec = dwLastTime;
	timeinfo = localtime(&time_sec);
	if (NULL != timeinfo)
	{
		locTime.wYear = timeinfo->tm_year + 1900;
		locTime.wMonth = timeinfo->tm_mon + 1;
		locTime.wDay = timeinfo->tm_mday;

		locTime.wHour = timeinfo->tm_hour;
		locTime.wMinute = timeinfo->tm_min;
		locTime.wSecond = timeinfo->tm_sec;

		locTime.wDayOfWeek = timeinfo->tm_wday;		
	}

	return bRet;
}
