#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "parser.tab.h"

//==================================
// Symbol Table
//==================================

namespace AST{
  File::File(const std::string& _filename, Term* _term)
    : filename(_filename), term(_term) {}

  void File::compile() {
    term->getVal();
    uint32_t ret_val = 0;
    Compiler::RinhaCompiler::getSingleton().createReturn(ret_val);
  };

  Int::Int(int64_t _value) : value(_value) {}

  llvm::Value* Int::getVal() {
    Compiler::RinhaCompiler& generator = Compiler::RinhaCompiler::getSingleton();
    return generator.createInt(value);
  }

  Str::Str(std::string* _str) : str(_str) {}

  llvm::Value* Str::getVal() {
    Compiler::RinhaCompiler& generator = Compiler::RinhaCompiler::getSingleton();
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

  llvm::Value* Print::getVal() {
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton();
    return compiler.print(arg->getVal());
  }

  First::First(Term* _arg) :
    arg(_arg) {}

  llvm::Value* First::getVal() {
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton(); 
    return compiler.getTupleFirst(arg->getVal());
  }

  Second::Second(Term* _arg) :
    arg(_arg) {}

  llvm::Value* Second::getVal() {
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton();
    return compiler.getTupleSecond(arg->getVal());
  }

  Bool::Bool(bool _val) :
    val(_val) {};

  llvm::Value* Bool::getVal() {
    Compiler::RinhaCompiler& generator = Compiler::RinhaCompiler::getSingleton();
    return generator.createBool(val); 
  }

  Tuple::Tuple(Term* _first, Term* _second) :
    first(_first), second(_second) {}

  llvm::Value* Tuple::getVal() {
    Compiler::RinhaCompiler& generator = Compiler::RinhaCompiler::getSingleton();
    return generator.createTuple(first->getVal(), second->getVal());
  }

  Var::Var(std::string&& _name) :
    name(std::move(_name)) {};
}

namespace Compiler {

  SymbolTableStack::Identifier::Identifier(const std::string& _name):
    isFn(false),
    name(_name) {}
  
  SymbolTableStack::Identifier::Identifier(const std::string& _name, llvm::Type* _ret, std::initializer_list<llvm::Type*> _args) :
    isFn(true),
    name(_name),
    ret(_ret),
    args(_args) {}

  SymbolTableStack::Identifier::Identifier(const std::string& _name, llvm::Type* _ret, std::vector<llvm::Type*> _args) :
    isFn(true),
    name(_name),
    ret(_ret),
    args(_args) {}
     
  SymbolTableStack::SymbolTableStack() {
    pushScope();  // Global Data
  }

  bool SymbolTableStack::Identifier::operator==(const Identifier& other) const {
    if (name != other.name) return false;
    if (!isFn && !other.isFn) return true;
    else if (isFn == other.isFn) {
      if (ret != other.ret) return false;
      if (args.size() != other.args.size()) return false;
      for (uint32_t i = 0; i < args.size(); i++) 
        if (args[i] != other.args[i]) return false;
      return true;
    } else return false;
  }

  std::size_t SymbolTableStack::IdentifierHasher::operator()(const Identifier& id) const {
    std::size_t hash = std::hash<std::string>()(id.name);
    if (!id.isFn) return hash;
    hash ^= std::hash<llvm::Type*>()(id.ret);
    for (llvm::Type* arg : id.args) {
      hash ^= std::hash<llvm::Type*>()(arg);
    }
    return hash;
 }

  llvm::Value* SymbolTableStack::getValue(Identifier id) const {
    for (auto it = symbol_tables.crbegin(); it != symbol_tables.crend(); it++) {
      SymbolTable::const_iterator descriptor = it->find(id);
      if (descriptor != it->end()) return descriptor->second;
    }
    return nullptr;
  }

  llvm::Value* SymbolTableStack::getVariable(const std::string& name) const {
    Identifier id(name);
    return getValue(id);
  }
  
  llvm::Function* SymbolTableStack::getFunction(const std::string& name, llvm::Type* ret, const std::vector<llvm::Type*>& args) const {
    Identifier id(name, ret, args);
    return reinterpret_cast<llvm::Function*>(getValue(id));
  }

  void SymbolTableStack::insertValue(const Identifier& id, llvm::Value* value) {
    assert(value);
    SymbolTable& cur_scope = symbol_tables.back();
    cur_scope[id] = value;
  };
 
