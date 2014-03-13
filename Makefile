# vim: noet:

ERL_FILE 	=$(wildcard ./src/erl/*.c)
ERL_TARGET 	=$(ERL_FILE:%.c=%)
ERL_INC 	=-I/usr/lib/erlang/usr/include/
ERL_LIB 	=-L/usr/lib/erlang/usr/lib -lerl_interface -lei -lpthread
ERLC 		=erlc

CC 			=gcc
CFLAGS 		=-Wall -g -O3 -fpic -I/usr/lib/erlang/usr/include/
LIB 		=-lm -lzmq -lczmq -lpthread -lc
VALGRIND 	=valgrind

BASE_FILE 	=src/base.c

FILE 		=$(wildcard ./src/*/*.c)
TARGET 		=$(FILE:%.c=%)

C_TARGET 	=$(filter-out $(ERL_TARGET), $(TARGET))

API_FILE 	=$(wildcard ./api/*/*.c)


all:api $(TARGET) erl

api:$(patsubst %.c,%.o,$(API_FILE))

$(C_TARGET):$(patsubst %,%.o,$(C_TARGET)) $(BASE_FILE:%.c=%.o)
	$(CC) $@.o $(BASE_FILE:%.c=%.o) -o ./bin/$(subst /,_,$(@:src/%=%)) $(LIB)

$(ERL_TARGET):$(patsubst %.c,%.o,$(BASE_FILE) $(ERL_FILE))
	$(CC) $@.o $(BASE_FILE:%.c=%.o) -o ./bin/$(subst /,_,$(@:src/%=%)) $(LIB) $(ERL_LIB)

erl:
	erlc -o ebin esrc/*.erl

clean:
	@rm $(wildcard ./src/*.o) $(wildcard ./src/*/*.o) $(wildcard ./api/*/*.o) \
		$(wildcard ./bin/*) $(wildcard ./ebin/*)

test:
	@echo $(wildcard ./bin/*)
