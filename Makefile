CC := clang
LLVM-LIBS := core executionengine native
LLVM-CFLAGS := $(shell llvm-config-18 --cflags)
LLVM-LDFLAGS := $(shell llvm-config-18 --ldflags --libs $(LLVM-LIBS))

CFLAGS := -O0 -g -Isrc/ $(LLVM-CFLAGS) -Wall -Wextra
LDFLAGS := $(LLVM-LDFLAGS)

override SRCFILES := $(shell find -L * -type f | LC_ALL=C sort)
override CFILES := $(filter %.c,$(SRCFILES))
override OBJS := $(addprefix ,$(CFILES:.c=.c.o))

.PHONY: all
all: paxc

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

paxc: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	@rm $(OBJS) paxc
