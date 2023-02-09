# Flags
CC=gcc
CXX=g++
RM=rm -f

SRCS=main.c
OBJS=$(subst .c,.o,$(SRCS))
OUT=udpClient

all: $(OUT)

# Automatically compile with flags
$(OUT): $(OBJS)
	$(CC) -o $(OUT) $(OBJS)

main.o: main.c
	$(CC) -c $(CFLAGS) main.c


clean:
	$(RM) $(OBJS)
	$(RM) $(OUT)
