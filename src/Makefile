SRCS = stub.c lzma/lzmadec.c
OBJS = $(SRCS:.c=.o) stubicon.o
CC = gcc

CFLAGS = -Wall -O2 -DWITH_LZMA -Ilzma -s
# CFLAGS += -D_DEBUG

all: stub.exe stubw.exe edicon.exe

stubicon.o: stub.rc
	windres -i $< -o $@

stub.exe: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o stub

stubw.exe: $(OBJS)
	$(CC) -mwindows $(CFLAGS) $(OBJS) -o stubw

clean:
	rm -f $(OBJS) stub.exe stubw.exe edicon.exe edicon.o

edicon.exe: edicon.o
	$(CC) $(CFLAGS) edicon.o -o edicon

