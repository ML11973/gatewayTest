# Flags
CC=arm-linux-g++
CXX=arm-linux-g++
RM=rm -f
INC=-I.
CPPFLAGS=-c $(INC) -fpermissive
CFLAGS=-c $(INC) -fpermissive
LDFLAGS=
LDLIBS=

SRCS_C=main.c
SRCS_CPP=
OBJS_CPP=$(subst .cpp,.o,$(SRCS_CPP))
OBJS_C=$(subst .c,.o,$(SRCS_C))
OUT=udpServ

all: $(OUT)

# Automatically compile with flags

main.o: main.c
	$(CXX) $(LDFLAGS) $(CPPFLAGS) main.c

$(OUT): $(OBJS_C) $(OBJS_CPP)
	$(CXX) $(LDFLAGS) -o $(OUT) $(OBJS_CPP) $(OBJS_C) $(LDLIBS)


clean:
	$(RM) $(OBJS_CPP)
	$(RM) $(OBJS_C)
	$(RM) $(OUT)
