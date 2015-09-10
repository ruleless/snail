LIBDIR:=../../lib
INCDIR:=../../../cppunit/include:$(LIBDIR)

VPATH:=$(INCDIR)

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I ,$(INCDIR))
LDFLAGS:=-L $(LIBDIR) -lsnail -lrt -lcppunit -lpthread

.Phony:all lib clean
all:lib TestMain

lib:
	cd $(LIBDIR) && $(MAKE)

TestMain:TestMain.o QueueTest.o StackTest.o FlexibleArrTest.o
	$(CXX) $^ $(LDFLAGS) -o $@

TestMain.o:TestMain.cpp
QueueTest.o:QueueTest.cpp QueueTest.h algorithm/Queue.h
StackTest.o:StackTest.cpp StackTest.h algorithm/Stack.h
FlexibleArrTest.o:FlexibleArrTest.cpp FlexibleArrTest.h algorithm/FlexibleArray.h

clean:
	rm *.o TestMain
