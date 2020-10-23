server: server.o
	./server
client: client.o
	./client
server.o: server.c
	gcc server.c -o server.o -lpthread
client.o: client.c
	gcc client.c -o client.o
clean:
	rm server.o
	rm client.o
	rm server
	rm client
