CC = gcc
server : load_config.o server.o fd_list.o
	$(CC) -o server load_config.o server.o -static
load_config.o server.o: load_config.h
clean:
	rm *.o
