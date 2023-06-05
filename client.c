#include "ftpc.h"

int main(int argc, char* args[])
{
   ftp_conn_data* conn_data;
   
   if((conn_data =
      (ftp_conn_data*)calloc(1, sizeof(ftp_conn_data))) == NULL)
         fatal("Out of mem");
         
   conn_data->status = DISCONNECTED;

   if(argc == 2){
      strcpy(conn_data->hostname, args[1]);
      conn_data->port = 21;
      conn_data->status = DO_CONNECT;
   } else if(argc == 3){
      strcpy(conn_data->hostname, args[1]);
      conn_data->port = strtol(args[2], (char**)NULL, 10);
      conn_data->status = DO_CONNECT;
   }
      
   do_ftp_client(conn_data);
   
   free(conn_data);
   
   return EXIT_SUCCESS;
}
