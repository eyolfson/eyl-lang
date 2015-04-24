CC := clang
CFLAGS := -std=c11 -O2

build/bin/assembler: build/cache/main.o build/bin
	$(CC) $(LDFLAGS) $< -o $@

build/cache/main.o: src/main.c build/cache
	$(CC) $(CFLAGS) src/main.c -c -o $@

build/bin: build
	mkdir build/bin

build/cache: build
	mkdir build/cache

build:
	mkdir build

.PHONY: clean
clean:
	rm -r build