  void SymbolTableStack::insertVariable(const std::string& name, llvm::Value* value) {
    Identifier id(name);
    insertValue(id, value);
  }

  void SymbolTableStack::insertFunction(const std::string& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, llvm::Function* value) {
    Identifier id(name, ret, args);
    insertValue(id, value);
  };

   void SymbolTableStack::insertGlobalValue(const Identifier& id, llvm::Value* value) {
    assert(value);
    SymbolTable& cur_scope = symbol_tables.front();
    cur_scope[id] = value;
  };
 
  void SymbolTableStack::insertGlobalVariable(const std::string& name, llvm::Value* value) {
    Identifier id(name);
    insertGlobalValue(id, value);
  }

  void SymbolTableStack::insertGlobalFunction(const std::string& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, llvm::Function* value) {
    Identifier id(name, ret, args);
    insertGlobalValue(id, value);
  }; 

  void SymbolTableStack::pushScope() {
    symbol_tables.emplace_back();
  }

  void SymbolTableStack::popScope() {
    symbol_tables.pop_back();
  }
  
  std::unique_ptr<AST::File> ast_root;

  RinhaCompiler* RinhaCompiler::singleton = nullptr;

  RinhaCompiler::RinhaCompiler(const std::string& input_file) :
    builder(context),
    module(input_file, context),
    filename(input_file) {};

  bool RinhaCompiler::isInitialized() { return singleton != nullptr; }

  RinhaCompiler& RinhaCompiler::initialize(const std::string& input_file) {  
    if (isInitialized()) throw std::runtime_error("IRGenerator is already initialized.");
    singleton = new RinhaCompiler(input_file);
    RinhaCompiler& generator = *singleton;
    generator.externInsertPoint = generator.builder.saveIP();
    generator.createFunction(llvm::Type::getInt32Ty(generator.context), {}, "main");
    
    return *singleton;
  };

  RinhaCompiler& RinhaCompiler::getSingleton() {
    if (!isInitialized()) throw std::runtime_error("IRGenerator has not been initialized.");
    return *singleton;
  }

  void RinhaCompiler::printCode(const std::string& out_file) {
    std::error_code fd_ostream_ec;
    llvm::raw_fd_ostream ostream(out_file, fd_ostream_ec);
    module.print(ostream, nullptr);
  };

  llvm::Function* RinhaCompiler::createFunction(llvm::Type* ret, const std::vector<llvm::Type*>& args, const std::string& name) {
    llvm::FunctionType* fn_type = llvm::FunctionType::get(ret, args, false);
    llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, name, module);
    llvm::BasicBlock* fn_entry = llvm::BasicBlock::Create(context, "entry", fn);
    builder.SetInsertPoint(fn_entry);
    
    symtbl_stack.insertFunction(name, ret, args, fn);
    symtbl_stack.pushScope();
    return fn;
  }

  llvm::Function* RinhaCompiler::createFunction(llvm::Type* type, std::initializer_list<llvm::Type*> _args, const std::string& name) {
    const std::vector<llvm::Type*> args(_args);
    return createFunction(type, args, name);
  };  

  llvm::Function* RinhaCompiler::declareExternFunction(llvm::Type* ret, const std::vector<llvm::Type*>& args, const std::string& name) {
    llvm::IRBuilder<>::InsertPoint previous_point= builder.saveIP();
    builder.restoreIP(externInsertPoint);
    llvm::FunctionType* fn_type = llvm::FunctionType::get(ret, args, false);
    llvm::Function* extern_fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, name, module);
    symtbl_stack.insertGlobalFunction(name, ret, args, extern_fn);
    
