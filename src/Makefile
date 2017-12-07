SRCS = lzma/LzmaDec.c
OBJS = $(SRCS:.c=.o) stubicon.o
CC = gcc
BINDIR = $(CURDIR)/../share/ocra

CFLAGS = -Wall -O2 -DWITH_LZMA -Ilzma -s
STUB_CFLAGS = -D_CONSOLE $(CFLAGS)
STUBW_CFLAGS = -mwindows $(CFLAGS)
# -D_MBCS

all: stub.exe stubw.exe edicon.exe

stubicon.o: stub.rc
	windres -i $< -o $@

stub.exe: $(OBJS) stub.o
	$(CC) $(STUB_CFLAGS) $(OBJS) stub.o -o stub

stubw.exe: $(OBJS) stubw.o
	$(CC) $(STUBW_CFLAGS) $(OBJS) stubw.o -o stubw

edicon.exe: edicon.o
	$(CC) $(CFLAGS) edicon.o -o edicon

stub.o: stub.c
	$(CC) $(STUB_CFLAGS) -o $@ -c $<

stubw.o: stub.c
	$(CC) $(STUBW_CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) stub.exe stubw.exe edicon.exe edicon.o stubw.o stub.o

install: stub.exe stubw.exe edicon.exe
	cp -f stub.exe $(BINDIR)/stub.exe
	cp -f stubw.exe $(BINDIR)/stubw.exe
	cp -f edicon.exe $(BINDIR)/edicon.exe
