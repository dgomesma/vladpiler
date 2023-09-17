#include <iostream>
#include <cxxopts.hpp>
#include <boost/bimap.hpp>
#include "common.h"
#include "lexer.h"
#include "compiler.h"

constexpr const char lexer_str[] = "lexer";
constexpr const char comp_str[] = "compiler";

enum class program_t : uint8_t {
  LEXER, COMPILER
};

struct args_t {
  program_t main;
  std::string filename;
};

boost::bimap<std::string_view, program_t> program_map;

void init_global() {
  program_map.insert({lexer_str, program_t::LEXER});
  program_map.insert({comp_str, program_t::COMPILER});
}

void parse_args(int argc, char* argv[], args_t& args) {
  constexpr const char prog_arg[] = "program";
  constexpr const char src_arg[] = "source"; 
  constexpr const char help_arg[] = "help";
  
  cxxopts::Options options_parser(
      "vladpiler",
      "Vladiau's convoluted rinha compiler. The writer of this program does not "
      "assume any responsibility for any physical or psychological damage, "
      "triggers, traumas, cringe, or segfaults caused by this program. Use at "
      "your sole discretion and risk."
    );
  options_parser.add_options()
  (prog_arg, "Main program to be run", cxxopts::value<std::string>()->default_value(comp_str))
  (src_arg, "Source file to read from", cxxopts::value<std::string>()->default_value(""))
  (help_arg, "Print help message in case you're not based enough to guess the "
    "arguments for this program by sole intuition and divine clairvoyance.");
  auto options = options_parser.parse(argc, argv);

  if (options.count("help")) {
    std::cout << options_parser.help() << std::endl;
    exit(EXIT_SUCCESS);
  }

  args.main = program_map.left.at(options[prog_arg].as<std::string>());
  args.filename= std::move(options[src_arg].as<std::string>());
}

int main(int argc, char* argv[]) {
  args_t args;
  init_global();
  parse_args(argc, argv, args);

  switch (args.main) {
    case program_t::LEXER:
      Lexer::tokens_scanner(args.filename);
      break;
    case program_t::COMPILER:
      Compiler::compile(args.filename);
      break;
    default:
      break;
  }

  return 0;
}