#!/bin/bash
if [[ -z $1 ]]; then
  source="./testcases/simple.rinha"
else
  source="$1"
fi
basename=$(basename "$source")
program="${basename%.*}"
build="build/${program}.o"
llfile="llvm/${program}.ll"

./bin/vladpiler ${source}
llc --filetype=obj ${llfile} -o ${build}
clang -no-pie ${build} build/rinha_extern.o -o exec
./exec
