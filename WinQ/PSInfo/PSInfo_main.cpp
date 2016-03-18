
#include "stdafx.h"
#include "PSInfo_main.h"

/*
	遍历进程
	WinQ.exe -ps -pid

	//遍历命令行
	WinQ.exe -ps -cmd

	//遍历指定进程的模块
	WinQ.exe -ps -mod <pid>

	//遍历服务
	WinQ.exe -ps -svr 
*/
int main_ps(int argc, char **argv)
{
	if (0 == strcmp("-pid",argv[2]))
	{
		Show_pid_list();
	}
	else if (0 == strcmp("-cmd", argv[2]))
	{
		Show_pid_cmdline();
	}
	else if (0 == strcmp("-mod",argv[2]))
	{
		Show_mod_list(atoi(argv[3]));
	}
	else if (0 == strcmp("-svr",argv[2]))
	{
		Show_server_list();
	}

	return 0;
}

void Show_pid_list()
{
	CPSInfo ps;

	ARRAY_PROC procList;

	ps.GetProcList(procList);

	ARRAY_PROC::iterator it;
	for (it = procList.begin(); it != procList.end(); it++)
	{			
		wprintf(L"%d        %s \r\n", (*it).ulProcID, (*it).wszProcName);	
	}
}

void Show_pid_cmdline()
{
	CPSInfo ps;

	ARRAY_PROC procList;

	ps.GetProcList(procList);

	ARRAY_PROC::iterator it;
	for (it = procList.begin(); it != procList.end(); it++)
	{
		wchar_t wszExePath[MAX_PATH] = { 0 };
		wchar_t wszCmdLine[1024] = { 0 };

		ps.GetProcPath((*it).ulProcID, wszExePath);
		ps.GetProcCmdLine((*it).ulProcID, wszCmdLine, 1024);

		wprintf(L"%d        %s    \r\n", (*it).ulProcID, wszExePath);		
		wprintf(L"%s \r\n", wszCmdLine);
	}
}

void Show_mod_list(unsigned long pid)
{
	CPSInfo ps;

	ARRAY_MOD modList;
	ps.GetModList(pid, modList);

	ARRAY_MOD::iterator it;
	for (it = modList.begin(); it != modList.end(); it++)
	{
		wprintf(L"%s     %s\r\n", (*it).wszModFileName, (*it).wszModFilePath);
	}
}

void Show_server_list()
{
	CPSInfo ps;

	ARRAY_SVR svrList;
	ps.GetSvrlist(svrList);

	ARRAY_SVR::iterator it;
	for (it = svrList.begin(); it != svrList.end(); it++)
	{
		wprintf(L"%s     %s\r\n", (*it).wszServerName, (*it).wszServerCmdLine);
	}
}
