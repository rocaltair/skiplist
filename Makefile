PLATFORM=$(shell uname)
CC = gcc

CFLAGS = -c -g3 -Wall -Werror=declaration-after-statement -std=c89 -pedantic
LIBS = 
LDFLAGS = -g3 -Wall $(LIBS)

BIN = test
OBJS = main.o skiplist.o

all : $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) 

$(OBJS) : %.o : %.c
	$(CC) -o $@ $(CFLAGS) $<

clean : 
	rm -f $(OBJS) $(BIN)

.PHONY : clean

