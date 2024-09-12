all:
	g++ -o server server.cpp 
	fuser 8888/tcp -k
	./server 8888
connect:
	nc localhost 8889