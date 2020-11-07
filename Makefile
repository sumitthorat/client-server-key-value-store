server:	Server
	./Server

client:	Client
	./Client

test: Test
	./Test

Server: server.o hashtable.o queue.o
	gcc server.o hashtable.o queue.o -o Server -lpthread -lm

Client: client.o
	gcc client.o -o Client -lpthread

Test: test.o
	gcc test.o -o Test -lpthread

server.o: server.c RW_lock/rwlock.h Requests/req_handler.h Storage/cache.h
	gcc -c server.c 

hashtable.o: DS_Utilities/hashtable.c DS_Utilities/ds_defs.h
	gcc -c DS_Utilities/hashtable.c

queue.o: DS_Utilities/queue.c DS_Utilities/ds_defs.h
	gcc -c DS_Utilities/queue.c

client.o: client.c 
	gcc -c client.c

test.o : Tests/test_client.c
	gcc -c Tests/test_client.c -o test.o

clean:
	rm -f Server Client Test *.o a.out
	rm -f test_client
	rm -f Tests/test_client.o KVClient/client_api.o
	echo -n "" > PS.txt



#### Run Test Client ####
run_test_client: test_client
	@./test_client

test_client: Tests/test_client.o KVClient/client_api.o
	@gcc Tests/test_client.o KVClient/client_api.o -o test_client -lpthread

Tests/test_client.o: Tests/test_client.c
	@gcc -c Tests/test_client.c -o Tests/test_client.o

KVClient/client_api.o: KVClient/KVClientLibrary.h KVClient/KVClientLibrary.c
	@gcc -c KVClient/KVClientLibrary.c -o KVClient/client_api.o
