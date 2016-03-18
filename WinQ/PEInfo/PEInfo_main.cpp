
#include "stdafx.h"

#include "PEInfo_main.h"

/*
	����PE������Ϣ;
	WinQ.exe -pe -b <mz_file_path>

	����PE���뺯��;
	WinQ.exe -pe -i <mz_file_path>

	����PE��������.
	WinQ.exe -pe -o <mz_file_path>
*/
int main_pe(int argc, char **argv)
{
	if (0 == strcmp("-b",argv[2]))
	{
		Show_PE_Base(argv[3]);
	}
	else if (0 == strcmp("-i", argv[2]))
	{
		Show_PE_Import(argv[3]);
	}
	else if (0 == strcmp("-o", argv[2]))
	{
		Show_PE_Export(argv[3]);
	}	

	return 0;
}

void Show_PE_Base(const char * pszFilePath)
{
	CPEInfo pe;

	if (!pe.MapFile(pszFilePath))
	{
		printf("���ļ�ʧ��!");

		return;
	}

	if (pe.GetDosHeader()->e_magic != IMAGE_DOS_SIGNATURE ||
		pe.GetImageNTHeader()->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("�ļ���ʽ���ǿ�ִ���ļ�!");

		return;
	}

	printf("\r\n"
		"--------------------------------------------------------------------\r\n"
		"����ƽ̨��    0x%04x\r\n"
		"����������    %d\r\n"
		"�ļ���ǣ�    0x%04x\r\n"
		"����װ���ַ��0x%08x\r\n"
		"�ļ������С��0x%08x\r\n"
		"�ڴ�Դ�С��0x%08x\r\n"
		"--------------------------------------------------------------------\r\n",
		pe.GetImageNTHeader()->FileHeader.Machine,
		pe.GetImageNTHeader()->FileHeader.NumberOfSections,
		pe.GetImageNTHeader()->FileHeader.Characteristics,
		pe.GetImageNTHeader()->OptionalHeader.ImageBase,
		pe.GetImageNTHeader()->OptionalHeader.FileAlignment,
		pe.GetImageNTHeader()->OptionalHeader.SectionAlignment);

	printf("\r\n-----------------------------------------------------------------------------"
		"\r\n��������   �����ַ   ������С   ������ַ   Raw_ƫ��   Raw_�ߴ�   ��������\r\n");
	
	int nSectionNum = pe.GetImageNTHeader()->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader = pe.GetSectionHeader();
	for (int i = 0; i < nSectionNum; i++)
	{
		char szName[12] = { 0 };
		memcpy(szName, (char*)pSectionHeader[i].Name, IMAGE_SIZEOF_SHORT_NAME);//������������0��β�ַ�����Ҫ���д���
		DWORD dwSectionAlign = pe.Align(pSectionHeader[i].Misc.VirtualSize, pe.GetImageNTHeader()->OptionalHeader.SectionAlignment);
		DWORD dwRvaEnd = pSectionHeader[i].VirtualAddress + dwSectionAlign - 1;

		printf("%8s  0x%08X  0x%08X  0x%08X  0x%08X  0x%08X\r\n",
			szName,
			pSectionHeader[i].VirtualAddress,		//���ڴ��е�RVA
			pSectionHeader[i].Misc.VirtualSize,		//���ڴ���δ�����ʵ�ʴ�С 
			dwRvaEnd,
			pSectionHeader[i].PointerToRawData,		//���ļ��е�ƫ��
			pSectionHeader[i].SizeOfRawData,		//���ļ��ж����Ĵ�С
			pSectionHeader[i].Characteristics		//��������
			);
	}

	printf("-----------------------------------------------------------------------------\r\n");
}

