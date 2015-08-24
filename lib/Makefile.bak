VPATH:=./common:./net:./algorithm

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I .
SHARED:=-fPIC --shared

.PHONY=all clean

all:libsnail.a

libsnail.a:common.o Thread.o Accepter.o NetManager.o Receiver.o Sender.o
	ar crv $@ $^

common.o:common.cpp common.h
Thread.o:Thread.cpp Thread.h
Accepter.o:Accepter.cpp Accepter.h NetManager.h NetDef.h common.h
NetManager.o:NetManager.cpp NetManager.h NetDef.h common.h SocketOutputStream.h SocketInputStream.h
Receiver.o:Receiver.cpp Receiver.h NetManager.h NetDef.h common.h
Sender.o:Sender.cpp Sender.h NetManager.h NetDef.h common.h

clean:
	-rm *.o *.a
