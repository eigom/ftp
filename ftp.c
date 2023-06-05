#include "ftp.h"

char* ftp_read(int fd)
{
   static char buffer[1024];
   ssize_t nread;
   
   if((nread = read(fd, buffer, 1024)) == -1)
      return NULL;
   if(!nread)
      return NULL;
      
   buffer[nread] = '\0';
   
   return buffer;
}

ssize_t ftp_write(int fd, char* line)
{
   ssize_t nwrite;
   int len;
   
   if((len = strlen(line)) == 0)
      return 0;
   
   if((nwrite = write(fd, line, len)) == -1)
   {
      perror("error writing to socket");
      exit(EXIT_FAILURE);
   }
   
   return nwrite;
}

int ftp_reply_code(char* reply_str)
{
   int code;
   
   sscanf(reply_str, "%d", &code);
   return code;
}

char* ftp_make_command(char* cmd, char* param)
{
   static char command[1024];
   
   sprintf(command, "%s %s\r\n", cmd, param);
   
   return command;
}

void fatal(const char* msg)
{
   perror(msg);
   exit(EXIT_FAILURE);
}
