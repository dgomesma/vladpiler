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

  void File::Compile() {
    linkExternPrint();

    llvm::FunctionType* ft = llvm::FunctionType::get(
      Compiler::ctx->llvm_builder.getInt32Ty(),
      false
    );

    llvm::Function* main_fn = llvm::Function::Create(
      ft,
      llvm::Function::ExternalLinkage,
      "main",
      Compiler::ctx->llvm_module.get()
    );

    if (main_fn == nullptr) {
      std::cerr << "Error creating main function!" << std::endl;
      abort();
    }

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(
      Compiler::ctx->llvm_context,
      "entry",
      main_fn  
    );

    Compiler::ctx->llvm_builder.SetInsertPoint(bb);   
    term->getVal();
    Compiler::ctx->llvm_builder.CreateRet(Compiler::ctx->llvm_builder.getInt32(0));
    llvm::verifyFunction(*main_fn);
  };

  void File::linkExternPrint() {
    std::vector<llvm::Type *> print_bool_params_ty = {Compiler::ctx->llvm_builder.getInt1Ty()};
    llvm::FunctionType* print_bool_fn_ty= llvm::FunctionType::get(
      Compiler::ctx->llvm_builder.getInt1Ty(),
      print_bool_params_ty,        
      false
    );

    std::vector<llvm::Type *> print_int_params_ty = {Compiler::ctx->llvm_builder.getInt32Ty()};
    llvm::FunctionType* print_int_fn_ty= llvm::FunctionType::get(
      Compiler::ctx->llvm_builder.getInt32Ty(),
      print_int_params_ty,        
      false
    );    

    std::vector<llvm::Type *> print_str_params_ty = {Compiler::ctx->llvm_builder.getInt8PtrTy()};
    llvm::FunctionType* print_str_fn_ty= llvm::FunctionType::get(
      Compiler::ctx->llvm_builder.getInt8PtrTy(),
      print_str_params_ty,        
      false
    );

    std::vector<llvm::Type *> print_closure_params_ty = {Compiler::ctx->llvm_builder.getVoidTy()};
    llvm::FunctionType* print_closure_fn_ty= llvm::FunctionType::get(
      Compiler::ctx->llvm_builder.getVoidTy(),
      print_str_params_ty,        
      false
    );    

    llvm::Function *print_bool_fn = llvm::Function::Create(
      print_bool_fn_ty,
      llvm::Function::ExternalLinkage,
      "print_bool",
      Compiler::ctx->llvm_module.get()
    );

  
    llvm::Function *print_int_fn = llvm::Function::Create(
      print_int_fn_ty,
      llvm::Function::ExternalLinkage,
      "print_int",
      Compiler::ctx->llvm_module.get()
    );

    
    llvm::Function *print_str_fn = llvm::Function::Create(
      print_str_fn_ty,
      llvm::Function::ExternalLinkage,
      "print_str",
      Compiler::ctx->llvm_module.get()
    );

    llvm::Function *print_closure_fn = llvm::Function::Create(
      print_closure_fn_ty,
      llvm::Function::ExternalLinkage,
      "print_str",
      Compiler::ctx->llvm_module.get()
    );
  }  

  Int::Int(int64_t _value) : value(_value) {}

  llvm::Value* Int::getVal() {
    return Compiler::ctx->llvm_builder.getInt64(value);
  }

  Str::Str(std::string* _str) : str(_str) {}

  llvm::Value* Str::getVal() {
    llvm::GlobalVariable* str_var = Compiler::ctx->llvm_builder
      .CreateGlobalString(*str.get(), "", 0, Compiler::ctx->llvm_module.get());
    return Compiler::ctx->llvm_builder
      .CreateConstGEP2_32(str_var->getValueType(), str_var, 0, 0);
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
  Context* ctx;
  std::unique_ptr<AST::File> ast_root;

  Context::Context(const std::string& input_file, const std::string& output_file) :
    llvm_builder(llvm_context),
    llvm_module(new llvm::Module(input_file, llvm_context)),
    filename(input_file) {
    std::error_code fd_ostream_ec;
    ostream = std::make_unique<llvm::raw_fd_ostream>(output_file, fd_ostream_ec);
  };

  void Context::printOut() {
    llvm_module->print(*ostream, nullptr);
  }

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