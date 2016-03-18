
#include "stdafx.h"

#include "PSInfo.h"

CPSInfo::CPSInfo()
{
	if (EnableDebug())
	{
		m_bIsHaveDebug = TRUE;
	}
}

CPSInfo::~CPSInfo()
{

}

BOOL CPSInfo::GetProcList(ARRAY_PROC & procList)
{
	BOOL bRet = TRUE;
	HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32W pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap != INVALID_HANDLE_VALUE)
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32FirstW(hProcessSnap, &pe32))
		{
			CloseHandle(hProcessSnap);
			hProcessSnap = INVALID_HANDLE_VALUE;
		}
		else
		{
			do
			{			
				PROC_INFO node = { 0 };
				node.ulProcID = pe32.th32ProcessID;
				node.ulParentID = pe32.th32ParentProcessID;
				wcscpy(node.wszProcName, pe32.szExeFile);

				procList.push_back(node);
			} while (Process32NextW(hProcessSnap, &pe32));
		}

		if (INVALID_HANDLE_VALUE != hProcessSnap)
		{
			CloseHandle(hProcessSnap);
			hProcessSnap = NULL;
		}
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}

BOOL CPSInfo::GetProcPath(unsigned long ulPid, wchar_t * pwszExePath)
{
	BOOL bRet = TRUE;
	wchar_t wszFilePath[MAX_PATH] = { 0 };

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ulPid);
	if (NULL == hProcess)
	{
		return bRet;
	}
		
	if(!GetProcessImageFileName(hProcess,wszFilePath,MAX_PATH))
	{
		bRet = FALSE;
	}
	else
	{
		wcscpy(pwszExePath, wszFilePath);
		DosPath2Path(pwszExePath);
	}
	
	CloseHandle(hProcess);
	hProcess = NULL;

	return bRet;
}

void CPSInfo::DosPath2Path(wchar_t * pwszFilePath)
{
	wchar_t wszDisk[27] = { L"ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	for (size_t i = 0; i < 27; i++)
	{
		wchar_t wszTemp[4] = { 0 };
		wchar_t wszPath[128] = { 0 };
		swprintf(wszTemp, L"%c:", wszDisk[i]);
		
		if(QueryDosDeviceW(wszTemp, wszPath, 128) > 0)
		{
			wstring wstr = pwszFilePath;
			size_t pos = wstr.find(wszPath);
			if (-1 != pos)
			{
				wsprintf(pwszFilePath, L"%c:%s", wszDisk[i], pwszFilePath + wcslen(wszPath));

				break;
			}
		}		
	}
}

BOOL CPSInfo::GetProcCmdLine(unsigned long ulPid, wchar_t * pwszCmdLine, unsigned long dwBufLen)
{
	BOOL bRet = FALSE;

	NTSTATUS status;
	PROCESS_BASIC_INFORMATION * pbi = NULL;
	PEB peb;
	DWORD dwBytesRead = 0;		
	RTL_USER_PROCESS_PARAMETERS peb_upp;
	HANDLE hHeap = GetProcessHeap();

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, ulPid);
	if (NULL == hProcess)
	{
		return bRet;
	}

	FUN_NtQueryInformationProcess pQueryInfo = (FUN_NtQueryInformationProcess)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), 
		"NtQueryInformationProcess");
	if (NULL != pQueryInfo)
	{
		pbi = (PROCESS_BASIC_INFORMATION*)HeapAlloc(hHeap,HEAP_ZERO_MEMORY, sizeof(PROCESS_BASIC_INFORMATION));
		if (NULL != pbi)
		{			
			status = pQueryInfo(hProcess, ProcessBasicInformation, pbi, sizeof(PROCESS_BASIC_INFORMATION), &dwBytesRead);
		}				
	}
	
	if (status >= 0)
	{
		// (PEB)
		if (pbi->PebBaseAddress)
		{
			if (ReadProcessMemory(hProcess, pbi->PebBaseAddress, &peb, sizeof(peb), &dwBytesRead))
			{
				//dwSessionID = (DWORD)peb.SessionId;
				//cBeingDebugged = (BYTE)peb.BeingDebugged;
				
				dwBytesRead = 0;
				if (ReadProcessMemory(hProcess,peb.ProcessParameters,&peb_upp,sizeof(RTL_USER_PROCESS_PARAMETERS),&dwBytesRead))
				{
					if (peb_upp.CommandLine.Length > 0) 
					{						
						wchar_t *pwszBuffer = (WCHAR *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, peb_upp.CommandLine.Length);												
						if (pwszBuffer)
						{
							if (ReadProcessMemory(hProcess,peb_upp.CommandLine.Buffer,pwszBuffer,
								peb_upp.CommandLine.Length,&dwBytesRead))
							{
								//取固定长度.
								memcpy(pwszCmdLine, pwszBuffer, peb_upp.CommandLine.Length);
							}

							HeapFree(hHeap, 0, pwszBuffer);							
							pwszBuffer = NULL;
						}
					}			
				}
			}
		}
	}

	if (NULL != pbi)
	{
		HeapFree(hHeap, 0, pbi);
		pbi = NULL;
	}

	CloseHandle(hProcess);
	hProcess = NULL;

	return bRet;
}

