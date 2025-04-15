EXE = mvmasm
INCLUDE_DIRS = -I../src
CFLAGS = -std=c99 -pedantic -Wall -MMD -MP $(INCLUDE_DIRS) -g
LDFLAGS = 
SRCS = $(shell find src -name *.c)
OBJS = $(SRCS:%=build/%.o)
DEPS = $(OBJS:.o=.d)

all: bin/$(EXE)

bin/$(EXE): $(OBJS)
	mkdir -p bin
	$(CC) $^ -o $@ $(LDFLAGS)

build/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: run clean

run: bin/$(EXE)
	./bin/$(EXE)

clean:
	rm -rf bin build

-include $(DEPS)
