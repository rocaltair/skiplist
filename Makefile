PLATFORM=$(shell uname)
CC = gcc

CFLAGS = -c -g3 -Wall -Werror=declaration-after-statement -std=c89 -pedantic -fPIC
LIBS = 
LDFLAGS = -g3 -Wall $(LIBS)

BIN = test/test
TEST_OBJS = test/main.o src/skiplist.o
LUALIB_OBJS = src/skiplist.o lua-bind/lskiplist.o

SOLIB = lua-bind/lskiplist.so

all : $(BIN)

lua : $(SOLIB)

$(BIN): $(TEST_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) 

$(TEST_OBJS) : %.o : %.c
	$(CC) -o $@ $(CFLAGS) $< -I./src

$(SOLIB) : $(LUALIB_OBJS)
	$(CC) -o $@ $^ --shared -dynamiclib -Wl,-undefined,dynamic_lookup

lua-bind/lskiplist.o : lua-bind/lskiplist.c | src/skiplist.h
	$(CC) -o $@ $(CFLAGS) $< -I./src

clean : 
	rm -f $(TEST_OBJS) $(BIN) $(LUALIB_OBJS) $(SOLIB)

.PHONY : clean

