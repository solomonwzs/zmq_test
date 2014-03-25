# vim: noet:

.SUFFIXES: 	.erl .beam

ERL_SRC 	=$(wildcard ./esrc/*.erl)

ERL_FILE 	=$(wildcard ./src/erl/*.c)
ERL_TARGET 	=$(ERL_FILE:%.c=%)
ERL_INC 	=-I/usr/lib/erlang/usr/include/
ERL_LIB 	=-L/usr/lib/erlang/usr/lib -lerl_interface -lei -lpthread
ERLC 		=erlc

A_SRC 		=$(wildcard ./src/MDP/*.c)
A_TARGET  	=$(A_SRC:%.c=%)

CC 			=gcc
CFLAGS 		=-Wall -g -O3 -fpic -I/usr/lib/erlang/usr/include/
LIB 		=-lm -lzmq -lczmq -lpthread -lc
VALGRIND 	=valgrind

BASE_FILE 	=src/base.c

FILE 		=$(wildcard ./src/*/*.c)
TARGET 		=$(FILE:%.c=%)

C_TARGET 	=$(filter-out $(ERL_TARGET) $(A_TARGET), $(TARGET))

API_FILE 	=$(wildcard ./api/*/*.c)

.erl.beam:
	erlc -Wall -o ebin $<


all:$(TARGET) $(ERL_SRC:%.erl=%.beam)

api:$(patsubst %.c,%.o,$(API_FILE))

$(C_TARGET):$(patsubst %,%.o,$(C_TARGET)) $(BASE_FILE:%.c=%.o)
	$(CC) $@.o $(BASE_FILE:%.c=%.o) -o ./bin/$(subst /,_,$(@:src/%=%)) $(LIB)

$(ERL_TARGET):$(patsubst %.c,%.o,$(BASE_FILE) $(ERL_FILE))
	$(CC) $@.o $(BASE_FILE:%.c=%.o) -o ./bin/$(subst /,_,$(@:src/%=%)) $(LIB) $(ERL_LIB)

$(A_TARGET):api $(patsubst %,%.o,$(A_TARGET))
	$(CC) $@.o $(BASE_FILE:%.c=%.o) $(patsubst src/%,api/%,$(dir $@.c))*.o \
		-o ./bin/$(subst /,_,$(@:src/%=%)) $(LIB)

clean:
	@rm $(wildcard ./src/*.o) $(wildcard ./src/*/*.o) $(wildcard ./api/*/*.o) \
		$(wildcard ./bin/*) $(wildcard ./ebin/*)

test:
	@echo $(A_TARGET)
