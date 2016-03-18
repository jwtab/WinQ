
#ifndef PS_INFO_MAIN_H_
#define PS_INFO_MAIN_H_

#include "PSInfo.h"

int main_ps(int argc, char **argv);

void Show_pid_list();
void Show_pid_cmdline();

void Show_mod_list(unsigned long pid);

void Show_server_list();

#endif
