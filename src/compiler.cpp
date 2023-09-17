#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "parser.tab.h"

llvm::Module* llvm_module;
llvm::LLVMContext llvm_context;
SymbolTableStack symtbl_stack;

//==================================
// Symbol Table
//==================================

SymbolTableStack::SymbolTableStack() {
  pushScope();
}

Descriptor* SymbolTableStack::getDescriptor(const std::string& symbol) const {
  for (const SymbolTable& table : symbol_tables) {
    SymbolTable::const_iterator descriptor = table.find(symbol);
    if (descriptor != table.end()) return descriptor->second;
  }
  return nullptr;
}

Descriptor* SymbolTableStack::getDescriptorInCurScope(const std::string& symbol) const {
  const SymbolTable& cur_scope = symbol_tables.front();
  SymbolTable::const_iterator descriptor = cur_scope.find(symbol);
  if (descriptor != cur_scope.end()) return descriptor->second;
  return nullptr;
}

void SymbolTableStack::insertDescriptor(const std::string& symbol, Descriptor* descriptor) {
  SymbolTable& cur_scope = symbol_tables.front();
  cur_scope[symbol] = descriptor;
}

void SymbolTableStack::pushScope() {
  symbol_tables.emplace_back();
}

void SymbolTableStack::popScope() {
  symbol_tables.pop_back();
}

int compiler(const std::string& filename) {
  yyin = read_file(filename);
  int ret = yyparse();
  if (ret != 0) {
    std::cerr << "Error while parsing: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}