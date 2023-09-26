#!/bin/bash

./bin/vladpiler testcases/simple.rinha
llc --filetype=obj llvm/simple.ll -o build/simple.o
clang -no-pie build/simple.o build/rinha_extern.o -o exec
./exec
