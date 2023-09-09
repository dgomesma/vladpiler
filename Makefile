CXX=g++

CXFLAGS=-Wall -O2 -Iinclude

vladpiler: bin/vladpiler
	
bin/vladpiler: build/main.o
	$(CXX) $(CXFLAGS) $< -o $@

build/%.o: src/%.cpp
	$(CXX) $(CXFLAGS) -c $< -o $@
 
.PHONY: clean
clean:
	rm build/*