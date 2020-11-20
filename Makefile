all: server

server:	server.o http.o


server.o:	http.h server.c 
			gcc -c -o server.o server.c

http.o:		http.c http.h 
			gcc -c -o http.o http.c

clean:
	@rm -rf *.o
	@rm -rf server

