LIBDIR:=../../lib
INCDIR:=../cppunit/include:$(LIBDIR)

VPATH:=$(INCDIR)

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I ,$(INCDIR))
LDFLAGS:=-L $(LIBDIR) -lsnail -lrt -lcppunit -lpthread

.Phony:all lib clean
all:lib TestMain

lib:
	cd $(LIBDIR) && $(MAKE)

TestMain:TestMain.o LogTest.o
	$(CXX) $^ $(LDFLAGS) -o $@

TestMain.o:TestMain.cpp
LogTest.o:LogTest.cpp LogTest.h

clean:
	rm *.o TestMain
