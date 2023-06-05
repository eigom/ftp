#include "ftps.h"

int newsockfd;
sockaddr client_data_addr;

int do_ftp_server(int port)
{
   int sockfd;
   int len;
   sockaddr server;
   sockaddr client;
   /*static struct sigaction act;
   act.sa_handler = catcher;
   sigfillset(&(act.sa_mask));
   sigaction(SIGPIPE, &act, NULL);*/

   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   /*server.sin_addr.s_addr = inet_addr("0.0.0.0");*/
   
   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("socket");
      exit(EXIT_FAILURE);
   }
   if(!inet_aton("0.0.0.0", &server.sin_addr))
   {
      perror("bad address");
      exit(EXIT_FAILURE);
   }
   if(bind(sockfd, (sockaddr*)&server, sizeof(server)) == -1)
   {
      perror("bind");
      exit(EXIT_FAILURE);
   }
   if(listen(sockfd, 5) == -1)
   {
      perror("listen");
      exit(EXIT_FAILURE);
   }
   
   printf("listening on port %d\n", port);
   
   while(1)
   {
      len = sizeof(client);
      if((newsockfd = accept(sockfd, (sockaddr*)&client, &len)) == -1)
      {
         perror("accept error");
         exit(EXIT_FAILURE);
      }
      printf("Connected\n");
      if(fork() == 0)
      {
         handle_client(newsockfd);
         printf("Connection lost\n");
         close(newsockfd);
         exit(EXIT_SUCCESS);
      }
      close(newsockfd); /* in parent */
   }
}

command* parse_cmd(char* cmd_line)
{
   static command cmd;
   
   cmd.argc = 0;
   cmd.cmd = CMD_NONE;
   cmd.args[0][0] = cmd.args[1][0] = cmd.args[2][0] = '\0';
   
   if(cmd_line == NULL)
   {
      cmd.cmd = CMD_QUIT;
      return &cmd;
   }
   
   sscanf(cmd_line, "%s %s", cmd.args[0], cmd.args[1]);
   if(strlen(cmd.args[0])) cmd.argc++;
   if(strlen(cmd.args[1])) cmd.argc++;
   /*printf("<%s %s>\n", cmd.args[0], cmd.args[1]);*/
   
   if(!cmd.argc) return &cmd; 
  
   if(!strcasecmp(cmd.args[0], "list")){
      cmd.cmd = CMD_LS;
   }
   else if(!strcasecmp(cmd.args[0], "cwd")){
      cmd.cmd = CMD_CD;
   }
   else if(!strcasecmp(cmd.args[0], "retr")){
      cmd.cmd = CMD_GET;
   }
   else if(!strcasecmp(cmd.args[0], "stor")){
      cmd.cmd = CMD_PUT;
   }
   else if(!strcasecmp(cmd.args[0], "pwd")){
      cmd.cmd = CMD_PWD;
   }
   else if(!strcasecmp(cmd.args[0], "type")){
      cmd.cmd = CMD_TYPE;
   }
   else if(!strcasecmp(cmd.args[0], "port")){
      cmd.cmd = CMD_PORT;
   }
   else if(!strcasecmp(cmd.args[0], "user")){
      cmd.cmd = CMD_USER;
   }
   else if(!strcasecmp(cmd.args[0], "pass")){
      cmd.cmd = CMD_PASS;
   }
   else if(!strcasecmp(cmd.args[0], "quit")){
      cmd.cmd = CMD_QUIT;
   }
   else{
      printf("<%s> is not a valid command\n", cmd.args[0]);
   }
   
   return &cmd;
}

void handle_client(int sockfd)
{
   char* cmd_str;
   char str[512];
   char tmp[256];
   command* user_cmd;
   int fd;
   
   ftp_write(sockfd, "220 Connected. \nUse PASS with any data to login.\r\n");
   
   while(1)
   {
      if((cmd_str = ftp_read(sockfd)) == NULL) return;
      user_cmd = parse_cmd(cmd_str);
      if(!user_cmd->argc) continue;
      
      switch(user_cmd->cmd)
      {
         case CMD_NONE:
            ftp_write(sockfd, "202 Command not implemented.\r\n");
            break;
           
         case CMD_QUIT:
            return;
         
         case CMD_LS:
            if((can_list("."))) {
               ftp_write(sockfd, "150 Opening data connection\r\n");
               send_list(".");
               ftp_write(sockfd, "226 Transfer complete.\r\n");
            } else {
               sprintf(str, "450 %s: %s\r\n", user_cmd->args[1], strerror(errno));
               ftp_write(sockfd, str);
            } break;
            
         case CMD_GET:
            if((fd = open_file(user_cmd->args[1]))) {
               ftp_write(sockfd, "150 Opening data connection\r\n");
               send_file(fd);
               ftp_write(sockfd, "226 Transfer complete.\r\n");
            } else {
               sprintf(str, "550 %s: %s\r\n", user_cmd->args[1], strerror(errno));
               ftp_write(sockfd, str);
            } break;
         
         case CMD_PUT:
            if((fd = create_file(user_cmd->args[1]))) {
               ftp_write(sockfd, "150 Opening data connection\r\n");
               recv_file(fd);
               ftp_write(sockfd, "226 Transfer complete.\r\n");
            } else {
               sprintf(str, "550 %s: %s\r\n", user_cmd->args[1], strerror(errno));
               ftp_write(sockfd, str);
            } break;
         
         case CMD_PWD:
            if(getcwd(tmp, 256) == NULL){
               sprintf(str, "450 %s: %s\r\n", user_cmd->args[1], strerror(errno));
               ftp_write(sockfd, str);
            } else {
               sprintf(str, "257 %s\r\n", tmp);
               ftp_write(sockfd, str);
            } break;
         
         case CMD_TYPE:
            ftp_write(sockfd,"200 Type set.\r\n");
            break;
         
         case CMD_PORT:
            parse_port(user_cmd->args[1]);
            ftp_write(sockfd, "200 Port ok.\r\n");
            break;
         
         case CMD_CD:
            if(chdir(user_cmd->args[1]) == -1){
               sprintf(str, "550 %s: %s\r\n", user_cmd->args[1], strerror(errno));
               ftp_write(sockfd, str);
            } else {
               getcwd(tmp, 256);
               sprintf(str, "250 Changed to %s\r\n", tmp);
               ftp_write(sockfd, str);
            } break;
            
         case CMD_USER:
            ftp_write(sockfd, "331 Password. Can be anything!\r\n");
            break;
            
         case CMD_PASS:
            ftp_write(sockfd, "230 Logged in.\r\n");
            break;
      }
   }
}

