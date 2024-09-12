
PROGS	= tunvpn
CXX		= g++
CFLAGS	= -Wall
LDFLAGS	=

PWD		= $(shell pwd)
USER	= "$(shell id -u):$(shell id -g)"

all: $(PROGS)

tunvpn: tunvpn.cpp
	docker run -it --rm -v "$(PWD):/build" --user $(USER) --workdir=/build chuang/tunlab $(CXX) -o $@ $(CFLAGS) $^ $(LDFLAGS)

demo: tunvpn.demo.cpp
	docker run -it --rm -v "$(PWD):/build" --user $(USER) --workdir=/build chuang/tunlab $(CXX) -o tunvpn $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f *.o $(PROGS)

ks:
	pkill -f iperf3
	pkill -f tunvpn
kc:
	pkill -f iperf3
	pkill -f tunvpn
rs:
	/dist/run.sh server

rc:
	/dist/run.sh client