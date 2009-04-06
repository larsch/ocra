SRCS = stub.c lzma/lzmadec.c
OBJS = $(SRCS:.c=.o)
CC = gcc

CFLAGS = -Wall -O2 -DWITH_LZMA -Ilzma
# CFLAGS += -D_DEBUG

all: stub.exe

stub.exe: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -s -o stub

clean:
	rm $(OBJS)
