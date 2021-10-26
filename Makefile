CC=g++
CFLAGS=-g -Wall
SRC=$(wildcard *.cc)
HEADERS=$(wildcard *.h)
OBJS=$(SRC:.cc=.o)
OBJNAME=clox
.PHONEY=clean test all
TESTFILE=testfile

all: $(OBJNAME)

token_convert.h: token.h
	python gen_reserve.py

token.cc: token_convert.h

%.o: %.cc $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJNAME): $(OBJS) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(OBJNAME)

clean:
	rm -rf $(OBJS) $(OBJNAME)

test: $(OBJNAME)
	./$(OBJNAME) $(TESTFILE)
