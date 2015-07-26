VPATH:=./include:./src

cc?=g++
CXXFLAGS:=-g -Wall -I ./include
LIBDIR:=./lib
SHARED:=-fPIC --shared

$(LIBDIR)/libsnail.a:common.o $(LIBDIR)
	ar crv $@ common.o

$(LIBDIR):
	mkdir $(LIBDIR)

common.o:common.cpp common.h

clean:
	-rm *.o && rm -r lib
