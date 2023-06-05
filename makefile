CC=gcc
OPTS=-Wall -ansi
CLIENT_OBJECTS=client.o ftpc.o ftp.o
SERVER_OBJECTS=server.o ftps.o ftp.o

all:	ftpc ftps

clean:
	rm -f *.o ftp
	rm -f *.o ftps

ftpc: $(CLIENT_OBJECTS)
	$(CC) $(OPTS) -o ftp $(CLIENT_OBJECTS)

client_main.o: client_main.c
	$(CC) $(OPTS) -o client.o -c client.c

ftpc.o: ftpc.c ftpc.h
	$(CC) $(OPTS) -o ftpc.o -c ftpc.c

ftps: $(SERVER_OBJECTS)
	$(CC) $(OPTS) -o ftps $(SERVER_OBJECTS)

server_main.o: server_main.c
	$(CC) $(OPTS) -o server.o -c server.c

ftps.o: ftps.c ftps.h
	$(CC) $(OPTS) -o ftps.o -c ftps.c

ftp.o: ftp.c ftp.h
	$(CC) $(OPTS) -o ftp.o -c ftp.c
