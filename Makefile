LLVMCONFIG=llvm-config
LFLAGS=-lfl
CXX=g++

ifeq ($(release), 1)
	DFLAGS=-O2
else
	DFLAGS=-O0 -g
endif

CXFLAGS=-Wall -Wno-unused-function $(DFLAG) -Iinclude `$(LLVMCONFIG) --system-libs --libs` $(LFLAGS)

OBJS=build/main.o build/parser.tab.o build/lexer.lex.o build/lexer.o

vladpiler: bin/vladpiler
	
bin/vladpiler: parse_src $(OBJS)
	$(CXX) $(CXFLAGS) $(LIBS) $(OBJS) -o $@

.PHONY:
parse_src: src/parser.tab.cpp src/lexer.lex.cpp

build/%.o: src/%.cpp
	$(CXX) $(CXFLAGS) -c $< -o $@

src/%.tab.cpp: src/%.y
	bison -o $@ --header=include/$*.tab.h $<

src/%.lex.cpp: src/%.l
	flex -o $@ $<
 
.PHONY: clean
clean:
	rm -f build/* **/*.tab.* **/*.lex.*

