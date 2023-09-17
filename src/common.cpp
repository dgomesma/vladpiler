#include "common.h"

FILE* read_file(const std::string& filepath) {
  if (filepath.empty()) {
    std::cerr << "No source file provided." << std::endl;
    exit(EX_NOINPUT);
  }
  FILE* file = fopen(filepath.data(), "r");
  if (file == NULL) {
    std::cerr << "Error reading " << filepath << ": " << strerror(errno) << std::endl; 
    exit(EX_NOINPUT);
  }
  return file;
}
