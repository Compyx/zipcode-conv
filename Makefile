# vim: set noet ts=8:
VPATH=src:tests

LD=$(CC)

INSTALL_PREFIX=/usr/local

# -Wformat-signedness not in GCC < 5.0??
CFLAGS=-Wall -Wextra -pedantic -std=c99 -Wshadow -Wpointer-arith \
	-Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes \
	-Wswitch-default -Wswitch-enum -Wuninitialized -Wconversion \
	-Wredundant-decls -Wnested-externs -Wunreachable-code -Wformat \
	-g -O3 \
	-DDEBUG_ZCC -DDEBUG_CMDLINE -DDEBUG_UNITTEST


BIN_PROG = zipcode-conv
BIN_TEST = unit_tests

all: $(BIN_PROG) $(BIN_TEST)

BASE_OBJS = cmdline.o cbmdos.o errors.o mem.o io.o strlist.o petasc.o
PROG_OBJS = d64.o rle.o zipdisk.o $(BASE_OBJS)
TEST_OBJS = unit.o $(BASE_OBJS) \
	    test_unittest.o


DOCS = doc/doxygen


.PHONY: doc
doc:
	doxygen 1>/dev/null

.PHONY: clean
clean:
	rm -f $(BASE_OBJS) $(PROG_OBJS) $(TEST_OBJS) main.o unit_tests.o
	rm -f $(BIN_PROG) $(BIN_TEST)
	rm -rfd $(DOCS)/html/*
	rm -f *.html

install:
	cp $(TARGET) $(INSTALL_PREFIX)/bin


# generic rule to build objects from sourcefiles
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_PROG): main.o $(PROG_OBJS) $(BASE_OBJS)
	$(LD) -o $@ $^

$(BIN_TEST): unit_tests.o $(TEST_OBJS) $(BASE_OBJS)
	$(LD) -o $@ $^



