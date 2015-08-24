INCDIR:=.:./deps

VPATH:=./common:./helper./algorithm:./network:./thread

CC:=clang++
CXX:=clang++
CXXFLAGS:=-g -Wall -I $(subst :, -I ,$(INCDIR))
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
Tasks.o:Tasks.cpp Tasks.h Task.h ThreadGuard.h
EventDispatcher.o:EventDispatcher.cpp EventDispatcher.h EventPoller.h Network.h NetworkDef.h Tasks.h Timer.h
EventPoller.o:EventPoller.cpp EventPoller.h SelectPoller.h EpollPoller.h Network.h NetworkDef.h
SelectPoller.o:SelectPoller.cpp SelectPoller.h EventPoller.h
EpollPoller.o:EpollPoller.cpp EpollPoller.h EventPoller.h
Address.o:Address.cpp Address.h ObjectPool.h NetworkDef.h
EndPoint.o:EndPoint.cpp EndPoint.inl EndPoint.h Address.h ObjectPool.h Bundle.h \
	TCPPacketReceiver.h TCPPacketSender.h UDPPacketReceiver.h
Listener.o:Listener.cpp Listener.h Network.h NetworkDef.h EndPoint.h NetworkManager.h
Channel.o:Channel.cpp Channel.h EndPoint.h Timer.h ObjectPool.h SmartPointer.h \
	Network.h NetworkDef.h Packet.h Bundle.h PacketFilter.h PacketReader.h \
	PacketReceiver.h TCPPacketReceiver.h UDPPacketReceiver.h TCPPacketSender.h \
	Packet.h TCPPacket.h UDPPacket.h MessageHandler.h NetworkStats.h
PacketReader.o:PacketReader.cpp PacketReader.h NetworkDef.h MemoryStream.h Channel.h MessageHandler.h \
	NetworkStats.h
MessageHandler.o:MessageHandler.cpp MessageHandler.h MemoryStream.h NetworkDef.h
PacketReceiver.o:PacketReceiver.cpp PacketReceiver.h Network.h ObjectPool.h \
	Channel.h EndPoint.h EventDispatcher.h NetworkManager.h
TCPPacketReceiver.o:TCPPacketReceiver.cpp TCPPacketReceiver.h PacketReceiver.h \
	Network.h NetworkDef.h TCPPacket.h
UDPPacketReceiver.o:UDPPacketReceiver.cpp UDPPacketReceiver.h PacketReceiver.h \
	Network.h NetworkDef.h UDPPacket.h
PacketSender.o:PacketSender.cpp PacketSender.h ObjectPool.h \
	Network.h NetworkDef.h 
TCPPacketSender.o:TCPPacketSender.cpp TCPPacketSender.h PacketSender.h ObjectPool.h \
	Network.h NetworkDef.h TCPPacket.h
Bundle.o:Bundle.cpp Bundle.h Bundle.inl Packet.h ObjectPool.h MessageHandler.h
TCPPacket.o:TCPPacket.cpp TCPPacket.h NetworkDef.h ObjectPool.h Packet.h Address.h EndPoint.h
UDPPacket.o:UDPPacket.cpp UDPPacket.h NetworkDef.h ObjectPool.h Packet.h Address.h EndPoint.h
PacketFilter.o:PacketFilter.cpp PacketFilter.h NetworkDef.h RefCountable.h SmartPointer.h \
	Channel.h PacketReceiver.h PacketSender.h
NetworkDef.o:NetworkDef.cpp NetworkDef.h
NetworkManager.o:NetworkManager.cpp NetworkManager.h common.h Timer.h MemoryStream.h \
	NetworkDef.h EndPoint.h
DelayedChannels.o:DelayedChannels.cpp DelayedChannels.h Tasks.h Singleton.h \
	EventDispatcher.h NetworkManager.h
NetworkStats.o:NetworkStats.cpp NetworkStats.h Network.h Singleton.h MessageHandler.h

clean:
	-rm *.o *.a
