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
    Compiler::IRGenerator& generator = Compiler::IRGenerator::getSingleton();
    return generator.createInt(value);
  }

  Str::Str(std::string* _str) : str(_str) {}

  llvm::Value* Str::getVal() {
    Compiler::IRGenerator& generator = Compiler::IRGenerator::getSingleton();
    return generator.createStr(*str);
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
  std::unique_ptr<AST::File> ast_root;

  IRGenerator* IRGenerator::singleton = nullptr;

  IRGenerator::IRGenerator(const std::string& input_file) :
    builder(context),
    module(input_file, context),
    filename(input_file) {};

  bool IRGenerator::isInitialized() { return singleton != nullptr; }

  IRGenerator& IRGenerator::initialize(const std::string& input_file) {  
    if (isInitialized()) throw std::runtime_error("IRGenerator is already initialized.");
    singleton = new IRGenerator(input_file);
    IRGenerator& generator = *singleton;
    generator.externInsertPoint = generator.builder.saveIP();
    generator.createFunction(llvm::Type::getInt32Ty(generator.context), {}, "main");
    
    return *IRGenerator::singleton;
  };

  IRGenerator& IRGenerator::getSingleton() {
    if (isInitialized()) throw std::runtime_error("IRGenerator has not been initialized.");
    return *singleton;
  }

  void IRGenerator::printCode(const std::string& out_file) {
    std::error_code fd_ostream_ec;
    llvm::raw_fd_ostream ostream(out_file, fd_ostream_ec);
    module.print(ostream, nullptr);
  };

  llvm::Function* IRGenerator::createFunction(llvm::Type* ret, const std::vector<llvm::Type*>& args, const std::string& name) {
    llvm::FunctionType* fn_type = llvm::FunctionType::get(ret, args, false);
    llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, name, module);
    llvm::BasicBlock* fn_entry = llvm::BasicBlock::Create(context, "entry", fn);
    builder.SetInsertPoint(fn_entry);
    return fn;
  }

  llvm::Function* IRGenerator::createFunction(llvm::Type* type, std::initializer_list<llvm::Type*> _args, const std::string& name) {
    const std::vector<llvm::Type*> args(_args);
    return createFunction(type, args, name);
  };  

  llvm::Function* IRGenerator::declareExternFunction(llvm::Type* ret, std::initializer_list<llvm::Type*>&& args, const std::string& name) {
    llvm::IRBuilder<>::InsertPoint previous_point= builder.saveIP();
    llvm::Function* extern_fn = createFunction(ret, args, name);
    builder.restoreIP(previous_point);
    return extern_fn;
  };

  llvm::Value* IRGenerator::createInt(int32_t value) {
    return builder.getInt32(value);
  }

  llvm::Value* IRGenerator::createStr(const std::string& str) {
    llvm::GlobalVariable* str_var = builder.CreateGlobalString(str, "", 0, &module);
    return builder.CreatePointerCast(str_var, builder.getInt8PtrTy());
  }

  int compile(const std::string& input_file, const std::string& output_file) {
    yyin = read_file(input_file);
    IRGenerator& generator = IRGenerator::initialize(input_file);
    
    int ret = yyparse();
    if (ret != 0) {
      std::cerr << "Error while parsing. yyparse error: " << ret << std::endl;
      exit(EXIT_FAILURE);
    }

    generator.printCode(output_file);
    return EXIT_SUCCESS;
  }
}