
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
	BOOL bRet = TRUE;
	LONG status;
	PROCESS_BASIC_INFORMATION pbi;
	PEB Peb;
	PROCESS_PARAMETERS ProcParam;
	DWORD dwDummy;
	DWORD dwSize;
	LPVOID IpAddress;

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, ulPid);
	if (NULL == hProcess)
	{
		return bRet;
	}

	FUN_NtQueryInformationProcess pQueryInfo = (FUN_NtQueryInformationProcess)GetProcAddress(GetModuleHandleW(L"ntdll"), 
		"NtQueryInformationProcess");
	if (NULL != pQueryInfo)
	{
		//查询进程的基本信息,获取PEB
		status = pQueryInfo(hProcess, 0, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
		
		//读取进程的PEB指针
		if (ReadProcessMemory(hProcess, pbi.PebBaseAddress, &Peb, sizeof(PEB), &dwDummy))
		{
			//获取目标进程空间存储的命令行参数字符串的指针
			if (ReadProcessMemory(hProcess, Peb.ProcessParameters, &ProcParam, sizeof(PROCESS_PARAMETERS), &dwDummy))
			{
				IpAddress = ProcParam.CommandLine.Buffer;
				dwSize = ProcParam.CommandLine.Length;

				//读取目标进程的命令行参数到本进程的缓冲区
				ReadProcessMemory(hProcess, IpAddress, pwszCmdLine, dwSize, &dwDummy);
			}			
		}	
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

BOOL CPSInfo::GetOSInfo(LPOS_INFO lpOSInfo)
{
	return TRUE;
}

