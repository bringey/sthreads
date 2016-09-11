
DEFINES=

DEFINES+=-D_GNU_SOURCE

CC=		gcc
CFLAGS=		-std=c11 -Wall -Wextra -pedantic -pthread
OBJECTS=	threads.o test.o

test: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $+ 

%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@