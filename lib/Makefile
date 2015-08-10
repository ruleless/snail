INC_DIR:=./lib
SRC_DIR:=./lib

VPATH:=$(SRC_DIR):$(SRC_DIR)/common:$(SRC_DIR)/net:$(SRC_DIR)/algorithm

CC:=g++
CXX:=g++
CXXFLAGS:=-g -Wall -I ./lib
SHARED:=-fPIC --shared

.PHONY=all clean

all:libsnail.a

libsnail.a:common.o Thread.o Accepter.o NetManager.o Receiver.o Sender.o
	ar crv $@ $^

common.o:common.cpp common.h
Thread.o:Thread.cpp Thread.h
Accepter.o:Accepter.cpp NetDef.h
NetManager.o:NetManager.cpp NetDef.h
Receiver.o:Receiver.cpp NetDef.h
Sender.o:Sender.cpp NetDef.h

clean:
	-rm *.o *.a