void parse_port(char* str)
{
   unsigned long a0, a1, a2, a3, p0, p1;
 
   sscanf(str, "%lu,%lu,%lu,%lu,%lu,%lu", &a0, &a1, &a2, &a3, &p0, &p1);
   client_data_addr.sin_addr.s_addr = htonl((a0 << 24) + (a1 << 16) + (a2 << 8) + a3);
   client_data_addr.sin_port = htons((p0 << 8) + p1);
   client_data_addr.sin_family = AF_INET;
}

int open_data_connection()
{
   int fd;

   if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("socket");
      exit(EXIT_FAILURE);
   }
   if(connect(fd, (sockaddr*)&client_data_addr, sizeof(sockaddr)) == -1)
   {
      perror("connect");
      exit(EXIT_FAILURE);
   }
   
   return fd;
}

/* check if dir can be opened
  if it is rm'ed/ch*'ed immediately after that and before
  send_list() then we're in error!*/
int can_list(char* fname)
{ 
   DIR* dp;
   
   if((dp = opendir(fname)) == NULL) return 0;
   closedir(dp);
   
   return 1;
}

int send_list(char* fname)
{
   struct dirent* d;
   DIR* dp;
   int sfd;
   
   if((dp = opendir(fname)) == NULL) return 0;
   sfd = open_data_connection();
   
   while((d = readdir(dp)))
      if(d->d_ino != 0)
         stat_file(d->d_name, sfd);
      
   closedir(dp);
   close(sfd);
   
   return 1;
}

int stat_file(char* fname, int sfd)
{
   struct stat s;
   struct tm* time;
   char str[256];
   char perms[11] = {"----------"};
   static const char month[12][4] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
   };
   
   if(lstat(fname, &s) != 0)
   {
      sprintf(str, "%s: %s\r\n", fname, strerror(errno));
      ftp_write(sfd, str);
   }
   
   if(S_ISDIR(s.st_mode))
      perms[0] = 'd';
   else if(S_ISBLK(s.st_mode))
      perms[0] = 'b';
   else if(S_ISCHR(s.st_mode))
      perms[0] = 'c';
   else if(S_ISFIFO(s.st_mode))
      perms[0] = 'p';
   
   if((s.st_mode & S_IRUSR) == S_IRUSR)
      perms[1] = 'r';
   if((s.st_mode & S_IWUSR) == S_IWUSR)
      perms[2] = 'w';
   if((s.st_mode & S_IXUSR) == S_IXUSR) 
      perms[3] = 'x';

   if((s.st_mode & S_IRGRP) == S_IRGRP)
      perms[4] = 'r';
   if((s.st_mode & S_IWGRP) == S_IWGRP)
      perms[5] = 'w';
   if((s.st_mode & S_IXGRP) == S_IXGRP)
      perms[6] = 'x';

   if((s.st_mode & S_IROTH) == S_IROTH)
      perms[7] = 'r';
   if((s.st_mode & S_IWOTH) == S_IWOTH)
      perms[8] = 'w';
   if((s.st_mode & S_IXOTH) == S_IXOTH)
      perms[9] = 'x';
   
   time = gmtime(&s.st_ctime);
   
   sprintf(str, "%s %4d %-10d %-6d %8d %3s %2d %02d:%02d %s\r\n",
   perms, s.st_nlink, s.st_uid, s.st_gid, (int)s.st_size,
   month[time->tm_mon], time->tm_mday, time->tm_hour, time->tm_min, fname);
   
   ftp_write(sfd, str);
   
   return 1;
}

int open_file(char* fname)
{
   int fd;
   
   if((fd = open(fname, O_RDONLY)) == -1) return 0;
   
   return fd;
}

int send_file(int fd)
{
   ssize_t nread;
   char buffer[4096];
   int sfd;
   
   sfd = open_data_connection();
   
   while((nread = read(fd, buffer, 4096)) > 0)
      if(nread == -1) return 0;
      else
         if(write(sfd, buffer, nread) == -1) return 0;
   
   close(fd);
   close(sfd);
   return 1;
}

int create_file(char* fname)
{
   int fd;
   
   if((fd = open(fname, O_WRONLY | O_CREAT)) == -1) return 0;
   
   return fd;
}

int recv_file(int fd)
{
   ssize_t nread;
   char buffer[4096];
   int sfd;
   
   sfd = open_data_connection();
   
   while((nread = read(sfd, buffer, 4096)) > 0)
      if(nread == -1) return 0;
      else
         if(write(fd, buffer, nread) == -1) return 0;
   
   close(fd);
   close(sfd);
   return 1;
}

void catcher(int sig)
{
   close(newsockfd);
   exit(EXIT_SUCCESS);
}
