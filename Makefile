IP=127.0.0.1# CHANGE IT IF YOU WANT
PORT=1115# CHANGE IT IF YOU WANT
all: build
server:
	gcc server.c -c -o server.o
client:
	gcc client.c -c -o client.o

build: client server
	gcc server.o util.h -o server
	gcc client.o util.h -o client
	rm server.o client.o
run_server:
	./server $(IP) $(PORT)

run_client:
	./client $(IP) $(PORT)

clean:
	rm -f server client

