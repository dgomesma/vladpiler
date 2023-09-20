#include <stdio.h>

void print_bool(char _bool) {
  if (_bool) {
      printf("true\n");
  } else {
      printf("false\n");
    }
}

void print_int(int i) {
  printf("%d\n", i);  
}

void print_str(char* str) {
  printf("%s\n", str);
}

void print_closure(void) {
  printf("<#closure>\n");
}