BOOL CPSInfo::EnableDebug()
{
	BOOL bRet = TRUE;

	HANDLE	hToken = NULL;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES
		| TOKEN_ADJUST_SESSIONID | TOKEN_ADJUST_DEFAULT | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE, &hToken))
	{
		bRet = FALSE;
	}
	else
	{
		SetPrivilegeEx(hToken, SE_DEBUG_NAME, TRUE);
		SetPrivilegeEx(hToken, SE_TCB_NAME, TRUE);
	}

	return bRet;
}

BOOL CPSInfo::SetPrivilegeEx(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;
	TOKEN_PRIVILEGES tpPrevious;
	DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);
	DWORD dwError = 0;

	if (!LookupPrivilegeValue(NULL, Privilege, &luid))
	{
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = 0;

	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrevious, &cbPrevious);
	dwError = GetLastError();
	if (dwError != ERROR_SUCCESS && dwError != ERROR_NOT_ALL_ASSIGNED)
	{
		return FALSE;
	}

	tpPrevious.PrivilegeCount = 1;
	tpPrevious.Privileges[0].Luid = luid;

	if (bEnablePrivilege)
	{
		tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
	}
	else
	{
		tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);
	}

	AdjustTokenPrivileges(hToken, FALSE, &tpPrevious, cbPrevious, NULL, NULL);

	dwError = GetLastError();
	if (dwError != ERROR_SUCCESS && dwError != ERROR_NOT_ALL_ASSIGNED)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CPSInfo::GetModList(unsigned long ulPid, ARRAY_MOD &modList)
{
	BOOL bRet = FALSE;

	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
		FALSE, ulPid);
	if (NULL == hProc)
	{
		return bRet;
	}

	//设置默认1024，应该不会超过这个数目吧
	HMODULE hMod[1024] = { 0 };
	DWORD cbNeeded;
	
	//获取进程模块. 
	if (EnumProcessModules(hProc, hMod, sizeof(hMod), &cbNeeded))
	{
		bRet = TRUE;

		//获得模块路径  
		for (int i = 1; i <= cbNeeded / sizeof(HMODULE); i++)
		{
			wchar_t wszModPath[MAX_PATH] = { 0 };
			if (GetModuleFileNameExW(hProc, hMod[i], wszModPath, MAX_PATH))
			{
				MOD_INFO modNode = { 0 };

				wcscpy(modNode.wszModFilePath, wszModPath);

				wchar_t * pwszTail = wcsrchr(wszModPath, L'\\');
				if (NULL != pwszTail)
				{
					pwszTail++;

					wcscpy(modNode.wszModFileName, pwszTail);
				}

				modList.push_back(modNode);
			}
		}
	}

	CloseHandle(hProc);
	hProc = NULL;

	return bRet;
}

