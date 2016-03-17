// WinQ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "PEInfo\PEInfo_main.h"
#include "PSInfo\PSInfo_main.h"

int main(int argc,char **argv)
{
	printf("Hello world!\n");

	if (0 == strcmp("-pe",argv[1]))
	{
		main_pe(argc, argv);
	}
	else if (0 == strcmp("-ps",argv[1]))
	{
		main_ps(argc, argv);
	}

	getchar();

    return 0;
}

