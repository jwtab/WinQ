
#ifndef MOD_INFO_H_
#define MOD_INFO_H_

#include <Windows.h>
#include <Wincrypt.h>
#include <stdio.h>

#pragma comment(lib,"Version.lib")
#pragma comment(lib,"Crypt32.lib")

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

typedef struct Translate
{
	WORD wLanguage;
	WORD wCodePage;
}TRANSLATE, *LPTRANSLATE;

//版本信息.
typedef struct Mod_Ver_Info
{
	//文件版本
	wchar_t wszFileVer[128];

	//文件说明
	wchar_t wszFileDesc[256];

	//产品版本
	wchar_t wszProductVer[128];

	//产品名称
	wchar_t wszProductName[128];

	//版权说明
	wchar_t wszCopyrightNotice[256];
}MOD_VER_INFO,*LPMOD_VER_INFO;

//签名信息.
typedef struct Mod_Sign_Info
{
	//颁发给(使用者)
	wchar_t wszUser[128];

	//颁发者
	wchar_t wszIssuer[128];

	//证书开始时间
	SYSTEMTIME timeStart;

	//证书结束时间
	SYSTEMTIME timeEnd;

	//证书序列号
	wchar_t wszSerialNumber[128];
}MOD_SIGN_INFO,*LPMOD_SIGN_INFO;


BOOL GetModVerInfo(const char * pszModFilePath, MOD_VER_INFO & verInfo);
BOOL GetModSignInfo(const char * pszModFilePath, MOD_SIGN_INFO & signInfo);

BOOL mod_GetModSignInfo(PCCERT_CONTEXT pCertContext, MOD_SIGN_INFO &signInfo);

BOOL mod_GetModFileVer(const wchar_t * pVerData,wchar_t * pwszFileVer);
BOOL mod_GetModProductVer(const wchar_t * pVerData, wchar_t * pwszFileProductVer);
BOOL mod_GetModNameValue(const wchar_t * pVerData, const wchar_t * pwszName,wchar_t * pwszNameValue);

#endif

