FTP client

AUTHOR

   Eigo Madaloja
   eigo.madaloja@gmail.com

SYNOPSIS

   ftp [hostname] [port]
   
      port defaults to 21.

COMMANDS

   open hostname [port] -  connect to host <hostname> on port <port>. <port>
                           defaults to 21.
      
   ls [dirname]         -  list directory contents. Argument <dirname> is not implemented
                           in FTP server but can of course be used with other servers that
                           implement it.    
                            
   cd path              -  change current path.
   
   pwd                  -  print current directory.
   
   get filename         -  download file from server.
   
   put filename         -  upload file to server.
   
   user username        -  send user's name to server.
   
   pass password        -  send user's password to server.
   
   close                -  disconnect from server.
   
   quit                 -  disconnect from server and end the program.


DESCRIPTION

The client program should be compatible with the RFC specifications. It has been
tested with wuFTP server and it worked fine. This results in that the ftp client
implements little bit more than ftp server can handle (<user> and <pass> commands).

The client can be started by just running the plain executable an then
using the <open> command or specifing the hostname and/or port in arguments to
executable.

To login to ftp server just type <pass XXX> where XXX can be any data
as it doesn't actually do the login. This is implemented for keeping the client
compatible with servers conforming to the standard where the USER and PASS commands
must be used.
