# Flags
CC=arm-linux-gcc
CXX=arm-linux-g++
RM=rm -f
INC=-IISM3_Linux
CPPFLAGS=-g $(shell root-config --cflags)
CFLAGS=-c $(INC)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)

SRCS=main.c ISM3_Linux/util.c ISM3_Linux/buffered_uart.c ISM3_Linux/framed_uart.c ISM3_Linux/ism3.c
OBJS=$(subst .c,.o,$(SRCS))
OUT=radioDriver

all: $(OUT)

# Automatically compile with flags
$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $(OUT) $(OBJS) $(LDLIBS)

ism3.o: ISM3_Linux/ism3.c ISM3_Linux/ism3.h
	$(CC) $(LDFLAGS) $(CFLAGS) ism3.c

framed_uart.o: ISM3_Linux/framed_uart.c ISM3_Linux/framed_uart.h
	$(CC) $(LDFLAGS) $(CFLAGS) framed_uart.c

buffered_uart.o: ISM3_Linux/buffered_uart.c ISM3_Linux/buffered_uart.h
	$(CC) $(LDFLAGS) $(CFLAGS) buffered_uart.c

util.o: ISM3_Linux/util.c ISM3_Linux/util.h
	$(CC) $(LDFLAGS) $(CFLAGS) util.c

main.o: main.c
	$(CC) $(LDFLAGS) $(CFLAGS) main.c


clean:
	$(RM) $(OBJS)
	$(RM) $(OUT)
