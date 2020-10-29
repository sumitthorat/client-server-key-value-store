server:	Server
	./Server

client:	Client
	./Client

Server: server.o
	gcc server.o -o Server -lpthread 

Client: client.o
	gcc client.o -o Client -lpthread

server.o: server.c RW_lock/rwlock.h Requests/req_handler.h Storage/cache.h
	gcc -c server.c 

client.o: client.c 
	gcc -c client.c

clean:
	rm -f Server Client *.o a.out
	echo -n "" > PS.txt