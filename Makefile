CC=clang++
CXXFLAGS=-g -Wall
LDFLAGS=$(shell llvm-config --cxxflags --ldflags --system-libs --libs core)
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
	$(CC) -c $(CXXFLAGS) $< -o $@

$(OBJNAME): $(OBJS) $(BUILD_DIR)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(OBJNAME)

clean:
	rm -rf $(OBJS) $(OBJNAME)

test: $(OBJNAME)
	@bash ./run_test
	./$(OBJNAME) $(TESTFILE)
