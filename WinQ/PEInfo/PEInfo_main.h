
#ifndef PE_INFOR_MAIN_H_
#define PE_INFO_MAIN_H_

#include "PEInfo.h"

int main_pe(int argc, char **argv);

void Show_PE_Base(const char * pszFilePath);
void Show_PE_Import(const char * pszFilePath);
void Show_PE_Export(const char * pszFilePath);

#endif
