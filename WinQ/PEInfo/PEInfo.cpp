
#include "stdafx.h"
#include "PEInfo.h"

CPEInfo::CPEInfo()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_hMap = NULL;
	m_pFileMap = NULL;
}
CPEInfo::~CPEInfo()
{
	UnMapFile();
}

BOOL CPEInfo::MapFile(const char * pszPath)
{
	BOOL bFileMaped = TRUE;

	//取消先前的文件映射
	UnMapFile();

	//打开文件
	m_hFile = CreateFileA(pszPath, GENERIC_READ,
		FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		bFileMaped = FALSE;		
	}

	//创建文件映射
	m_hMap = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m_hMap == NULL)
	{
		bFileMaped = FALSE;		
	}

	m_pFileMap = (PUCHAR)MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0);
	if (m_pFileMap == NULL)
	{
		bFileMaped = FALSE;		
	}
	
	if (!bFileMaped)
	{
		UnMapFile();
	}
	
	return bFileMaped;
}

PIMAGE_DOS_HEADER CPEInfo::GetDosHeader()
{
	return PIMAGE_DOS_HEADER(m_pFileMap);
}

PIMAGE_NT_HEADERS CPEInfo::GetImageNTHeader()
{
	return (PIMAGE_NT_HEADERS)((char*)m_pFileMap + GetDosHeader()->e_lfanew);;
}

PIMAGE_SECTION_HEADER CPEInfo::GetSectionHeader()
{
	return (PIMAGE_SECTION_HEADER)((char*)m_pFileMap + GetDosHeader()->e_lfanew + sizeof(IMAGE_NT_HEADERS));
}

PCHAR CPEInfo::GetFileMap()
{
	return (PCHAR)m_pFileMap;
}

int CPEInfo::RvaToOffSet(DWORD dwRva)
{
	int dwSectionNum=GetImageNTHeader()->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader=GetSectionHeader();

	for(int i = 0;i < dwSectionNum; i++)
	{
		DWORD dwSectionAlign=Align(pSectionHeader[i].Misc.VirtualSize,GetImageNTHeader()->OptionalHeader.SectionAlignment);
		DWORD dwSectionStart = pSectionHeader[i].VirtualAddress;
		//注意最后要减1，因为dwSectionAlign求得的是内存中整个节区的大小
		//即：dwSectionEnd-dwSectionStart+1=dwSectionAlign
		DWORD dwSectionEnd = dwSectionStart+dwSectionAlign-1;
		if(dwRva>=dwSectionStart && dwRva<dwSectionEnd)//落在此节区内
		{
			DWORD dwOffSet=dwRva-dwSectionStart;
			//如果此RVA是节区在内存中对齐补0的空白数据，
			//那么在文件中应该没有对应的数据
			//属于无效的RVA
			return dwOffSet<pSectionHeader[i].SizeOfRawData?
				pSectionHeader[i].PointerToRawData+dwOffSet : -1; 
		}
	}

	return -1;
}

PUCHAR CPEInfo::GetSectionNameByRva(DWORD dwRva)
{
	int dwSectionNum=GetImageNTHeader()->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader=GetSectionHeader();

	for(int i = 0;i < dwSectionNum; i++)
	{
		DWORD dwSectionAlign=Align(pSectionHeader[i].Misc.VirtualSize,GetImageNTHeader()->OptionalHeader.SectionAlignment);
		DWORD dwSectionStart = pSectionHeader[i].VirtualAddress;

		//注意最后要减1，因为dwSectionAlign求得的是内存中整个节区的大小
		//即：dwSectionEnd-dwSectionStart+1=dwSectionAlign
		DWORD dwSectionEnd = dwSectionStart+dwSectionAlign-1;
		if(dwRva>=dwSectionStart && dwRva<=dwSectionEnd)//落在此节区内
		{
			return (PUCHAR)(pSectionHeader+i);
		}
	}

	return NULL;
}

void CPEInfo::UnMapFile()
{
	if(m_pFileMap) 
	{
		UnmapViewOfFile(m_pFileMap);
		m_pFileMap = NULL;
	}

	if(m_hMap)
	{
		CloseHandle(m_hMap);
		m_hMap = NULL;
	}

	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

DWORD CPEInfo::Align(DWORD dwAddress,DWORD dwAlignNum)
{
	if(dwAddress%dwAlignNum == 0)
	{
		return dwAddress;
	}

	return (dwAddress/dwAlignNum+1)*dwAlignNum;
}
