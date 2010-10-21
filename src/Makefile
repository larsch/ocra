SRCS = lzma/lzmadec.c
OBJS = $(SRCS:.c=.o) stubicon.o
CC = gcc
BINDIR = $(CURDIR)/../share/ocra

CFLAGS = -Wall -O2 -DWITH_LZMA -Ilzma -s
DEBUG_CFLAGS = -D_DEBUG
STUB_CFLAGS = -D_CONSOLE $(CFLAGS)
STUBW_CFLAGS = -mwindows $(CFLAGS)
# -D_MBCS

all: stub.exe stubw.exe stubd.exe stubdw.exe edicon.exe

stubicon.o: stub.rc
	windres -i $< -o $@

stub.exe: $(OBJS) stub.o
	$(CC) $(STUB_CFLAGS) $(OBJS) stub.o -o stub

stubw.exe: $(OBJS) stubw.o
	$(CC) $(STUBW_CFLAGS) $(OBJS) stubw.o -o stubw

stubd.exe: $(OBJS) stubd.o
	$(CC) $(DEBUG_CFLAGS) $(STUB_CFLAGS) $(OBJS) stubd.o -o stubd

stubdw.exe: $(OBJS) stubdw.o
	$(CC) $(DEBUG_CFLAGS) $(STUBW_CFLAGS) $(OBJS) stubdw.o -o stubdw

edicon.exe: edicon.o
	$(CC) $(CFLAGS) edicon.o -o edicon

stub.o: stub.c
	$(CC) $(STUB_CFLAGS) -o $@ -c $<

stubw.o: stub.c
	$(CC) $(STUBW_CFLAGS) -o $@ -c $<

stubd.o: stub.c
	$(CC) $(DEBUG_CFLAGS) $(STUB_CFLAGS) -o $@ -c $<

stubdw.o: stub.c
	$(CC) $(DEBUG_CFLAGS) $(STUBW_CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) stub.exe stubw.exe stubd.exe stubdw.exe edicon.exe edicon.o stubw.o stub.o stubd.o stubdw.o

install: stub.exe stubw.exe stubd.exe stubdw.exe edicon.exe
	cp -f stub.exe $(BINDIR)/stub.exe
	cp -f stubw.exe $(BINDIR)/stubw.exe
	cp -f stubd.exe $(BINDIR)/stubd.exe
	cp -f stubdw.exe $(BINDIR)/stubdw.exe
	cp -f edicon.exe $(BINDIR)/edicon.exe
