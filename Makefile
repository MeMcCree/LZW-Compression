CC=gcc
CFLAGS=-std=c11 -O2
CTARGET=main.exe

clean: main.exe
	del *.exe

ndeb: main.c
	$(CC) $(CFLAGS) -o $(CTARGET) $^

deb: main.c
	$(CC) $(CFLAGS) -D_DEBUG -o $(CTARGET) $^