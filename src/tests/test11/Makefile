#.PHONY: ../../ruros.a ../ruros-test.a
all: server 

COPTS=-g -I ../../ -I ../

../../gen/gen:
	make -C ../../gen
../../ruros.a:
	make -C ../..
../ruros-test.a:
	make -C ..

clean:
	rm -f serv1.client.cpp serv1.server.cpp serv2.client.cpp serv2.server.cpp 
	rm -f client.o server.o client server socket

serv1.client.cpp serv1.server.cpp: serv1.idl ../../gen/gen
	../../gen/gen -idl serv1.idl -client serv1.client.cpp -server serv1.server.cpp


server.o: server.cpp serv1.server.cpp serv1.client.cpp
	g++ $(COPTS) -c -o $@ $<

server: server.o ../ruros-test.a ../../ruros.a
	g++ -o $@ $^ ../ruros-test.a ../../ruros.a -lpthread

run:
	./run.sh
	echo Test successfull