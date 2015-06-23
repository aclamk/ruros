#.PHONY: ../../gen/gen ../../ruros.a ../ruros-test.a
all: client server 

COPTS=-g -I ../../ -I ../

../../gen/gen:
	make -C ../../gen
../../ruros.a:
	make -C ../..
../ruros-test.a:
	make -C ..
	
clean:
	rm -f add.client.cpp add.server.cpp client.o server.o client server socket

add.client.cpp add.server.cpp: add.idl ../../gen/gen
	../../gen/gen -idl add.idl -client add.client.cpp -server add.server.cpp

client.o: client.cpp add.client.cpp
	g++ $(COPTS) -c -o $@ $<

server.o: server.cpp add.server.cpp
	g++ $(COPTS) -c -o $@ $<


client: client.o 
	g++ -o $@ $^ ../ruros-test.a ../../ruros.a -lpthread

server: server.o 
	g++ -o $@ $^ ../ruros-test.a ../../ruros.a -lpthread

run:
	./run.sh
	echo Test successfull