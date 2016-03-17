
#include "stdafx.h"
#include "PSInfo_main.h"

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
