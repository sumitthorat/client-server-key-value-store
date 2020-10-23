
server: server_main
	./server
client: client_main
	./client
test: test_main
	./test
server_main: server.c handle_reqs.h
	gcc server.c handle_reqs.h -lpthread -o server
client_main: client.c 
	gcc client.c -o client
test_main: test_client.c
	gcc test_client.c -o test
clean:
	rm server client test
