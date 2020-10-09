

INC := -Ideps/optfetch -Ideps/gifenc
CFLAGS ?= -O0 -g

downgen: deps/optfetch/optfetch.c deps/gifenc/gifenc.c main.c
	$(CC) $(CFLAGS) -o downgen $^ $(INC)

.PHONY: clean
clean:
	rm -rf downgen
