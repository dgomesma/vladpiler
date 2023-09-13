#include <iostream>
#include <cxxopts.hpp>
#include <boost/bimap.hpp>

enum class program_t : uint8_t {
  LEXER
};

struct args_t {
   program_t main;
};

boost::bimap<std::string_view, program_t> program_map;

void init_global() {
    program_map.insert({"lexer", program_t::LEXER});
}

void parse_args(int argc, char* argv[], args_t& args) {
  constexpr const char program_arg[] = "program";
  
  cxxopts::Options options_parser(
      "vladpiler",
      "Vladiau's convoluted rinha compiler. Use at your own risk."
    );
  options_parser.add_options()
  (program_arg, "Main program to be run", cxxopts::value<std::string>()->default_value("lexer"));
  auto options = options_parser.parse(argc, argv);

  args.main = program_map.left.at(options[program_arg].as<std::string>());
}

int main(int argc, char* argv[]) {
  args_t args;
  init_global();
  parse_args(argc, argv, args);

  std::cout << "Program: " << program_map.right.at(args.main) << std::endl;
  return 0;
}