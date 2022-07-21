CFLAGS = -g -pthread
objects = client.o server.o
CC = gcc
dbg_flags =
DEPS = chatFunctions.h 

server: server.o perm
	./server.o

client: client.o perm
	./client.o

all: $(objects)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $< -o $@

perm:
	chmod +x *.o

debug_%: %.o
	gdb $< $(dbg_flags)

clean:
	rm -f *.o