    builder.restoreIP(previous_point);
    return extern_fn;
  };

  llvm::Value* RinhaCompiler::createBool(bool value) {
    return builder.getInt1(value); 
  }

  llvm::Value* RinhaCompiler::createInt(int32_t value) {
    return builder.getInt32(value);
  }

  llvm::Value* RinhaCompiler::createStr(const std::string& str) {
    return builder.CreateGlobalString(str, "", 0, &module);
  }

  llvm::Value* RinhaCompiler::createTuple(llvm::Value* first, llvm::Value* second) {
    llvm::Type* first_type = first->getType();
    llvm::Type* second_type = second->getType();

    llvm::StructType* tuple_type = llvm::StructType::get(context, {first_type, second_type});
    llvm::Value* tuple = builder.CreateAlloca(tuple_type);
    llvm::Value* zero = builder.getInt32(0);
    llvm::Value* one = builder.getInt32(1);

    llvm::Value* first_ptr = builder.CreateGEP(tuple_type, tuple, {zero, zero});
    llvm::Value* second_ptr = builder.CreateGEP(tuple_type, tuple, {zero, one});

    builder.CreateStore(first, first_ptr);
    builder.CreateStore(second, second_ptr);
    return tuple;
  }

  llvm::Value* RinhaCompiler::createUndefined() {
    llvm::Value* zero = builder.getInt8(0);
    return builder.CreateIntToPtr(zero, llvm::Type::getInt8PtrTy(context));
  }

  void RinhaCompiler::createVoidReturn() {
    builder.CreateRetVoid();    
  }

  void RinhaCompiler::createReturn(llvm::Value* val) {
    builder.CreateRet(val);
  }

  void RinhaCompiler::createReturn(uint32_t val) {
    builder.CreateRet(builder.getInt32(val));
  }

  llvm::Value* RinhaCompiler::getTupleFirst(llvm::Value* tuple) {
    if (!llvm::isa<llvm::AllocaInst>(tuple)) return tuple;
    llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(tuple);
    llvm::Type* alloc_type = alloca->getAllocatedType();
    if (!alloc_type->isStructTy()) return tuple;
    llvm::Value* first_element_ptr = builder.CreateStructGEP(alloc_type, alloca, 0);
    return builder.CreateLoad(alloc_type->getStructElementType(0), first_element_ptr);
  }

  llvm::Value* RinhaCompiler::getTupleSecond(llvm::Value* tuple) {
    if (!llvm::isa<llvm::AllocaInst>(tuple)) return createUndefined();
    llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(tuple);
    llvm::Type* alloc_type = alloca->getAllocatedType();
    if (!alloc_type->isStructTy()) return createUndefined();
    if (alloc_type->getStructNumElements() < 2) return createUndefined();
    llvm::Value* second_element_ptr = builder.CreateStructGEP(alloc_type, alloca, 1);
    return builder.CreateLoad(alloc_type->getStructElementType(1), second_element_ptr);
  }

  llvm::Value* RinhaCompiler::print(llvm::Value* val) {
    std::string print_name;

    llvm::Type* type = val->getType();
    if (type->isIntegerTy(1)) {
      print_name = "print_bool";
      type = llvm::Type::getInt8Ty(context);
      val = builder.CreateZExt(val, type);
    } else if (type->isIntegerTy()) {
      print_name = "print_num";
    } else if (type->isIntOrPtrTy()) {
      print_name = "print_str";
    } else if (type->isStructTy()) {
      print_name = "print_tuple";
      throw std::runtime_error("Tuple printing not supported yet. Must be implemented.");
    } else if (type->isFunctionTy()) {
      print_name = "print_closure";
      type = llvm::Type::getVoidTy(context);
    } else {
      print_name = "print_undefined";
      type = llvm::Type::getVoidTy(context);
      abort();
    }

    std::vector<llvm::Type*> args = {type};
    llvm::Type* void_type = llvm::Type::getVoidTy(context);
    llvm::Function* print_fn = symtbl_stack.getFunction(print_name, void_type, args); 
    if (!print_fn) {
      print_fn = declareExternFunction(void_type, args, print_name);
    }

    builder.CreateCall(print_fn, {val});
    return val;
  }

  static std::string __rinha_file = "rinha_default_filename";
  static AST::File* __ast_file = nullptr;

  static void set_rinha_file(const std::string& rinha_file) {
    __rinha_file = rinha_file;
    yyin = read_file(rinha_file);
  };

  const std::string& get_rinha_filename() {
    return __rinha_file;
  }

  void set_ast_file(AST::File* file) {
    __ast_file = file;
  }

  int compile(const std::string& input_file, const std::string& output_file) {
    set_rinha_file(input_file);
    RinhaCompiler& generator = RinhaCompiler::initialize(input_file);
    
    int ret = yyparse();
    if (ret != 0) {
      std::cerr << "Error while parsing. yyparse error: " << ret << std::endl;
      exit(EXIT_FAILURE);
    }

    assert(__ast_file);
    __ast_file->compile();
    generator.printCode(output_file);
    delete __ast_file;
    return EXIT_SUCCESS;
  }
}