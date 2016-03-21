TOPDIR:=..

DIRS:=$(TOPDIR)/src $(TOPDIR)/src/ipc
OBJS:=$(foreach v,$(DIRS),$(wildcard $(v)/*.o))

LIB:=libsnail.a
LIBPATH:=$(TOPDIR)/lib

include ./build.mak
