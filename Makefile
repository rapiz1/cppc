CC=g++
CFLAGS=-g
SRC=$(wildcard *.cc)
OBJS=$(SRC:.cc=.o)
OBJNAME=clox
.PHONEY=clean test
TESTFILE=testfile

token_convert.h: token.h
	python gen_reserve.py

token.cc: token_convert.h

%.o: %.cc
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJNAME): $(OBJS) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(OBJNAME)

clean:
	rm -rf $(OBJS) $(OBJNAME)

test: $(OBJNAME)
	./$(OBJNAME) $(TESTFILE)
