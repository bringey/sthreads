
OBJDIR=obj
BINDIR=bin
INCDIR=include
SRCDIR=src

CC=			gcc
CFLAGS=		-std=c11 -Wall -Wextra -pedantic -pthread
DEFINES=	-D_GNU_SOURCE

LIB=$(BINDIR)/libsthreads.a

TESTS=oncetest test

# ==============================================================================
# library target

all: sthreads

$(LIB): obj/threads.o
	ar -cvr $(BINDIR)/libsthreads.a $+

sthreads: $(LIB)

# =============================================================================
# test programs

tests: $(TESTS)

test: obj/test.o $(LIB)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $+

oncetest: obj/oncetest.o $(LIB)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $+
	
# =============================================================================	
# utils

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o

realclean: clean
	rm -f $(LIB)
	rm -f $(addprefix $(BINDIR)/,$(TESTS))
#	rm -f $(BINDIR)/test