void Show_PE_Import(const char * pszFilePath)
{
	CPEInfo pe;
	PCHAR pFile = NULL;

	if (!pe.MapFile(pszFilePath))
	{
		printf("���ļ�ʧ��!");

		return;
	}

	if (pe.GetDosHeader()->e_magic != IMAGE_DOS_SIGNATURE ||
		pe.GetImageNTHeader()->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("�ļ���ʽ���ǿ�ִ���ļ�!");

		return;
	}

	//������RVA
	PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)pe.GetImageNTHeader()->OptionalHeader.DataDirectory[1].VirtualAddress;

	if (NULL == pImportDescriptor)
	{
		printf("�ļ�û�е����.");	
		return;
	}

	printf("\r\n����������Ľڣ�%s\r\n", pe.GetSectionNameByRva((DWORD)pImportDescriptor));
	
	//RVAת��Ϊ���ļ��е�offset
	int nOffSet = pe.RvaToOffSet((DWORD)pImportDescriptor);
	if (nOffSet<0)
	{
		printf("��Ч�ĵ����RVA��%08X", (DWORD)pImportDescriptor);		
		return;
	}

	pFile = pe.GetFileMap();
	pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(pFile + nOffSet);	

	int i = 0;
	while (pImportDescriptor[i].Characteristics || pImportDescriptor[i].FirstThunk ||
		pImportDescriptor[i].ForwarderChain || pImportDescriptor[i].Name ||
		pImportDescriptor[i].OriginalFirstThunk || pImportDescriptor[i].TimeDateStamp)
	{
		printf("\r\n"
			"----------------------------------------------------------------\r\n"
			"��������ƣ�%s\r\n"			
			"OriginalFirstThunk: %08X\r\n"			
			"FirstThunk: %08X\r\n"
			"-----------------------------------------------------------------\r\n"
			"            �������    ���뺯������          ��ַ                 \r\n"
			"-----------------------------------------------------------------\r\n",
			pFile + pe.RvaToOffSet(pImportDescriptor[i].Name),
			pImportDescriptor[i].OriginalFirstThunk,		
			pImportDescriptor[i].FirstThunk);
		
		//ȡ��RVA
		PIMAGE_THUNK_DATA pImageThunkdata = (PIMAGE_THUNK_DATA)pImportDescriptor[i].OriginalFirstThunk;
		if (NULL == pImageThunkdata)
		{
			pImageThunkdata = (PIMAGE_THUNK_DATA)pImportDescriptor[i].FirstThunk;
		}

		//��RVA���ļ��е�λ��
		pImageThunkdata = (PIMAGE_THUNK_DATA)(pFile + pe.RvaToOffSet((DWORD)pImageThunkdata));
		int j = 0;
		while (pImageThunkdata[j].u1.Ordinal)
		{			
			//����ŷ�ʽ����
			if (pImageThunkdata[j].u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				printf("            %8u    (����ŵ���)    0x%08x\r\n", pImageThunkdata[j].u1.Ordinal & 0x0FFFF,
					pImageThunkdata[j].u1.Function);				
			}
			else//�����Ƶ���
			{
				PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)(pFile + pe.RvaToOffSet(pImageThunkdata[j].u1.Ordinal));
				printf("            %8u    %s      \r\n", pImportByName->Hint, pImportByName->Name);
			}
			
			j++;
		}

		i++;
	}
}

void Show_PE_Export(const char * pszFilePath)
{
	CPEInfo pe;

	if (!pe.MapFile(pszFilePath))
	{
		printf("���ļ�ʧ��!");

		return;
	}

	if (pe.GetDosHeader()->e_magic != IMAGE_DOS_SIGNATURE ||
		pe.GetImageNTHeader()->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("�ļ���ʽ���ǿ�ִ���ļ�!");

		return;
	}

	//������RVA
	PIMAGE_EXPORT_DIRECTORY pImageExport = (PIMAGE_EXPORT_DIRECTORY)pe.GetImageNTHeader()->OptionalHeader.DataDirectory[0].VirtualAddress;
	if (NULL == pImageExport)
	{
		printf("�ļ�û�е�����!\r\n");
		
		return;
	}

	printf("-----------------------------------------------------------\r\n"
		"�����������Ľڣ�%s\r\n",
		pe.GetSectionNameByRva((DWORD)pImageExport));	

	//תRVAΪ�ļ���λ��
	int nOffSet = pe.RvaToOffSet((DWORD)pImageExport);
	if (nOffSet < 0)
	{
		printf("��Ч�ĵ�����RVA��%08X", (DWORD)pImageExport);
		
		return;
	}

	pImageExport = (PIMAGE_EXPORT_DIRECTORY)(pe.GetFileMap() + nOffSet);

	printf("\r\n"
		"-----------------------------------------------------------------\r\n"
		"ԭʼ�ļ�����            %s\r\n"
		"nBase:                  0x%08X\r\n"
		"NumberOfFunctions:      0x%08X\r\n"
		"NumberOfNames:          0x%08X\r\n"
		"AddressOfFunctions:     0x%08X\r\n"
		"AddressOfNames:         0x%08X\r\n"
		"AddressOfNamesOrd:      0x%08X\r\n"
		"------------------------------------------------------------------\r\n"
		"            �������        �����ַ        ������������\r\n"
		"------------------------------------------------------------------\r\n",
		pe.GetFileMap() + pe.RvaToOffSet(pImageExport->Name),
		pImageExport->Base,
		pImageExport->NumberOfFunctions,
		pImageExport->NumberOfNames,
		pImageExport->AddressOfFunctions,
		pImageExport->AddressOfNames,
		pImageExport->AddressOfNameOrdinals);	

	//�����Ƶ����ĺ�������ֵ����
	WORD * pNameOrdinals = (WORD*)(pe.GetFileMap() + pe.RvaToOffSet(pImageExport->AddressOfNameOrdinals));
	for (int i = 0; i < pImageExport->NumberOfFunctions; i++)
	{
		BOOL bExportByName = FALSE;		
		char * strName = NULL;

		//������ڵ�ַ������
		DWORD *pFunctionAddress = (DWORD*)(pe.GetFileMap() + pe.RvaToOffSet(pImageExport->AddressOfFunctions));
		for (DWORD j = 0; j < pImageExport->NumberOfNames; j++)
		{
			//�����Ƶ�����
			if (i == pNameOrdinals[j])
			{
				bExportByName = TRUE;
				//��������ַ������
				DWORD* pNamesAddress = (DWORD*)(pe.GetFileMap() + pe.RvaToOffSet(pImageExport->AddressOfNames));
				strName = pe.GetFileMap() + pe.RvaToOffSet(pNamesAddress[j]);
				break;
			}
		}

		printf("            %8u        0x%08X      %s\r\n",
			i + pImageExport->Base,
			pFunctionAddress[i],
			strName);		
	}
}