INC_DIR:=./include
SRC_DIR:=./src
LIB_DIR:=./lib

VPATH:=$(INC_DIR):$(INC_DIR)/net:$(SRC_DIR):$(SRC_DIR)/net

CC:=g++
CXX:=g++
CXXFLAGS:=-g -Wall -I ./include
SHARED:=-fPIC --shared

.PHONY=all dir clean

all:dir $(LIB_DIR)/libsnail.a

dir:$(LIB_DIR)
$(LIB_DIR):
	mkdir $(LIB_DIR)

$(LIB_DIR)/libsnail.a:common.o Thread.o Accepter.o NetManager.o Receiver.o Sender.o
	ar crv $@ $^

common.o:common.cpp common.h
Thread.o:Thread.cpp Thread.h
Accepter.o:Accepter.cpp NetDef.h
NetManager.o:NetManager.cpp NetDef.h
Receiver.o:Receiver.cpp NetDef.h
Sender.o:Sender.cpp NetDef.h

clean:
	-rm *.o && rm -r lib
