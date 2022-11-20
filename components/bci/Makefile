CC=clang -Ofast
CFLAGS=-pedantic 
LDFLAGS=

SRC_OBJECTS=src/bci.o src/block.o src/block-builder.o src/buffer.o src/memory.o src/op.o
SRC_MAIN_OBJECTS=src/main.o
SRC_TARGETS=src/main

TEST_OBJECTS=test/minunit.o test/scenario-runner.o test/suite-block.o
TEST_MAIN_OBJECTS=test/test-main.o
TEST_TARGETS=test/test-runner

.PHONY: all clean
all: $(SRC_TARGETS) $(TEST_TARGETS)
	./test/test-runner

./src/main: $(SRC_OBJECTS) src/main.o
	$(CC) $(LDFLAGS) -o $@ $^

./test/test-runner: $(SRC_OBJECTS) $(TEST_OBJECTS) test/test-runner.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c ./src/*.h ./test/*.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJECTS) $(SRC_TARGETS) $(TEST_OBJECTS) $(TEST_TARGETS) $(SRC_MAIN_OBJECTS) $(TEST_MAIN_OBJECTS)
