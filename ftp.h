#ifndef _FTP_H_
#define _FTP_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

enum status{
   CONNECTED,
   DO_CONNECT,
   DISCONNECTED,
   WAIT_LOGIN,
   LOGIN
};

enum commands{
   CMD_NONE,
   CMD_LS,
   CMD_CD,
   CMD_GET,
   CMD_PUT,
   CMD_PWD,
   CMD_OPEN,
   CMD_TYPE,
   CMD_PORT,
   CMD_USER,
   CMD_PASS,
   CMD_HELP,
   CMD_CLOSE,
   CMD_QUIT
};

typedef struct _command{
   int cmd;
   int argc;
   char args[3][128];
} command;


typedef struct sockaddr_in sockaddr;

typedef struct _ftp_conn_data {
   char hostname[64];
   int port;
   unsigned long localip;
   int status;
   int controlfd;
   int datafd;
   int dataconfd;
} ftp_conn_data;

char* ftp_read(int);
ssize_t ftp_write(int, char*);
int ftp_reply_code(char*);
char* ftp_make_command(char*, char*);

void fatal(const char*);

#endif
