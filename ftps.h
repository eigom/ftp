#ifndef _FTPS_H_
#define _FTPS_H_

#include "ftp.h"
#include <signal.h>
#include <dirent.h>
#include <time.h>

int do_ftp_server(int);
int open_data_connection();
command* parse_cmd(char*);
void handle_client(int);
void parse_port(char*);
int can_list(char*);
int send_list(char*);
int stat_file(char*, int);
int open_file(char*);
int send_file(int);
int create_file(char*);
int recv_file(int);
void catcher(int);

#endif
