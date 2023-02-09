# Flags
CC=arm-linux-gcc
CXX=arm-linux-g++
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)

SRCS=main.c
OBJS=$(subst .c,.o,$(SRCS))
OUT=serialTest

all: $(OUT)

# Automatically compile with flags
$(OUT): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUT) $(OBJS) $(LDLIBS)

main.o: main.c
	$(CXX) $(LDFLAGS) -c main.c


clean:
	$(RM) $(OBJS)
	$(RM) $(OUT)
