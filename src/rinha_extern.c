#include <stdio.h>
#include <stdint.h>

void _print_undefined() {
  printf("undefined");
}

void print_undefined() {
  _print_undefined();
  printf("\n");
}

void _print_bool(uint8_t val) {
  if (val) printf("true");
  else printf("false");
}

void print_bool(uint8_t val) {
  _print_bool(val);
  printf("\n");
}

void _print_num(int32_t val) {
  printf("%d", val);
}

void print_num(int32_t val) {
  _print_num(val);
  printf("\n");
}

void _print_str(char* str) {
  if (!str) _print_undefined();
  else printf("%s", str);
}

void print_str(char* str) {
  _print_str(str);
  printf("\n");
}

void _print_closure(void) {
  printf("<#closure>");
}

void print_closure(void) {
  _print_closure();
  printf("\n");
}