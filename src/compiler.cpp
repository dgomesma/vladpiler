#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "parser.tab.h"

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

namespace AST{
  File::File(const std::string& _filename, Term* _term)
    : filename(_filename), term(_term) {}

  Int::Int(int64_t _value) : value(_value) {}

  Str::Str(std::string* _str) : str(_str) {}

  Arguments::Arguments() = default;
  
  Call::Call(std::string* _callee, Arguments* _args) :
    callee(*_callee), args(_args) {} 
  
  Binary::Binary(Term* _lhs, Term* _rhs, BinOp _binop) :
    lhs(_lhs), rhs(_rhs), binop(_binop) {}

  Parameter::Parameter(std::string* id) :
    identifier(id) {}

  Parameter::Parameter(Parameter* parameter) :
    identifier(std::move(parameter->identifier)) {}

  Parameters::Parameters() = default;

  Function::Function(Parameters* _parameters, Term* _value) :
    parameters(_parameters), value(std::move(_value)) {}

  Let::Let(Parameter* _parameter, Term* _val, Term* _next) :
    parameter(_parameter), val(_val), next(_next) {}

  If::If(Term* _condition, Term* _then, Term* _orElse) :
    condition(_condition), then(_then), orElse(_orElse) {}

  Print::Print(Term* _arg) :
    arg(_arg) {}

  First::First(Term* _arg) :
    arg(_arg) {}

  Second::Second(Term* _arg) :
    arg(_arg) {}

  Bool::Bool(bool _val) :
    val(_val) {};

  Tuple::Tuple(Term* _first, Term* _second) :
    first(_first), second(_second) {}

  Var::Var(std::string&& _name) :
    name(std::move(_name)) {};
}

namespace Compiler {
  Context* context;

  Context::Context(const std::string& _filename) :
    llvm_module(new llvm::Module(_filename, llvm_context)),
    filename(_filename) {
    std::error_code fd_ostream_ec;
    ostream = std::make_unique<llvm::raw_fd_ostream>(_filename, fd_ostream_ec);
  };

  void Context::printOut() {
    llvm_module->print(*ostream, nullptr);
  }

  int compile(const std::string& _filename) {
    yyin = read_file(_filename);
    context = new Context(_filename);
  
    int ret = yyparse();
    if (ret != 0) {
      std::cerr << "Error while parsing. yyparse error: " << ret << std::endl;
      exit(EXIT_FAILURE);
    }

    context->printOut();
    delete context;
    return EXIT_SUCCESS;
  }
}