all: client.o server.o

client.o: client.c common.h
		cc -o client client.c -W -Wall
server.o: server.c common.h
		cc -o server server.c -W -Wall -lpthread

clean:
		rm -rf *.o