CC := clang
LLVM-LIBS := core executionengine native
LLVM-CONFIG := $(llvm-config-18 --cflags --ldflags --libs $(LLVM-LIBS))

CFLAGS := -O0 -g -Isrc/ $(LLVM-CONFIG) -Wall -Wextra
LDFLAGS := 

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
