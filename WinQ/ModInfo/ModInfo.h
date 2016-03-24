
#ifndef MOD_INFO_H_
#define MOD_INFO_H_

#include <Windows.h>
#include <stdio.h>

#pragma comment(lib,"Version.lib")

typedef struct Translate
{
	WORD wLanguage;
	WORD wCodePage;
}TRANSLATE, *LPTRANSLATE;

//�汾��Ϣ.
typedef struct Mod_Ver_Info
{
	//�ļ��汾
	wchar_t wszFileVer[128];

	//�ļ�˵��
	wchar_t wszFileDesc[256];

	//��Ʒ�汾
	wchar_t wszProductVer[128];

	//��Ʒ����
	wchar_t wszProductName[128];

	//��Ȩ˵��
	wchar_t wszCopyrightNotice[256];
}MOD_VER_INFO,*LPMOD_VER_INFO;

//ǩ����Ϣ.
typedef struct Mod_Sign_Info
{

}MOD_SIGN_INFO,*LPMOD_SIGN_INFO;


BOOL GetModVerInfo(const char * pszModFilePath, MOD_VER_INFO & verInfo);
BOOL GetModSignInfo(const char * pszModFilePath, MOD_VER_INFO & verInfo);

BOOL mod_GetModFileVer(const wchar_t * pVerData,wchar_t * pwszFileVer);
BOOL mod_GetModProductVer(const wchar_t * pVerData, wchar_t * pwszFileProductVer);
BOOL mod_GetModNameValue(const wchar_t * pVerData, const wchar_t * pwszName,wchar_t * pwszNameValue);

#endif
