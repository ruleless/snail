INCDIR:=.:./deps

VPATH:=./common:./helper./algorithm:./network:./thread

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I,$(INCDIR))
SHARED:=-fPIC --shared

objs:=common.o timestamp.o strutil.o MemoryStream.o Tasks.o \
	EventDispatcher.o EventPoller.o SelectPoller.o EpollPoller.o \
	Address.o EndPoint.o Listener.o Channel.o PacketReader.o MessageHandler.o \
	PacketReceiver.o TCPPacketReceiver.o UDPPacketReceiver.o \
	PacketSender.o TCPPacketSender.o \
	Bundle.o TCPPacket.o UDPPacket.o PacketFilter.o \
	NetworkDef.o NetworkManager.o DelayedChannels.o NetworkStats.o \

.PHONY=all clean

all:libsnail.a

libsnail.a:$(objs)
	ar crv $@ $^

common.o:common.cpp common.h platform.h
timestamp.o:timestamp.cpp timestamp.h
strutil.o:strutil.cpp strutil.h
MemoryStream.o:MemoryStream.cpp MemoryStream.h
Tasks.o:
EventDispatcher.o:
EventPoller.o:
SelectPoller.o:
EpollPoller.o:
Address.o:
EndPoint.o:
Listener.o:
Channel.o:
PacketReader.o:
MessageHandler.o:
PacketReceiver.o:
TCPPacketReceiver.o:
UDPPacketReceiver.o:
PacketSender.o:
TCPPacketSender.o:
Bundle.o:
TCPPacket.o:
UDPPacket.o:
PacketFilter.o:
NetworkDef.o:
NetworkManager.o:
DelayedChannels.o:
NetworkStats.o:

clean:
	-rm *.o *.a
