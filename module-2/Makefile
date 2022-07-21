CFLAGS = -g -pthread
objects = client.o server.o chatFunctions.o
CC = gcc
dbg_flags =
DEPS = chatFunctions.h
inc= chatFunctions.c


server: server.o perm
	./server.o

client: client.o perm
	./client.o

all: $(objects)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(inc) $< -o $@

perm:
	chmod +x *.o

debug_%: %.o
	gdb $< $(dbg_flags)

clean:
	rm -f *.o