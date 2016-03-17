
#ifndef PE_INFO_H
#define PE_INFO_H

#include <Windows.h>

class CPEInfo
{
public:
	CPEInfo();
	~CPEInfo();

	BOOL MapFile(const char * pszPath);
	PIMAGE_DOS_HEADER GetDosHeader();
	PIMAGE_NT_HEADERS GetImageNTHeader();
	PIMAGE_SECTION_HEADER GetSectionHeader();
	
	//将Rva转换为在文件中的偏移.
	int RvaToOffSet(DWORD dwRva);

	//取得Rva所在节区的名称
	PUCHAR GetSectionNameByRva(DWORD dwRva);

	//地址对齐
	DWORD Align(DWORD dwAddress,DWORD dwAlignNum);

	PCHAR GetFileMap();

private:
	void UnMapFile();

private:
	HANDLE m_hFile;
	HANDLE m_hMap;
	LPVOID m_pFileMap;	
};

#endif