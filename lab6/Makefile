all: server.cpp client.cpp
	g++ -static -o myserver server.cpp -g
	g++ -static -o myclient client.cpp -g
	rm ./server_files/*
	./myserver /server_files 1000 12345 &
	./myclient /files 1000 12345 127.0.0.1
	sleep 15s
	pkill -f myserver
	pkill -f myserver

test: server.cpp client.cpp
	g++ -static -o server server.cpp
	g++ -static -o client client.cpp
	TOKEN=9d9erJ6aJ8u23P37 python3 submit.py ./server ./client

check:
	for number in 000009 000100 000123 000125 000789 000987 ; do \
		diff ./files/$$number ./server_files/$$number ; \
	done

kill:
	pkill -f myserver

rs:
	./myserver /server_files 1000 12345
rc: 
	./myclient /files 1000 12345 127.0.0.1