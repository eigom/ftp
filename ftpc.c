#include "ftpc.h"

void put_promt()
{
   printf("ftp> ");
}

command* parse_cmd_line(char* cmd_line)
{
   static command cmd;
   
   cmd.cmd = CMD_NONE;
   cmd.argc = 0;
   cmd.args[0][0] = cmd.args[1][0] = cmd.args[2][0] = '\0';
   
   sscanf(cmd_line, "%s%s%s", cmd.args[0], cmd.args[1], cmd.args[2]);
   if(strlen(cmd.args[0])) cmd.argc++;
   if(strlen(cmd.args[1])) cmd.argc++;
   if(strlen(cmd.args[2])) cmd.argc++;
   
   if(!cmd.argc) return &cmd; 
  
   if(!strcasecmp(cmd.args[0], "ls")){
      if(cmd.argc == 1) strcpy(cmd.args[1], ".");
      strcpy(cmd.args[0], "list");
      cmd.cmd = CMD_LS;
   }
   else if(!strcasecmp(cmd.args[0], "cd")){
      if(cmd.argc == 1) strcpy(cmd.args[1], "~");
      strcpy(cmd.args[0], "cwd");
      cmd.cmd = CMD_CD;
   }
   else if(!strcasecmp(cmd.args[0], "get")){
      if(cmd.argc != 2) printf(" get filename\n");
      else {
         strcpy(cmd.args[0], "retr");
         cmd.cmd = CMD_GET; }
   }
   else if(!strcasecmp(cmd.args[0], "put")){
      if(cmd.argc != 2) printf(" put filename\n");
      else {
         strcpy(cmd.args[0], "stor");
         cmd.cmd = CMD_PUT; }
   }
   else if(!strcasecmp(cmd.args[0], "pwd")){
      cmd.cmd = CMD_PWD;
   }
   else if(!strcasecmp(cmd.args[0], "open")){
      if(cmd.argc == 1) printf(" open address [port]\n");
      else cmd.cmd = CMD_OPEN;
   }
   else if(!strcasecmp(cmd.args[0], "user")){
      if(cmd.argc != 2) printf(" user username\n");
      else cmd.cmd = CMD_USER;
   }
   else if(!strcasecmp(cmd.args[0], "pass")){
      if(cmd.argc != 2) printf(" pass password\n");
      else cmd.cmd = CMD_PASS;
   }
   else if(!strcasecmp(cmd.args[0], "help")){
      if(cmd.argc == 1) printf(" help <command> (server side help!)\n");
      else cmd.cmd = CMD_HELP;
   }
   else if(!strcasecmp(cmd.args[0], "close")){
      cmd.cmd = CMD_CLOSE;
   }
   else if(!strcasecmp(cmd.args[0], "quit")){
      cmd.cmd = CMD_QUIT;
   }
   else{
      printf(" %s is not a valid command\n", cmd.args[0]);
   }
     
   return &cmd;
}

int do_ftp_client(ftp_conn_data* conn_data)
{
   char cmd_line[256];
   command* command;
   int get_reply = 1;
   char* getline;
   char* sendline;

   if(conn_data->status == DO_CONNECT)
      open_ctrl_connection(conn_data);
   
   while(1)
   {
      put_promt();
      fgets(cmd_line, 256, stdin);
      command = parse_cmd_line(cmd_line);
      if(command->argc)
         sendline = ftp_make_command(command->args[0], command->args[1]);
         
      switch(command->cmd)
      {
         case CMD_NONE:
            get_reply = 0;
            break;

         case CMD_OPEN:
            if(conn_data->status == DISCONNECTED){
               strcpy(conn_data->hostname, command->args[1]);
               conn_data->port = (command->argc == 3) ?
                     strtol(command->args[2], (char**)NULL, 10) : 21;
               open_ctrl_connection(conn_data);
               puts(""); 
               get_reply = 0;
            } break;

         case CMD_QUIT:
            if(conn_data->status == CONNECTED){
               ftp_write(conn_data->controlfd, sendline);
               close_ctrl_connection(conn_data);
            } return 1;

         case CMD_CLOSE:
            close_ctrl_connection(conn_data);
            get_reply = 0;
            break;
         
         case CMD_LS:
            get_reply = get_list(conn_data, sendline);
            break;
            
         case CMD_GET:
            get_reply = ftp_get_file(conn_data, command->args[1]);
            break;
         
         case CMD_PUT:
            get_reply = ftp_put_file(conn_data, command->args[1]);
            break;
         
         case CMD_PWD:
            ftp_write(conn_data->controlfd, "pwd\r\n");
            get_reply = 1;
            break;
         
         case CMD_CD:
         case CMD_USER:
         case CMD_PASS:
         case CMD_HELP:
            if(conn_data->status == DISCONNECTED){
               printf("Not connected\n");
               break;
            }
            get_reply = ftp_write(conn_data->controlfd, sendline);
            break;
      }
      
      if(get_reply)  /* get server reply */
      {
         getline = ftp_read(conn_data->controlfd);
         printf("%s", getline);
         
         switch(ftp_reply_code(getline))
         {
            case 230:
               conn_data->status = LOGIN;
               break;
               
            default:
               break;
         }
      }
   }
   
   return 1;
}

