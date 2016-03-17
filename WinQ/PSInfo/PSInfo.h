
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

//����ϵͳ��Ϣ.
typedef struct Os_Info
{
	wchar_t wszOsVersion[128];

	//�ǲ���x64ϵͳ���ǲ�������ϵͳ.
	BOOL bIsX64OS;
	BOOL bIsRegister;

	//�ڴ��Ӳ�̴�С.
	unsigned long ulMemorySize;
	unsigned long ulDiskSize;
	wchar_t wszDiskSerialNumber[128];

	//ip��ַ
	wchar_t wszIpAddr[128];
	BOOL bIsIPV4;
}OS_INFO,*LPOS_INFO;

//������Ϣ
typedef struct Proc_Info
{
	//����pid
	unsigned long ulProcID;

	//������id
	unsigned long ulParentID;

	//��������
	wchar_t wszProcName[128];
}PROC_INFO,*LPPROC_INFO;
typedef vector<PROC_INFO> ARRAY_PROC;


//
//UNICODE_STRING�ṹ����
typedef struct
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
}UNICODE_STRING, *PUNICODE_STRING;

//���̲����ṹ����,���к�NativeAPI�淶�����
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

//PEB: Process Environment Block, ���̻���������
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

//���̻�����Ϣ�ṹ����
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

