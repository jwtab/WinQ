
#include "stdafx.h"

#include "PEInfo_main.h"

/*
	解析PE基本信息;
	WinQ.exe -pe -b <mz_file_path>

	解析PE导入函数;
	WinQ.exe -pe -i <mz_file_path>

	解析PE导出函数.
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
		printf("打开文件失败!");

		return;
	}

	if (pe.GetDosHeader()->e_magic != IMAGE_DOS_SIGNATURE ||
		pe.GetImageNTHeader()->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("文件格式不是可执行文件!");

		return;
	}

	printf("\r\n"
		"--------------------------------------------------------------------\r\n"
		"运行平台：    0x%04x\r\n"
		"节区数量：    %d\r\n"
		"文件标记：    0x%04x\r\n"
		"建议装入地址：0x%08x\r\n"
		"文件对齐大小：0x%08x\r\n"
		"内存对大小：0x%08x\r\n"
		"--------------------------------------------------------------------\r\n",
		pe.GetImageNTHeader()->FileHeader.Machine,
		pe.GetImageNTHeader()->FileHeader.NumberOfSections,
		pe.GetImageNTHeader()->FileHeader.Characteristics,
		pe.GetImageNTHeader()->OptionalHeader.ImageBase,
		pe.GetImageNTHeader()->OptionalHeader.FileAlignment,
		pe.GetImageNTHeader()->OptionalHeader.SectionAlignment);

	printf("\r\n-----------------------------------------------------------------------------"
		"\r\n节区名称   虚拟地址   节区大小   结束地址   Raw_偏移   Raw_尺寸   节区属性\r\n");
	
	int nSectionNum = pe.GetImageNTHeader()->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader = pe.GetSectionHeader();
	for (int i = 0; i < nSectionNum; i++)
	{
		char szName[12] = { 0 };
		memcpy(szName, (char*)pSectionHeader[i].Name, IMAGE_SIZEOF_SHORT_NAME);//节区名不是以0结尾字符串，要自行处理
		DWORD dwSectionAlign = pe.Align(pSectionHeader[i].Misc.VirtualSize, pe.GetImageNTHeader()->OptionalHeader.SectionAlignment);
		DWORD dwRvaEnd = pSectionHeader[i].VirtualAddress + dwSectionAlign - 1;

		printf("%8s  0x%08X  0x%08X  0x%08X  0x%08X  0x%08X\r\n",
			szName,
			pSectionHeader[i].VirtualAddress,		//在内存中的RVA
			pSectionHeader[i].Misc.VirtualSize,		//在内存中未对齐的实际大小 
			dwRvaEnd,
			pSectionHeader[i].PointerToRawData,		//在文件中的偏移
			pSectionHeader[i].SizeOfRawData,		//在文件中对齐后的大小
			pSectionHeader[i].Characteristics		//节区属性
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
		printf("打开文件失败!");

		return;
	}

	if (pe.GetDosHeader()->e_magic != IMAGE_DOS_SIGNATURE ||
		pe.GetImageNTHeader()->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("文件格式不是可执行文件!");

		return;
	}

	//导入表的RVA
	PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)pe.GetImageNTHeader()->OptionalHeader.DataDirectory[1].VirtualAddress;

	if (NULL == pImportDescriptor)
	{
		printf("文件没有导入表.");	
		return;
	}

	printf("\r\n导入表所处的节：%s\r\n", pe.GetSectionNameByRva((DWORD)pImportDescriptor));
	
	//RVA转换为在文件中的offset
	int nOffSet = pe.RvaToOffSet((DWORD)pImportDescriptor);
	if (nOffSet<0)
	{
		printf("无效的导入表RVA：%08X", (DWORD)pImportDescriptor);		
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
			"导入库名称：%s\r\n"			
			"OriginalFirstThunk: %08X\r\n"			
			"FirstThunk: %08X\r\n"
			"-----------------------------------------------------------------\r\n"
			"            导入序号    导入函数名称          地址                 \r\n"
			"-----------------------------------------------------------------\r\n",
			pFile + pe.RvaToOffSet(pImportDescriptor[i].Name),
			pImportDescriptor[i].OriginalFirstThunk,		
			pImportDescriptor[i].FirstThunk);
		
		//取得RVA
		PIMAGE_THUNK_DATA pImageThunkdata = (PIMAGE_THUNK_DATA)pImportDescriptor[i].OriginalFirstThunk;
		if (NULL == pImageThunkdata)
		{
			pImageThunkdata = (PIMAGE_THUNK_DATA)pImportDescriptor[i].FirstThunk;
		}

		//求RVA在文件中的位置
		pImageThunkdata = (PIMAGE_THUNK_DATA)(pFile + pe.RvaToOffSet((DWORD)pImageThunkdata));
		int j = 0;
		while (pImageThunkdata[j].u1.Ordinal)
		{			
			//以序号方式导入
			if (pImageThunkdata[j].u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				printf("            %8u    (按序号导入)    0x%08x\r\n", pImageThunkdata[j].u1.Ordinal & 0x0FFFF,
					pImageThunkdata[j].u1.Function);				
			}
			else//按名称导入
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
		printf("打开文件失败!");

		return;
	}

	if (pe.GetDosHeader()->e_magic != IMAGE_DOS_SIGNATURE ||
		pe.GetImageNTHeader()->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("文件格式不是可执行文件!");

		return;
	}

	//导出表RVA
	PIMAGE_EXPORT_DIRECTORY pImageExport = (PIMAGE_EXPORT_DIRECTORY)pe.GetImageNTHeader()->OptionalHeader.DataDirectory[0].VirtualAddress;
	if (NULL == pImageExport)
	{
		printf("文件没有导出表!\r\n");
		
		return;
	}

	printf("-----------------------------------------------------------\r\n"
		"导出表所处的节：%s\r\n",
		pe.GetSectionNameByRva((DWORD)pImageExport));	

	//转RVA为文件中位置
	int nOffSet = pe.RvaToOffSet((DWORD)pImageExport);
	if (nOffSet < 0)
	{
		printf("无效的导出表RVA：%08X", (DWORD)pImageExport);
		
		return;
	}

	pImageExport = (PIMAGE_EXPORT_DIRECTORY)(pe.GetFileMap() + nOffSet);

	printf("\r\n"
		"-----------------------------------------------------------------\r\n"
		"原始文件名：            %s\r\n"
		"nBase:                  0x%08X\r\n"
		"NumberOfFunctions:      0x%08X\r\n"
		"NumberOfNames:          0x%08X\r\n"
		"AddressOfFunctions:     0x%08X\r\n"
		"AddressOfNames:         0x%08X\r\n"
		"AddressOfNamesOrd:      0x%08X\r\n"
		"------------------------------------------------------------------\r\n"
		"            导出序号        虚拟地址        导出函数名称\r\n"
		"------------------------------------------------------------------\r\n",
		pe.GetFileMap() + pe.RvaToOffSet(pImageExport->Name),
		pImageExport->Base,
		pImageExport->NumberOfFunctions,
		pImageExport->NumberOfNames,
		pImageExport->AddressOfFunctions,
		pImageExport->AddressOfNames,
		pImageExport->AddressOfNameOrdinals);	

	//按名称导出的函数索引值数组
	WORD * pNameOrdinals = (WORD*)(pe.GetFileMap() + pe.RvaToOffSet(pImageExport->AddressOfNameOrdinals));
	for (int i = 0; i < pImageExport->NumberOfFunctions; i++)
	{
		BOOL bExportByName = FALSE;		
		char * strName = NULL;

		//函数入口地址表数组
		DWORD *pFunctionAddress = (DWORD*)(pe.GetFileMap() + pe.RvaToOffSet(pImageExport->AddressOfFunctions));
		for (DWORD j = 0; j < pImageExport->NumberOfNames; j++)
		{
			//按名称导出的
			if (i == pNameOrdinals[j])
			{
				bExportByName = TRUE;
				//函数名地址表数组
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