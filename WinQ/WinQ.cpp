// WinQ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "PEInfo\PEInfo_main.h"
#include "PSInfo\PSInfo_main.h"
#include "UserInfo\UserInfo_main.h"
#include "ModInfo\ModInfo_main.h"

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
	else if (0 == strcmp("-user", argv[1]))
	{
		main_user(argc,argv);
	}
	else if(0 == strcmp("-mod",argv[1]))
	{
		main_mod(argc, argv);
	}

	getchar();

    return 0;
}

