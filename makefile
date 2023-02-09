# Flags
CC=arm-linux-g++
CXX=arm-linux-g++
RM=rm -f
INC=-I. -IISM3_Linux -IWPAN -INetwork -IProtocol -ItestMenu -IRouter
CPPFLAGS=-c $(INC) -fpermissive
CFLAGS=-c $(INC) -fpermissive
LDFLAGS=
LDLIBS=

SRCS_C=ISM3_Linux/util.c ISM3_Linux/buffered_uart.c ISM3_Linux/framed_uart.c ISM3_Linux/ism3.c WPAN/ism3_server.c WPAN/cm4_utils.c
SRCS_CPP=main.cpp WPAN/node.cpp WPAN/powernode.cpp WPAN/datanode.cpp WPAN/ism3_handlers.cpp testMenu/menu.cpp WPAN/wpanManager.cpp Router/Connection.cpp Router/BorderRouter.cpp
OBJS_CPP=$(subst .cpp,.o,$(SRCS_CPP))
OBJS_C=$(subst .c,.o,$(SRCS_C))
OUT=gatewayTest

all: $(OUT)

# Automatically compile with flags
Connection.o: Router/Connection.cpp Router/Connection.h Protocol/network.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) Connection.cpp

BorderRouter.o: Router/BorderRouter.cpp Router/BorderRouter.h Router/netconfig.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) BorderRouter.cpp

node.o: WPAN/node.cpp WPAN/node.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) node.cpp

powernode.o: WPAN/powernode.cpp WPAN/powernode.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) powernode.cpp

datanode.o: WPAN/datanode.cpp WPAN/datanode.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) datanode.cpp

menu.o: testMenu/menu.cpp testMenu/menu.h
	$(CXX) $(LDFLAGS) $(CFLAGS) menu.cpp

wpanManager.o: WPAN/wpanManager.cpp WPAN/wpanManager.h Protocol/wpan.h
	$(CXX) $(LDFLAGS) $(CFLAGS) wpanManager.cpp

ism3_handlers.o: WPAN/ism3_handlers.cpp WPAN/ism3_handlers.h
	$(CXX) $(LDFLAGS) $(CFLAGS) ism3_handlers.cpp

ism3_server.o: WPAN/ism3_server.c WPAN/ism3_server.h
	$(CC) $(LDFLAGS) $(CFLAGS) ism3_server.c

ism3.o: ISM3_Linux/ism3.c ISM3_Linux/ism3.h
	$(CC) $(LDFLAGS) $(CFLAGS) ism3.c

framed_uart.o: ISM3_Linux/framed_uart.c ISM3_Linux/framed_uart.h
	$(CC) $(LDFLAGS) $(CFLAGS) framed_uart.c

buffered_uart.o: ISM3_Linux/buffered_uart.c ISM3_Linux/buffered_uart.h
	$(CC) $(LDFLAGS) $(CFLAGS) buffered_uart.c

util.o: ISM3_Linux/util.c ISM3_Linux/util.h
	$(CC) $(LDFLAGS) $(CFLAGS) util.c

cm4_utils.o: WPAN/cm4_utils.c WPAN/cm4_utils.h
	$(CC) $(LDFLAGS) $(CFLAGS) cm4_utils.c

main.o: main.cpp main.h
	$(CXX) $(LDFLAGS) $(CPPFLAGS) main.cpp

$(OUT): $(OBJS_C) $(OBJS_CPP) Protocol/wpan.h
	$(CXX) $(LDFLAGS) -o $(OUT) $(OBJS_CPP) $(OBJS_C) $(LDLIBS)


clean:
	$(RM) $(OBJS_CPP)
	$(RM) $(OBJS_C)
	$(RM) $(OUT)
