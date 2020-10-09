

INC := -Ideps/optfetch -Ideps/gifenc
CFLAGS ?= -O3 -g

downgen: deps/optfetch/optfetch.c deps/gifenc/gifenc.c
	$(CC) main.c -o downgen $^ $(INC)
