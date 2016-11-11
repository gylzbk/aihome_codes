CC = mipsel-linux-gcc

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.c.o, $(SRCS))

libflash.a: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -f $(OBJS) libflash.a

.PHONY: clean libflash.a
