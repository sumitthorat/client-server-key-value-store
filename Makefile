server:	Server
	./Server

client:	Client
	./Client

test: Test
	./Test

Server: server.o
	gcc server.o -o Server -lpthread 

Client: client.o
	gcc client.o -o Client -lpthread

Test: test.o
	gcc test.o -o Test -lpthread

server.o: server.c RW_lock/rwlock.h Requests/req_handler.h Storage/cache.h
	gcc -c server.c 

client.o: client.c 
	gcc -c client.c

test.o : Tests/new_client.c
	gcc -c Tests/new_client.c -o test.o

clean:
	rm -f Server Client Test *.o a.out 
	echo -n "" > PS.txt