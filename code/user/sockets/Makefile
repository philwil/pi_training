all: tcp_server

tcp_server: tcp_server.c
	$(CC) -o $@ $< -lpthread -lrt

clean:
	rm -f tcp_server *.o
 