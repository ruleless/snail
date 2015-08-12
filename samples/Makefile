SNAIL_DIR:=../lib
SNAIL_LIB:=../lib/libsnail.lib

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(SNAIL_DIR)
LDFLAGS:=-L $(SNAIL_DIR) -lsnail -lpthread -lrt

.PHONY:all lib exe clean
all:lib exe

lib:
	cd $(SNAIL_DIR) && $(MAKE)

exe:EchoServer

EchoServer:EchoServer.o
	$(CXX) EchoServer.o $(LDFLAGS) -o $@

EchoServer.o:EchoServer.cpp EchoServer.h

clean:
	-rm *.o EchoServer
