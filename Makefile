CC=gcc
CFLAGS=-ggdb -lpthread -std=c99

all: crt_effect

crt_effect: crt_effect.c
	$(CC) $(CFLAGS) -o crt_effect crt_effect.c

.PHONY: clean

clean:
	rm -rf *.o crt_effect
strip:
	-s crt_effect
