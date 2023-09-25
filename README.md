# Vladpiler
**Project progress:** Still missing major features :( It's been a pain using
LLVM to compile a dynamically typed language, so I'm working on some of these
features (e.g. functions, nested tuples, and let). But you can try
some things.

## Building docker
```
docker build -t vladpiler .
```

## Compiling the Vladpiler
```
make install
```

## Compiling the Rinha files
```
make llvm
```

The rinha files must be in testcases.

## Running manually
```
./bin/vladpiler rinha_source
```

This will cause the file to be compiled to LLVM IR and placed in the llvm
directory with the same name. Further compilation of these files can be done
with make

You may also run it with `--help` for some help options. You can use `--program`
to try out its lexer (I used for debugging purposes) like so: `bin/vladpiler
--program lexer rinha_source`

## Notes
I spent too much time trying to hack type inference after I discovered about
the fact that all pointer types are _opaque_, and getting the types of pointers
is **painful**. For this reason, although the project started with good foundations,
it was rushed near the end and there is plenty of room for improvement in the
back-end (which was supposed to be the main thing, but oh well).

## Fetching Testcases
Make sure you have Pyhton 3 installed and all necessary packages. Then run
`./scripts/fetch-rinha-files.py`.
