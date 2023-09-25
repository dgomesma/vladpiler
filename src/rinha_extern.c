#include <stdio.h>
#include <stdint.h>

void print_undefined() {
  printf("undefined");
}

void print_bool(uint8_t val) {
  if (val) printf("true");
  else printf("false");
}

void print_num(int32_t val) {
  printf("%d", val);
}


void print_str(char* str) {
  if (!str) print_undefined();
  else printf("%s", str);
}


void print_closure(void) {
  printf("<#closure>");
}

void print_lp(void) {
  printf("(");
}

void print_delim(void) {
  printf(", ");
}

void print_rp(void) {
  printf(")");
}

void print_nl(void) {
  printf("\n");
}