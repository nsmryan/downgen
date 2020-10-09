

INC := -Ideps/optfetch -Ideps/gifenc
CFLAGS ?= -O3 -g

downgen: deps/optfetch/optfetch.c deps/gifenc/gifenc.c main.c
	$(CC) -o downgen $^ $(INC)

.PHONY: clean
clean:
	rm -rf downgen
