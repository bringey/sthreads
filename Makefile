
OBJDIR=obj
BINDIR=bin
INCDIR=include
SRCDIR=src

CC=			gcc
CFLAGS=		-std=c11 -Wall -Wextra -pedantic -pthread
DEFINES=	-D_GNU_SOURCE

LIB=$(BINDIR)/libsthreads.a

$(LIB): obj/threads.o
	ar -cvr $(BINDIR)/libsthreads.a $+

#$(BINDIR)/libsthreads.a: CFLAGS += -pthread

sthreads: $(LIB)

test: obj/test.o $(LIB)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $+
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) -I$(INCDIR) -c $< -o $@

	
# utils

clean:
	rm -f obj/*.o

realclean: clean
	rm -f $(LIB)
	rm -f $(BINDIR)/test