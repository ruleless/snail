INCDIR:=../../cppunit/include:../lib

VPATH:=$(INCDIR)

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I,$(INCDIR))
LDFLAGS:=-lrt -lcppunit

.Phony:all clean
all:TestMain

TestMain:TestMain.o QueueTest.o StackTest.o
	$(CXX) $^ $(LDFLAGS) -o $@

TestMain.o:TestMain.cpp
QueueTest.o:QueueTest.cpp QueueTest.h algorithm/Queue.h
StackTest.o:StackTest.cpp StackTest.h algorithm/Stack.h

clean:
	rm *.o TestMain
