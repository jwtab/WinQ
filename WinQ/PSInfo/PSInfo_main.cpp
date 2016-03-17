
#include "stdafx.h"
#include "PSInfo_main.h"

int main_ps(int argc, char **argv)
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
		//ps.GetProcCmdLine((*it).ulProcID, wszCmdLine, 1024);

		wprintf(L"%d        %s \r\n", (*it).ulProcID, wszExePath);
	}

	return 0;
}
