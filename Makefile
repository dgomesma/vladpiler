LLVMCONFIG=llvm-config
LFLAGS=-lfl
CC=gcc
CXX=g++
LLC=llc
VLAD=bin/vladpiler

DFLAG=-O2

CXFLAGS=-Wall -Wno-unused-variable -Wno-unused-function $(DFLAG) -Iinclude `$(LLVMCONFIG) --system-libs --libs` $(LFLAGS)

OBJS=build/main.o build/parser.tab.o build/lexer.lex.o build/lexer.o build/compiler.o build/common.o build/rinha_extern.o
RINHA_FILES := $(wildcard testcases/*.rinha)
LL_BIN := $(patsubst testcases/%.rinha,bin/%,$(RINHA_FILES))

vladpiler: $(VLAD)

.PHONY: llvm
llvm: $(LL_BIN)
	@echo $(LL_BIN)
	
bin/vladpiler: parse_src $(OBJS)
	$(CXX) $(CXFLAGS) $(LIBS) $(OBJS) -o $@

.PHONY:
parse_src: src/parser.tab.cpp src/lexer.lex.cpp

bin/%: build/%.o build/rinha_extern.o
	clang -no-pie $^ -o $@

llvm/%.ll: testcases/%.rinha
	$(VLAD) $^

build/%.o: src/%.c
	$(CC) $(CXFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	$(CXX) $(CXFLAGS) -c $< -o $@

build/%.o: llvm/%.ll
	$(LLC) --filetype=obj $< -o $@

src/%.tab.cpp: src/%.y
	bison -o $@ --header=include/$*.tab.h $<

src/%.lex.cpp: src/%.l
	flex -o $@ $<
 
.PHONY: clean
clean:
	rm -f build/* **/*.tab.* **/*.lex.* llvm/*.ll

