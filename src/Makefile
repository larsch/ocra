SRCS = stub.c lzma/lzmadec.c
OBJS = $(SRCS:.c=.o)
CC = gcc

CFLAGS = -Wall -O2 -DWITH_LZMA -Ilzma
# CFLAGS += -D_DEBUG

all: stub.exe stubw.exe

stub.exe: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -s -o stub

stubw.exe: $(OBJS)
	$(CC) -mwindows $(CFLAGS) $(OBJS) -s -o stubw

clean:
	rm -f $(OBJS)
