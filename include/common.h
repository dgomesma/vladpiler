#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <map>

// C Libraries
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sysexits.h>

FILE* read_file(const std::string& filepath);

#endif