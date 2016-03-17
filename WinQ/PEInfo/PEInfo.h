
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
	
	//��Rvaת��Ϊ���ļ��е�ƫ��.
	int RvaToOffSet(DWORD dwRva);

	//ȡ��Rva���ڽ���������
	PUCHAR GetSectionNameByRva(DWORD dwRva);

	//��ַ����
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