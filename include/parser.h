#ifndef _PARSER_H_
#define _PARSER_H_

#include "common.h"

namespace Parser {
  struct Localization {
    uint64_t start;
    uint64_t end;
    std::string filename;

    Localization(uint64_t _start, uint64_t _end, std::string _filename);
  };

}

#endif