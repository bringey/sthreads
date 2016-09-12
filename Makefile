
OBJDIR=obj
BINDIR=bin

CC=			gcc
CFLAGS=		-std=c11 -Wall -Wextra -pedantic -pthread
OBJECTS=	threads.o test.o
DEFINES=	-D_GNU_SOURCE

test: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $+

sthreads: threads.o
	ar -cvr $(BINDIR)/libsthreads.a $+

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@