#ifndef _FTPC_H_
#define _FTPC_H_

#include "ftp.h"

command* parse_cmd_line(char*);
int do_ftp_client(ftp_conn_data*);
int open_ctrl_connection(ftp_conn_data*);
int close_ctrl_connection(ftp_conn_data*);
int ftp_open_data_connection(ftp_conn_data*);
int ftp_close_data_connection(ftp_conn_data*);
int ftp_wait_data(ftp_conn_data*);
int get_list(ftp_conn_data*, char*);
int ftp_get_file(ftp_conn_data*, char*);
int ftp_put_file(ftp_conn_data*, char*);

#endif
