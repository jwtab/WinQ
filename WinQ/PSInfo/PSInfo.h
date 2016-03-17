
#ifndef PS_INFO_H_
#define PS_INFO_H_

#include <Windows.h>
#include <tlhelp32.h>
#include <Psapi.h>

#include <stdio.h>

#include <vector>
#include <string>
using namespace std;

#pragma comment(lib,"Psapi.lib")

//操作系统信息.
typedef struct Os_Info
{
	wchar_t wszOsVersion[128];

	//是不是x64系统，是不是正版系统.
	BOOL bIsX64OS;
	BOOL bIsRegister;

	//内存和硬盘大小.
	unsigned long ulMemorySize;
	unsigned long ulDiskSize;
	wchar_t wszDiskSerialNumber[128];

	//ip地址
	wchar_t wszIpAddr[128];
	BOOL bIsIPV4;
}OS_INFO,*LPOS_INFO;

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


//
//UNICODE_STRING结构定义
typedef struct
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
}UNICODE_STRING, *PUNICODE_STRING;

//进程参数结构定义,必有和NativeAPI规范相符和
typedef struct
{
	ULONG AllocationSize;
	ULONG ActualSize;
	ULONG Flags;
	ULONG Unknown1;
	UNICODE_STRING Unknown2;
	HANDLE InputHandle;
	HANDLE OutputHandle;
	HANDLE ErrorHandle;
	UNICODE_STRING CurrentDirectory;
	HANDLE CurrentDirectoryHandle;
	UNICODE_STRING SearchPaths;
	UNICODE_STRING ApplicationName;
	UNICODE_STRING CommandLine;
	PVOID EnvironmentBlock;
	ULONG Unknown[9];
	UNICODE_STRING Unknown3;
	UNICODE_STRING Unknown4;
	UNICODE_STRING Unknown5;
	UNICODE_STRING Unknown6;
}PROCESS_PARAMETERS, *PPROCESS_PARAMETERS;

//PEB: Process Environment Block, 进程环境变量块
typedef struct
{
	ULONG AllocationSize;
	ULONG Unknown1;
	HINSTANCE ProcessHinstance;
	PVOID ListDlls;
	PPROCESS_PARAMETERS ProcessParameters;
	ULONG Unknown2;
	HANDLE Heap;
}PEB, *PPEB;

//进程基本信息结构定义
typedef struct
{
	DWORD ExitStatus;
	PPEB PebBaseAddress;
	DWORD AffinityMask;
	DWORD BasePriority;
	ULONG UniqueProcessId;
	ULONG InheritedFromUniqueProcessId;
}PROCESS_BASIC_INFORMATION;

typedef NTSTATUS ( WINAPI *FUN_NtQueryInformationProcess)(
	__in       HANDLE ProcessHandle,
	__in       UINT  ProcessInformationClass,
	__out      PVOID ProcessInformation,
	__in       ULONG ProcessInformationLength,
	__out_opt  PULONG ReturnLength
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

public:
	BOOL GetOSInfo(LPOS_INFO lpOSInfo);
	BOOL GetProcList(ARRAY_PROC & procList);
	BOOL GetProcPath(unsigned long ulPid, wchar_t * pwszExePath);
	BOOL GetProcCmdLine(unsigned long ulPid, wchar_t * pwszCmdLine,unsigned long dwBufLen);

private:
	BOOL m_bIsHaveDebug;
};

#endif

