
#include "stdafx.h"

#include "ModInfo_main.h"

int main_mod(int argc, char **argv)
{
	if (0 == strcmp("-b",argv[2]))
	{
		mod_Show_Base(argv[3]);
	}
	else if (0 == strcmp("-s", argv[2]))
	{
		mod_Show_Sign(argv[3]);
	}

	return 0;
}

void mod_Show_Base(const char * pszModPath)
{
	MOD_VER_INFO verInfo = { 0 };

	GetModVerInfo(pszModPath, verInfo);

	wprintf(L"VER -> %s \r\nP_VER -> %s\r\nP_NAME -> %s\r\nF_D -> %s\r\nC_N -> %s\r\n", 
		verInfo.wszFileVer,
		verInfo.wszProductVer,
		verInfo.wszProductName,
		verInfo.wszFileDesc,
		verInfo.wszCopyrightNotice);
}

void mod_Show_Sign(const char * pszModPath)
{
	MOD_SIGN_INFO  signInfo = { 0 };

	GetModSignInfo(pszModPath, signInfo);

	wprintf(L"User:%s\r\nIssuer:%s\r\nSn:%s\r\n",
		signInfo.wszUser,
		signInfo.wszIssuer,
		signInfo.wszSerialNumber);
}
