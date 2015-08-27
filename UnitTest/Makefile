INCDIR:=../../cppunit/include:../lib

VPATH:=$(INCDIR)

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I ,$(INCDIR))
LDFLAGS:=-L ../lib -lsnail -lrt -lcppunit -lpthread

.Phony:all lib clean
all:lib TestMain TestServer TestClient

lib:
	cd ../lib && $(MAKE)

TestMain:TestMain.o QueueTest.o StackTest.o
	$(CXX) $^ $(LDFLAGS) -o $@
TestServer:TestServer.o ServerTest.o EchoServer.o
	$(CXX) $^ $(LDFLAGS) -o $@
TestClient:TestClient.o ClientTest.o EchoClient.o
	$(CXX) $^ $(LDFLAGS) -o $@

TestMain.o:TestMain.cpp
QueueTest.o:QueueTest.cpp QueueTest.h algorithm/Queue.h
StackTest.o:StackTest.cpp StackTest.h algorithm/Stack.h

TestServer.o:TestServer.cpp
ServerTest.o:ServerTest.cpp ServerTest.h
EchoServer.o:EchoServer.cpp EchoMessage.h

TestClient.o:TestClient.cpp
ClientTest.o:ClientTest.cpp ClientTest.h
EchoClient.o:EchoClient.cpp EchoMessage.h

clean:
	rm *.o TestMain TestServer