/* will open control connection
 return:  1 if OK, -1 on error */
int open_ctrl_connection(ftp_conn_data* conn_data)
{
   unsigned int len = 128;
   struct hostent* h_ent;
   sockaddr server;
   char* line;
   char* ipaddr;
   
   if(conn_data->status == CONNECTED)
   {
      printf("Already connected\nhint: use close");
      return 0;
   }
   
   if(!(h_ent = (struct hostent*)gethostbyname(conn_data->hostname)))
   {
      printf("Can't resolve %s: error code: %d\n", conn_data->hostname, h_errno);
      return -1;
   }	
   if((conn_data->controlfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket error");
      exit(EXIT_FAILURE);
   }
   
   printf("Resolving %s\n", conn_data->hostname);
   server.sin_family = AF_INET;
   server.sin_port = htons(conn_data->port);
   memcpy((char*)&server.sin_addr, h_ent->h_addr, h_ent->h_length);
   ipaddr = inet_ntoa(server.sin_addr);
   printf("%s resolves to %s\n", conn_data->hostname, ipaddr);
   
   if(connect(conn_data->controlfd, (sockaddr*)&server, sizeof(server)) < 0)
   {
      printf("Connection refused to %s\n", conn_data->hostname);
      return -1;
   }
   if(getsockname(conn_data->controlfd, (sockaddr*)&server, &len) < 0)
   {
      perror("getsockname error");
      exit(EXIT_FAILURE);
   }
   
   conn_data->localip = server.sin_addr.s_addr;
   conn_data->status = CONNECTED;
   
   line = ftp_read(conn_data->controlfd);
   printf("%s\n", line);
   
   if(ftp_reply_code(line) != 220)
   {  
      fprintf(stderr, "Got non standard/implemented reply!\n");
      return -1;
   }
   
   printf("Connected to %s on port %d\n", conn_data->hostname, conn_data->port);
   conn_data->status = WAIT_LOGIN;
   
   return 1;
}

int close_ctrl_connection(ftp_conn_data* conn_data)
{
   if(conn_data->status == DISCONNECTED)
   {
      printf("Not connected\n");
      return 0;
   }
   
   if(close(conn_data->controlfd) < 0)
   {
      perror("close");
      exit(EXIT_FAILURE);
   }
   conn_data->controlfd = 0;
   conn_data->status = DISCONNECTED;
   
   printf("Disconnected from %s\n", conn_data->hostname);
   
   return 1;
}

int ftp_open_data_connection(ftp_conn_data* conn_data)
{
   sockaddr server;
   struct in_addr sin;
   char line[128];
   int h1, h2, h3, h4, p1, p2, port;
   socklen_t sock_len = sizeof(sockaddr);
   
   if((conn_data->dataconfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket error");
      exit(EXIT_FAILURE);
   }
   
   memset(&server, 0, sizeof(sockaddr));
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   server.sin_port = 0;
   
   if(bind(conn_data->dataconfd, (sockaddr*)&server, sizeof(sockaddr)) < 0)
   {
      perror("bind error");
      exit(EXIT_FAILURE);
   }
   if(listen(conn_data->dataconfd, 1) < 0)
   {
      perror("listen error");
      exit(EXIT_FAILURE);
   }
   if(getsockname(conn_data->dataconfd, (sockaddr*)&server, &sock_len) < 0)
   {
      perror("getsockname error");
      exit(EXIT_FAILURE);
   }
   
   port = server.sin_port;
   sin.s_addr = conn_data->localip;
   sprintf(line, "%s.%d\r\n", inet_ntoa(sin), port);
   sscanf(line, "%d.%d.%d.%d.%d", &h1, &h2, &h3, &h4, &port);
   p1 = (port / 256);
   p2 = port - (256 * p1);
   sprintf(line, "PORT %d,%d,%d,%d,%d,%d\r\n", h1, h2, h3, h4, p2, p1);
   ftp_write(conn_data->controlfd, line);
   printf("%s", ftp_read(conn_data->controlfd));
   
   return 1;
}

int ftp_close_data_connection(ftp_conn_data* conn_data)
{
   if(conn_data->dataconfd) close(conn_data->dataconfd);
   if(conn_data->datafd) close(conn_data->datafd);
   conn_data->datafd = conn_data->dataconfd = 0;
   
   return 1;
}

int ftp_wait_data(ftp_conn_data* conn_data)
{
   sockaddr server;
   int len = sizeof(sockaddr);
   
   conn_data->datafd = accept(conn_data->dataconfd, (sockaddr*)&server, &len);
	
   return 1;
}

int get_list(ftp_conn_data* conn_data, char* line)
{
   char buffer[4096+1];
   ssize_t nread = 1;
   
   if(conn_data->status == DISCONNECTED)
   {
      printf("Not connected\n");
      return 0;
   } 
   else if(conn_data->status == WAIT_LOGIN)
   {
      printf("You must login with USER and PASS first\n");
      return 0;
   }
  
   ftp_open_data_connection(conn_data);
   ftp_write(conn_data->controlfd, line);
   printf("%s", ftp_read(conn_data->controlfd));
   ftp_wait_data(conn_data);
   
   while(nread)
   {
      if((nread = read(conn_data->datafd, buffer, 4096)) < 0)
      {
         perror("read error");
         exit(EXIT_FAILURE);
      }
      else if(nread == 4096)
         buffer[4096] = '\0';
      else
         buffer[nread] = '\0';
      printf("%s", buffer);
      buffer[0] = '\0';
   }
   
   ftp_close_data_connection(conn_data);
   
   return 1;
}

int ftp_get_file(ftp_conn_data* conn_data, char* fname)
{
   int fd;
   ssize_t nread = 1;
   char buffer[4096];
   char* line;
   
   if(conn_data->status == DISCONNECTED)
   {
      printf("Not connected\n");
      return 0;
   } 
   else if(conn_data->status == WAIT_LOGIN)
   {
      printf("You must login with USER and PASS first\n");
      return 0;
   }
   
   ftp_write(conn_data->controlfd, ftp_make_command("type", "i"));
   printf("%s", ftp_read(conn_data->controlfd));
   
   ftp_open_data_connection(conn_data);
   ftp_write(conn_data->controlfd, ftp_make_command("retr", fname));
   line = ftp_read(conn_data->controlfd);
   printf("%s", line);
   
   if(ftp_reply_code(line) == 550)
   {
      close(fd);
      ftp_close_data_connection(conn_data);
      return 0;
   }
   
   if((fd = open(fname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
   {
      perror(fname);
      return 0;
   }
   
   ftp_wait_data(conn_data);
   
   while((nread = read(conn_data->datafd, buffer, 4096)) > 0)
   {
      if(nread == -1)
      {
         perror("socket read error");
         exit(EXIT_FAILURE);
      }
      else if(write(fd, buffer, nread) == -1)
      {
         perror("local file write error");
         exit(EXIT_FAILURE);
      }
   }
   
   ftp_close_data_connection(conn_data);
   close(fd);
   return 1;
}

int ftp_put_file(ftp_conn_data* conn_data, char* fname)
{
   int fd;
   ssize_t nread = 1;
   char buffer[4096];
   char* line;
   
   if(conn_data->status == DISCONNECTED)
   {
      printf("Not connected\n");
      return 0;
   } 
   else if(conn_data->status == WAIT_LOGIN)
   {
      printf("You must login with USER and PASS first\n");
      return 0;
   }
   
   if((fd = open(fname, O_RDONLY)) == -1)
   {
      perror(fname);
      return 0;
   }
   
   ftp_write(conn_data->controlfd, ftp_make_command("type", "i"));
   printf("%s", ftp_read(conn_data->controlfd));
   
   ftp_open_data_connection(conn_data);
   ftp_write(conn_data->controlfd, ftp_make_command("stor", fname));
   line = ftp_read(conn_data->controlfd);
   printf("%s", line);
   
   if(ftp_reply_code(line) == 550)
   {
      close(fd);
      ftp_close_data_connection(conn_data);
      return 0;
   }
   
   ftp_wait_data(conn_data);
   
   while((nread = read(fd, buffer, 4096)) > 0)
   {
      if(nread == -1)
      {
         perror("local file read error");
         exit(EXIT_FAILURE);
      }
      else if(write(conn_data->datafd, buffer, nread) == -1)
      {
         perror("socket write error");
         exit(EXIT_FAILURE);
      }
   }
   
   close(fd);
   ftp_close_data_connection(conn_data);
   return 1;
}
