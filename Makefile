CC = g++
CPPFLAGS = -std=c++17
LDLIBS = -lpthread

all: client server

client: client.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

server: server.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f client server *.o