BOOL CPSInfo::GetSvrlist(ARRAY_SVR &svrList)
{
	BOOL bRet = FALSE;

	SC_HANDLE hSCM = NULL;
	char *pBuf = NULL;                  // 缓冲区指针  
	DWORD dwBufSize = 0;                // 传入的缓冲长度  
	DWORD dwBufNeed = 0;                // 需要的缓冲长度  
	DWORD dwNumberOfService = 0;        // 返回的服务个数  
	ENUM_SERVICE_STATUS_PROCESS *pServiceInfo = NULL;   // 服务信息  

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
	if (NULL == hSCM)
	{
		return bRet;
	}

	// 获取需要的缓冲区大小  
	EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
		NULL, dwBufSize, &dwBufNeed, &dwNumberOfService, NULL, NULL);

	//多设置存放1个服务信息的长度  
	dwBufSize = dwBufNeed + sizeof(ENUM_SERVICE_STATUS_PROCESS);
	pBuf = new char[dwBufSize];
	if (NULL != pBuf)
	{
		memset(pBuf, 0, dwBufSize);

		//获取服务信息  
		if (EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
			(LPBYTE)pBuf, dwBufSize, &dwBufNeed, &dwNumberOfService, NULL, NULL))
		{
			pServiceInfo = (LPENUM_SERVICE_STATUS_PROCESS)pBuf;

			//打印取得的服务信息  
			for (unsigned int i = 0; i < dwNumberOfService; i++)
			{
				SVR_INFO svrNode = { 0 };
				svrNode.ulPid = pServiceInfo[i].ServiceStatusProcess.dwProcessId;
				svrNode.dwCurrentState = pServiceInfo[i].ServiceStatusProcess.dwCurrentState;
				svrNode.dwServiceType = pServiceInfo[i].ServiceStatusProcess.dwServiceType;
				wcscpy(svrNode.wszServerName, pServiceInfo[i].lpServiceName);

				//获取服务的全路径+命令行.
				GetServiceExePath(hSCM, svrNode.wszServerName, svrNode.wszServerCmdLine);

				svrList.push_back(svrNode);
			}				
		}

		delete []pBuf;
		pBuf = NULL;
	}
	
	//关闭打开的服务句柄.
	bRet = CloseServiceHandle(hSCM);
	hSCM = NULL;

	return bRet;
}

BOOL CPSInfo::GetServiceExePath(SC_HANDLE hSCM, const wchar_t * pwszSrvName,wchar_t * pwszSvrFilePath)
{
	BOOL bRet = FALSE;

	LPQUERY_SERVICE_CONFIG lpsc = NULL;
	DWORD dwBytesNeeded, cbBufSize, dwError;

	SC_HANDLE hSvc = ::OpenService(hSCM, pwszSrvName, SERVICE_QUERY_CONFIG);
	if (hSvc == NULL)
	{
		return bRet;
	}

	do
	{
		if (!QueryServiceConfig(
			hSvc,
			NULL,
			0,
			&dwBytesNeeded))
		{
			dwError = GetLastError();
			if (ERROR_INSUFFICIENT_BUFFER == dwError)
			{
				cbBufSize = dwBytesNeeded;
				lpsc = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, cbBufSize);
			}
			else
			{
				break;
			}
		}

		if (!QueryServiceConfig(
			hSvc,
			lpsc,
			cbBufSize,
			&dwBytesNeeded))
		{
			break;
		}

		wcscpy(pwszSvrFilePath, lpsc->lpBinaryPathName);

		LocalFree(lpsc);
		lpsc = NULL;

		bRet = TRUE;
	} while (false);

	CloseServiceHandle(hSvc);
	hSvc = NULL;	

	return bRet;
}
