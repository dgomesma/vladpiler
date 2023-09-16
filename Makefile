LLVMCONFIG=llvm-config
LIBS=-lfl
CXX=g++
CXFLAGS=-Wall -O2 -Iinclude `$(LLVMCONFIG) --system-libs --libs` $(LIBS)

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

