
#include "stdafx.h"
#include "UserInfo_main.h"

/*
	//查看所有用户.
	WinQ.exe -user -u

	//查看指定用户的权限和最后登录时间.
	WinQ.exe -user -q <username>
*/
int main_user(int argc, char **argv)
{
	if (0 == strcmp("-u",argv[2]))
	{
		Show_users();
	}
	else if (0 == strcmp("-q",argv[2]))
	{
		Show_user_detail(argv[3]);
	}

	return 0;
}

void Show_users()
{
	ARRAY_USER userList;

	CUserInfo ui;

	ui.GetUsersList(userList);

	ARRAY_USER::iterator it;
	for (it = userList.begin(); it != userList.end(); it++)
	{
		wprintf(L"%s(<uid>%d:<gid>%d)\r\n", (*it).wszName, (*it).dwUserID, (*it).dwGroupID);
	}
}

void Show_user_detail(const char * pwszUserName)
{
	CUserInfo ui;

	DWORD dwPrivilege = 0;
	SYSTEMTIME sysTime = { 0 };

	ui.GetUserPrivilege(dwPrivilege);
	ui.GetUserLastLogon(sysTime);

	if (USER_PRIV_GUEST == dwPrivilege)
	{
		printf("GUEST user\r\n");
	}
	else if (USER_PRIV_USER == dwPrivilege)
	{
		printf("USER user\r\n");
	}
	else if (USER_PRIV_ADMIN == dwPrivilege)
	{
		printf("ADMIN user\r\n");
	}

	printf("%d\r\n",dwPrivilege);

	printf("%d-%02d-%02d %02d:%02d:%02d\r\n", sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
}
