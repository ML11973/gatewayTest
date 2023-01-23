# Flags
CC=arm-linux-g++
CXX=arm-linux-g++
RM=rm -f
INC=-I. -IISM3_Linux -IGateway -INetwork -IProtocol -ItestMenu -IRouter
CPPFLAGS=-c $(INC) -fpermissive
CFLAGS=-c $(INC) -fpermissive
LDFLAGS=
LDLIBS=

SRCS_C=ISM3_Linux/util.c ISM3_Linux/buffered_uart.c ISM3_Linux/framed_uart.c ISM3_Linux/ism3.c Gateway/ism3_server.c Gateway/cm4_utils.c
SRCS_CPP=main.cpp Gateway/node.cpp Gateway/powernode.cpp Gateway/datanode.cpp Gateway/ism3_handlers.cpp testMenu/menu.cpp Gateway/wpanManager.cpp Router/Connection.cpp Router/BorderRouter.cpp
OBJS_CPP=$(subst .cpp,.o,$(SRCS_CPP))
OBJS_C=$(subst .c,.o,$(SRCS_C))
OUT=gatewayTest

all: $(OUT)

# Automatically compile with flags
Connection.o: Router/Connection.cpp Router/Connection.h Protocol/network.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) Connection.cpp

BorderRouter.o: Router/BorderRouter.cpp Router/BorderRouter.h Router/netconfig.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) BorderRouter.cpp

node.o: Gateway/node.cpp Gateway/node.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) node.cpp

powernode.o: Gateway/powernode.cpp Gateway/powernode.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) powernode.cpp

datanode.o: Gateway/datanode.cpp Gateway/datanode.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) datanode.cpp

menu.o: testMenu/menu.cpp testMenu/menu.h
	$(CXX) $(LDFLAGS) $(CFLAGS) menu.cpp

wpanManager.o: Gateway/wpanManager.cpp Gateway/wpanManager.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CFLAGS) wpanManager.cpp

ism3_handlers.o: Gateway/ism3_handlers.cpp Gateway/ism3_handlers.h
	$(CXX) $(LDFLAGS) $(CFLAGS) ism3_handlers.cpp

ism3_server.o: Gateway/ism3_server.c Gateway/ism3_server.h
	$(CC) $(LDFLAGS) $(CFLAGS) ism3_server.c

ism3.o: ISM3_Linux/ism3.c ISM3_Linux/ism3.h
	$(CC) $(LDFLAGS) $(CFLAGS) ism3.c

framed_uart.o: ISM3_Linux/framed_uart.c ISM3_Linux/framed_uart.h
	$(CC) $(LDFLAGS) $(CFLAGS) framed_uart.c

buffered_uart.o: ISM3_Linux/buffered_uart.c ISM3_Linux/buffered_uart.h
	$(CC) $(LDFLAGS) $(CFLAGS) buffered_uart.c

util.o: ISM3_Linux/util.c ISM3_Linux/util.h
	$(CC) $(LDFLAGS) $(CFLAGS) util.c

cm4_utils.o: Gateway/cm4_utils.c Gateway/cm4_utils.h
	$(CC) $(LDFLAGS) $(CFLAGS) cm4_utils.c

main.o: main.cpp main.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) main.cpp

$(OUT): $(OBJS_C) $(OBJS_CPP) Protocol/wpan.h
	$(CXX) $(LDFLAGS) -o $(OUT) $(OBJS_CPP) $(OBJS_C) $(LDLIBS)


clean:
	$(RM) $(OBJS_CPP)
	$(RM) $(OBJS_C)
	$(RM) $(OUT)
