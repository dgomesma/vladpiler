#!/bin/bash
program="print"
source="testcases/${program}.rinha"
build="build/${program}.o"
llfile="llvm/${program}.ll"

./bin/vladpiler ${source}
llc --filetype=obj ${llfile} -o ${build}
clang -no-pie ${build} build/rinha_extern.o -o exec
./exec
