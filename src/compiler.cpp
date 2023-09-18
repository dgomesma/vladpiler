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
  File::File(const std::string& _filename, std::unique_ptr<Term> _term)
    : filename(_filename), term(std::move(_term)) {}

  Int::Int(int64_t _value) : value(_value) {}

  Str::Str(std::string&& _str) : str(std::move(_str)) {}
  
  Call::Call(std::string&& _callee, std::vector<Term>&& _args) :
    callee(std::move(_callee)), args(std::move(_args)) {} 
  
  Binary::Binary(std::unique_ptr<Term> _lhs, std::unique_ptr<Term> _rhs, BinOp _binop) :
    lhs(std::move(_lhs)), rhs(std::move(_rhs)), binop(_binop) {}

  Parameter::Parameter(std::string&& id, Descriptor* _descriptor) :
    identifier(id), descriptor(_descriptor) {}

  Parameters::Parameters(std::vector<Parameter>&& _params) : 
    params(std::move(_params)) {} 

  Parameters::Parameters(Parameters&& _params) :
    params(std::move(_params.params)) {}

  Function::Function(Parameters&& _parameters, std::unique_ptr<Term> _value) :
    parameters{std::move(_parameters)}, value(std::move(_value)) {}

  Let::Let(Parameter&& _parameter, std::unique_ptr<Term> _val, std::unique_ptr<Term> _next) :
    parameter(std::move(_parameter)), val(std::move(_val)), next(std::move(_next)) {}

  If::If(std::unique_ptr<Term> _condition, std::unique_ptr<Term> _then, std::unique_ptr<Term> _orElse) :
    condition(std::move(_condition)), then(std::move(_then)), orElse(std::move(_orElse)) {}

  Print::Print(std::unique_ptr<Term> _arg) :
    arg(std::move(_arg)) {}

  First::First(std::unique_ptr<Term> _arg) :
    arg(std::move(_arg)) {}

  Second::Second(std::unique_ptr<Term> _arg) :
    arg(std::move(_arg)) {}

  Tuple::Tuple(std::unique_ptr<Term> _first, std::unique_ptr<Term> _second) :
    first(std::move(_first)), second(std::move(_second)) {}

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
      std::cerr << "Error while parsing: " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }

    context->printOut();
    delete context;
    return EXIT_SUCCESS;
  }
}