.PHONY: gen/gen
CXXFLAGS =	-O2 -ggdb3 -Wall -I . -DDEBUG

OBJS =		client_call.o connection.o ruros.o threads.o service0.o serialize.o tests/unix_io.o
#service0.o 
LIBS =

TARGET =	ruros.a

$(TARGET):	$(OBJS)
	$(AR) cr $(TARGET) $(OBJS) $(LIBS)

gen/gen:
	make -C gen

service0.o: service0.cpp service0.client.cpp service0.server.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

service0.client.cpp service0.server.cpp: service0.idl gen/gen
	gen/gen -idl service0.idl -client service0.client.cpp -server service0.server.cpp

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) service0.client.cpp service0.server.cpp
