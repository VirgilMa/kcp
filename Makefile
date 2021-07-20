all:
	gcc ikcp.c server.c client.c -c -g
	gcc ikcp.o server.o -o server -g
	gcc ikcp.o client.o -o client -g

clean:
	rm server client *.o
