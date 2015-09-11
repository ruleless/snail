LIBDIR:=../../lib
INCDIR:=../../../cppunit/include:$(LIBDIR)

ALG:=algorithm

VPATH:=$(INCDIR):$(LIBDIR)/$(ALG)

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I ,$(INCDIR))
LDFLAGS:=-L $(LIBDIR) -lsnail -lrt -lcppunit -lpthread

.Phony:all lib clean
all:lib TestMain

lib:
	cd $(LIBDIR) && $(MAKE)

TestMain:TestMain.o QueueTest.o StackTest.o FlexibleArrTest.o math.o
	$(CXX) $^ $(LDFLAGS) -o $@

TestMain.o:TestMain.cpp
QueueTest.o:QueueTest.cpp QueueTest.h $(ALG)/Queue.h
StackTest.o:StackTest.cpp StackTest.h $(ALG)/Stack.h
math.o:$(ALG)/math.h $(ALG)/math.cpp
FlexibleArrTest.o:FlexibleArrTest.cpp FlexibleArrTest.h $(ALG)/FlexibleArray.h

clean:
	rm *.o TestMain
