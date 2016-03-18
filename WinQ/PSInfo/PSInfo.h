
#ifndef PS_INFO_H_
#define PS_INFO_H_

#include <Windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <Psapi.h>

#include <stdio.h>

#include <vector>
#include <string>
using namespace std;

#pragma comment(lib,"Psapi.lib")

//进程信息
typedef struct Proc_Info
{
	//进程pid
	unsigned long ulProcID;

	//父进程id
	unsigned long ulParentID;

	//进程名称
	wchar_t wszProcName[128];
}PROC_INFO,*LPPROC_INFO;
typedef vector<PROC_INFO> ARRAY_PROC;

//模块信息
typedef struct Mod_Info
{
	//模块名称.
	wchar_t wszModFileName[128];

	//模块全路径.
	wchar_t wszModFilePath[MAX_PATH];
}MOD_INFO,*LPMOD_INFO;
typedef vector<MOD_INFO> ARRAY_MOD;

//服务信息
typedef struct Svr_Info
{
	//服务名称.
	wchar_t wszServerName[128];

	//pid
	unsigned long ulPid;

	//服务命令行.
	wchar_t wszServerCmdLine[1024];

	//服务是否在运行.
	unsigned long dwCurrentState;
	unsigned long dwServiceType;
}SVR_INFO,*LPSVR_INFO;
typedef vector<SVR_INFO> ARRAY_SVR;

//动态加载的函数. ntdll.dll
typedef NTSTATUS ( WINAPI *FUN_NtQueryInformationProcess)(
	__in       HANDLE ProcessHandle,
	__in       UINT  ProcessInformationClass,
	__out      PVOID ProcessInformation,
	__in       ULONG ProcessInformationLength,
	__out_opt  PULONG ReturnLength
	);

typedef NTSTATUS (WINAPI *FUN_RtlGetVersion)(
	IN OUT PRTL_OSVERSIONINFOW  lpVersionInformation
	);

class CPSInfo
{
public:
	CPSInfo();
	~CPSInfo();

private:
	BOOL EnableDebug();
	BOOL SetPrivilegeEx(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege);

	void DosPath2Path(wchar_t * pwszFilePath);
	BOOL GetServiceExePath(SC_HANDLE hSCM, const wchar_t * pwszSrvName,wchar_t * pwszSvrFilePath);

public:	
	BOOL GetProcList(ARRAY_PROC & procList);
	BOOL GetProcPath(unsigned long ulPid, wchar_t * pwszExePath);
	BOOL GetProcCmdLine(unsigned long ulPid, wchar_t * pwszCmdLine,unsigned long dwBufLen);

	BOOL GetModList(unsigned long ulPid, ARRAY_MOD &modList);
	BOOL GetSvrlist(ARRAY_SVR &svrList);
private:
	BOOL m_bIsHaveDebug;
};

#endif

