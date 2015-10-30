LIBDIR:=../../lib
INCDIR:=../cppunit/include:$(LIBDIR)

VPATH:=$(INCDIR)

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I ,$(INCDIR))
LDFLAGS:=-L $(LIBDIR) -lsnail -lrt -lcppunit -lpthread

.Phony:all lib clean
all:lib TestServer TestClient

lib:
	cd $(LIBDIR) && $(MAKE)

TestServer:TestServer.o ServerTest.o EchoServer.o
	$(CXX) $^ $(LDFLAGS) -o $@
TestClient:TestClient.o ClientTest.o EchoClient.o
	$(CXX) $^ $(LDFLAGS) -o $@

TestServer.o:TestServer.cpp
ServerTest.o:ServerTest.cpp ServerTest.h
EchoServer.o:EchoServer.cpp EchoMessage.h

TestClient.o:TestClient.cpp
ClientTest.o:ClientTest.cpp ClientTest.h
EchoClient.o:EchoClient.cpp EchoMessage.h

clean:
	rm *.o TestClient TestServer
