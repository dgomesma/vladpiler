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

  llvm::Value* Int::getVal() {
    return Compiler::context->llvm_builder.getInt64(value);
  }

  Str::Str(std::string* _str) : str(_str) {}

  llvm::Value* Str::getVal() {
    std::string* s = str.get();
    llvm::GlobalVariable* tr = Compiler::context->llvm_builder
      .CreateGlobalString(*str.get(), "", 0, Compiler::context->llvm_module.get());
    // Left over from here
  }

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

  Context::Context(const std::string& input_file, const std::string& output_file) :
    llvm_builder(llvm_context),
    llvm_module(new llvm::Module(input_file, llvm_context)),
    filename(input_file) {
    std::error_code fd_ostream_ec;
    ostream = std::make_unique<llvm::raw_fd_ostream>(output_file, fd_ostream_ec);
  };

  void Context::beginCodegen() {
    llvm::FunctionType* ft = llvm::FunctionType::get(
      llvm_builder.getInt32Ty(),
      false
    );

    main_fn = llvm::Function::Create(
      ft,
      llvm::Function::ExternalLinkage,
      "main",
      llvm_module.get()
    );

    if (main_fn == nullptr) {
      std::cerr << "Error creating main function!" << std::endl;
      abort();
    }

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(
      llvm_context,
      "entry",
      main_fn  
    );

    llvm_builder.SetInsertPoint(bb);       
  }

  void Context::endCodegen() {
    llvm_builder.CreateRet(llvm_builder.getInt32(0));
  }

  void Context::printOut() {
    llvm::verifyFunction(*main_fn);
    
    llvm_module->print(*ostream, nullptr);
  }

  int compile(const std::string& input_file, const std::string& output_file) {
    yyin = read_file(input_file);
    context = new Context(input_file, output_file);
    context->beginCodegen();
    
    int ret = yyparse();
    if (ret != 0) {
      std::cerr << "Error while parsing. yyparse error: " << ret << std::endl;
      exit(EXIT_FAILURE);
    }
    context->endCodegen();
    context->printOut();
    delete context;
    return EXIT_SUCCESS;
  }
}