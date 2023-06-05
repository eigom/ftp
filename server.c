#include "ftps.h"

int main(int argc, char* args[])
{
   int port = 21;
   
   if(argc == 2)
      port = strtol(args[1], (char**)NULL, 10);

   do_ftp_server(port);
   
   return EXIT_SUCCESS;
}
