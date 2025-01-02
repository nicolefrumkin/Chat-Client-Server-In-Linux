all: client server

client: client.c
	gcc -Wall client.c -o hw3client

server: server.c
	gcc -Wall server.c -o hw3server

clean:
	rm -rf hw3server hw3client *.o
