
#ifndef USER_INFO_H_
#define USER_INFO_H_

#include <stdio.h>
#include <time.h>

#include <Windows.h>

#include <LM.h>
#pragma comment(lib,"Netapi32.lib")

#include <vector>
#include <string>
using namespace std;

typedef struct User_Info
{
	wchar_t wszName[100];
	DWORD   dwUserID;
	DWORD   dwGroupID;
}USER_INFO, *LPUSER_INFO;
typedef vector<USER_INFO> ARRAY_USER;

class CUserInfo
{
public:
	CUserInfo(const wchar_t * pDomainName = NULL);
	~CUserInfo();

public:
	BOOL GetUsersList(ARRAY_USER &userList);
	
	BOOL GetUserPrivilege(DWORD &dwPri,const wchar_t * pName = NULL);
	BOOL GetUserLastLogon(SYSTEMTIME &locTime, const wchar_t * pName = NULL);

private:
	BOOL GetUserInfo(DWORD &dwUid, DWORD &dwGid, const wchar_t * pName = NULL);

	wchar_t m_wszDomainName[MAX_PATH];
};

#endif

