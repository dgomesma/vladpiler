#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "parser.tab.h"
#include "rinha_extern.h"
#include <ostream>

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

  llvm::Value* Binary::getVal() {
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton(); 
    switch (binop) {
      case BinOp::PLUS:  return compiler.createAdd(lhs->getVal(), rhs->getVal());
      case BinOp::MINUS: return compiler.createMinus(lhs->getVal(), rhs->getVal());
      case BinOp::MULT:  return compiler.createMult(lhs->getVal(), rhs->getVal());
      case BinOp::DIV:   return compiler.createDiv(lhs->getVal(), rhs->getVal());
      case BinOp::MOD:   return compiler.createMod(lhs->getVal(), rhs->getVal());
      case BinOp::EQ:    return compiler.createEq(lhs->getVal(), rhs->getVal());
      case BinOp::NEQ:   return compiler.createNeq(lhs->getVal(), rhs->getVal());
      case BinOp::GT:    return compiler.createGt(lhs->getVal(), rhs->getVal());
      case BinOp::LT:    return compiler.createLt(lhs->getVal(), rhs->getVal());
      case BinOp::GTE:   return compiler.createGte(lhs->getVal(), rhs->getVal());
      case BinOp::LTE:   return compiler.createLte(lhs->getVal(), rhs->getVal());
      case BinOp::AND:   return compiler.createAnd(lhs.get(), rhs.get());
      case BinOp::OR:    return compiler.createOr(lhs.get(), rhs.get());
      default:           return nullptr;
    }
  }

  Parameter::Parameter(std::string* id) :
    identifier(id) {}

  Parameter::Parameter(Parameter* parameter) :
    identifier(std::move(parameter->identifier)) {}

  Parameters::Parameters() = default;

  Function::Function(Parameters* _parameters, Term* _value) :
    parameters(_parameters), value(std::move(_value)) {}

  Let::Let(Parameter* _parameter, Term* _val, Term* _next) :
    parameter(_parameter), val(_val), next(_next) {}

  llvm::Value* Let::getVal() {
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton();
    compiler.createVariable(*parameter->identifier, val->getVal());
    return next->getVal();
  }

  If::If(Term* _condition, Term* _then, Term* _orElse) :
    condition(_condition), then(_then), orElse(_orElse) {}

  llvm::Value* If::getVal() {
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton();
    return compiler.createIfElse(condition.get(), then.get(), orElse.get());
  }

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
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton();
    return compiler.createTuple(first->getVal(), second->getVal());
  }

  Var::Var(std::string* _name) :
    name(_name) {};

  llvm::Value* Var::getVal() {
    Compiler::RinhaCompiler& compiler = Compiler::RinhaCompiler::getSingleton();
    return compiler.getVariable(*name);
  }

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
    filename(input_file),
    default_type(builder.getInt32Ty()) {};

  bool RinhaCompiler::isInitialized() { return singleton != nullptr; }

  llvm::Function* RinhaCompiler::createMain() {
    llvm::FunctionType* main_fn_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {}, false);
    llvm::Function* main = llvm::Function::Create(main_fn_type, llvm::Function::ExternalLinkage, "main", module);
    llvm::BasicBlock* main_entry = llvm::BasicBlock::Create(context, "entry", main);
    builder.SetInsertPoint(main_entry);

    return main;
  }

  RinhaCompiler& RinhaCompiler::initialize(const std::string& input_file) {  
    if (isInitialized()) throw std::runtime_error("IRGenerator is already initialized.");
    singleton = new RinhaCompiler(input_file);
    RinhaCompiler& generator = *singleton;
    generator.externInsertPoint = generator.builder.saveIP();
    generator.createMain();
    
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

  llvm::FunctionType* RinhaCompiler::getDefaultFnType(uint32_t n_args) {
    std::vector<llvm::Type*> args;
    for (uint32_t i = 0; i < n_args; i++) {
      args.push_back(default_type); 
    }

    return llvm::FunctionType::get(default_type, args, false);
  }

  llvm::Function* RinhaCompiler::createClosure(const std::string& name, uint32_t n_args) {
    llvm::FunctionType* default_fn_type = getDefaultFnType(n_args);
    llvm::Function* closure = llvm::Function::Create(default_fn_type, llvm::Function::ExternalLinkage, name, module);
    symtbl_stack.insertFunction(name, default_type, default_fn_type->params(), closure);
    return closure;
  }

  llvm::Function* RinhaCompiler::createFunction(llvm::Type* ret, const std::vector<llvm::Type*>& args, const std::string& name) {
    llvm::FunctionType* fn_type = llvm::FunctionType::get(ret, args, false);
    llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, name, module);
    
    symtbl_stack.pushScope();
        
    return fn;
  }

  llvm::Function* RinhaCompiler::createFunction(llvm::Type* type, std::initializer_list<llvm::Type*> _args, const std::string& name) {
    const std::vector<llvm::Type*> args(_args);
    return createFunction(type, args, name);
  };  

  llvm::Value* RinhaCompiler::getVariable(const std::string& name) {
    llvm::Value* var = symtbl_stack.getVariable(name);
    return var ?  var : createUndefined();
  }

  llvm::Function* RinhaCompiler::getExternFunction(llvm::Type* ret, const std::vector<llvm::Type*>& args, const std::string& name) {
    llvm::Function* print_fn = symtbl_stack.getFunction(name, ret, args);    
    if (print_fn) return print_fn;

    llvm::IRBuilder<>::InsertPoint previous_point= builder.saveIP();
    builder.restoreIP(externInsertPoint);
    llvm::FunctionType* fn_type = llvm::FunctionType::get(ret, args, false);
    llvm::Function* extern_fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, name, module);
    symtbl_stack.insertGlobalFunction(name, ret, args, extern_fn);
    
    builder.restoreIP(previous_point);
    return extern_fn;
  };

  void RinhaCompiler::printType(llvm::Type* type) {
    type->print(llvm::errs());
    std::cerr << std::endl;
  }

  void RinhaCompiler::printType(llvm::Value* val) {
    std::cerr << "Value " << val->getName().str() << ": ";
    printType(val->getType());
  }

  llvm::Value* RinhaCompiler::createBool(bool value) {
    return builder.getInt1(value); 
  }

  llvm::Value* RinhaCompiler::createInt(int32_t value) {
    return builder.getInt32(value);
  }

  llvm::Value* RinhaCompiler::createStr(const std::string& str) {
    llvm::GlobalVariable* ret = builder.CreateGlobalString(str, "str", 0, &module);
    std::string id = ret->getName().str();
    ptr_id_table[ret] = id;
    ptr_type_table[id] = ret->getValueType();
    return ret;
  }

  llvm::Value* RinhaCompiler::createTuple(llvm::Value* first, llvm::Value* second) {
    llvm::Type* first_type = first->getType();
    llvm::Type* second_type = second->getType();

    llvm::StructType* tuple_type = llvm::StructType::get(context, {first_type, second_type});
    llvm::Value* tuple = builder.CreateAlloca(tuple_type, nullptr, "tuple");

    std::string tuple_id = tuple->getName().str();
    ptr_id_table[tuple] = tuple_id;

    std::string* first_ptr_id = first_type->isPointerTy() ? 
      &ptr_id_table[first] : nullptr; 
    std::string* second_ptr_id = second_type->isPointerTy() ?
      &ptr_id_table[second] : nullptr;
    tuple_ptr_types[tuple_id] = {first_ptr_id, second_ptr_id};
    ptr_type_table[tuple_id] = tuple_type;

    llvm::Value* zero = builder.getInt32(0);
    llvm::Value* one = builder.getInt32(1);

    llvm::Value* first_ptr = builder.CreateGEP(tuple_type, tuple, {zero, zero});
    llvm::Value* second_ptr = builder.CreateGEP(tuple_type, tuple, {zero, one});

    builder.CreateStore(first, first_ptr);
    builder.CreateStore(second, second_ptr);
    return tuple;
  }

  
  llvm::Value* RinhaCompiler::createAdd(llvm::Value* lhs, llvm::Value* rhs) {

    if (is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateAdd(lhs, rhs, "add");
    }

    // What about string operations?

    return createUndefined();
  }

  llvm::Value* RinhaCompiler::createMinus(llvm::Value* lhs, llvm::Value* rhs){

    if (is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateSub(lhs, rhs, "sub");
    }

    return createUndefined();
  }; 
  llvm::Value* RinhaCompiler::createMult(llvm::Value* lhs, llvm::Value* rhs){

    if (is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateMul(lhs, rhs, "mul");
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createDiv(llvm::Value* lhs, llvm::Value* rhs){
    
    if (is32Int(lhs) && is32Int(rhs)) {
      // Deal with div by zero
      return builder.CreateSDiv(lhs, rhs, "div");
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createMod(llvm::Value* lhs, llvm::Value* rhs){

    if(is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateSRem(lhs, rhs);
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createEq(llvm::Value* lhs, llvm::Value* rhs){

    if (isInt(lhs) && isInt(rhs)) {
      return builder.CreateICmpEQ(lhs, rhs, "eq");
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createNeq(llvm::Value* lhs, llvm::Value* rhs){

    if (isInt(lhs) && isInt(rhs)) {
      return builder.CreateICmpNE(lhs, rhs);
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createGt(llvm::Value* lhs, llvm::Value* rhs){

    if (is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateICmpSGT(lhs, rhs, "gt");
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createLt(llvm::Value* lhs, llvm::Value* rhs){

    if (is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateICmpSLT(lhs, rhs, "lt");
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createGte(llvm::Value* lhs, llvm::Value* rhs){

    if (is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateICmpSGE(lhs, rhs, "gte"); 
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createLte(llvm::Value* lhs, llvm::Value* rhs){

    if (is32Int(lhs) && is32Int(rhs)) {
      return builder.CreateICmpSLE(lhs, rhs, "lte");
    }

    return createUndefined();
  };
  llvm::Value* RinhaCompiler::createOr(AST::Term* lhs, AST::Term* rhs){
    llvm::Function* current_fn = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* or_block = llvm::BasicBlock::Create(context, "or", current_fn);
    llvm::BasicBlock* or_false = llvm::BasicBlock::Create(context, "and_false", current_fn);
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "or_merge", current_fn);

    builder.CreateBr(or_block);
    builder.SetInsertPoint(or_block);

    llvm::Value* lhs_val = lhs->getVal();
    if (!isBool(lhs_val)) return createUndefined();

    llvm::BasicBlock* current = builder.GetInsertBlock();
    builder.CreateCondBr(lhs_val, merge_block, or_false);

    builder.SetInsertPoint(or_false);
    llvm::Value* rhs_val = rhs->getVal();
    if (!isBool(rhs_val)) return createUndefined();

    llvm::BasicBlock* current_or_false = builder.GetInsertBlock();
    builder.CreateBr(merge_block);

    builder.SetInsertPoint(merge_block);
    llvm::PHINode *and_phi = builder.CreatePHI(lhs_val->getType(), 2, "or_phi");
    and_phi->addIncoming(lhs_val, current);
    and_phi->addIncoming(rhs_val, current_or_false);

    return and_phi;
  };

  llvm::Value* RinhaCompiler::createAnd(AST::Term* lhs, AST::Term* rhs){
    llvm::Function* current_fn = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* and_block = llvm::BasicBlock::Create(context, "and", current_fn);
    llvm::BasicBlock* true_block = llvm::BasicBlock::Create(context, "and_true", current_fn);
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "and_merge", current_fn);

    builder.CreateBr(and_block);
    builder.SetInsertPoint(and_block);

    llvm::Value* lhs_val = lhs->getVal();
    if (!isBool(lhs_val)) return createUndefined();

    llvm::BasicBlock* current = builder.GetInsertBlock();
    builder.CreateCondBr(lhs_val, true_block, merge_block);

    builder.SetInsertPoint(true_block);
    llvm::Value* rhs_val = rhs->getVal();
    if (!isBool(rhs_val)) return createUndefined();

    llvm::BasicBlock* current_and_true = builder.GetInsertBlock();
    builder.CreateBr(merge_block);

    builder.SetInsertPoint(merge_block);
    llvm::PHINode *and_phi = builder.CreatePHI(lhs_val->getType(), 2, "and_phi");
    and_phi->addIncoming(lhs_val, current);
    and_phi->addIncoming(rhs_val, current_and_true);

    return and_phi;
  };

  llvm::Value* RinhaCompiler::createUndefined() {
    llvm::Value* zero = builder.getInt8(0);
    return builder.CreateIntToPtr(zero, llvm::Type::getInt8PtrTy(context));
  }

  void RinhaCompiler::createVoidReturn() {
    builder.CreateRetVoid();    
    symtbl_stack.popScope();
  }

  void RinhaCompiler::createReturn(llvm::Value* val) {
    builder.CreateRet(val);
    symtbl_stack.popScope();
  }

  void RinhaCompiler::createReturn(uint32_t val) {
    builder.CreateRet(builder.getInt32(val));
    symtbl_stack.popScope();
  }

  llvm::Value* RinhaCompiler::createIfElse(AST::Term* cond, AST::Term* then, AST::Term* orElse) {
  	llvm::Function* current_fn = builder.GetInsertBlock()->getParent();
    
    llvm::BasicBlock* if_block = llvm::BasicBlock::Create(context, "if", current_fn);
  	llvm::BasicBlock* then_block = llvm::BasicBlock::Create(context, "then", current_fn);
  	llvm::BasicBlock* else_block = llvm::BasicBlock::Create(context, "or_else", current_fn);
  	llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "merge", current_fn);

    builder.CreateBr(if_block);
    builder.SetInsertPoint(if_block);
    llvm::Value* decision = cond->getVal();
    assert(decision);

    if (!isBool(decision)) return createUndefined();

    builder.CreateCondBr(decision, then_block, else_block);

    builder.SetInsertPoint(then_block);
    llvm::Value* then_val = then->getVal();
    builder.CreateBr(merge_block);
    
    builder.SetInsertPoint(else_block);
    llvm::Value* else_val = orElse->getVal();
    builder.CreateBr(merge_block);


    builder.SetInsertPoint(merge_block);
    llvm::PHINode* phi = builder.CreatePHI(then_val->getType(), 2, "if_phi");
    phi->addIncoming(then_val, then_block);
    phi->addIncoming(else_val, else_block);

    return phi;
   }

  void RinhaCompiler::createVariable(const std::string& identifier, llvm::Value* val) {
    symtbl_stack.insertVariable(identifier, val);
  }

  llvm::Type* RinhaCompiler::getPtrType(llvm::Value* val) {
    llvm::Type* type = val->getType();
    if (!type->isPointerTy()) return nullptr;
    if (llvm::isa<llvm::AllocaInst>(val)) {
      llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(val);
      return alloca->getAllocatedType();
    }
    else if (llvm::isa<llvm::GlobalVariable>(val)) {
      llvm::GlobalVariable* global = llvm::dyn_cast<llvm::GlobalVariable>(val);
      return global->getValueType();
    } else if (llvm::isa<llvm::LoadInst>(val)) {
      llvm::LoadInst* load = llvm::dyn_cast<llvm::LoadInst>(val);
      return load->getType();
    } else if (llvm::isa<llvm::GetElementPtrInst>(val)) {
      llvm::GetElementPtrInst* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(val);
      return gep->getSourceElementType();
    } else {
      std::cerr << "Pointer type not recognized" << std::endl;
      val->getType()->print(llvm::errs());
      std::cerr << std::endl;
      exit(1);
    }
  }

  llvm::Value* RinhaCompiler::getTupleFirst(llvm::Value* tuple_ptr) {
    if (!tuple_ptr->getType()->isPointerTy()) {
      std::cerr << "Warning: Running first on non-pointer value. " << std::endl; 
      return tuple_ptr;
    }

    llvm::Type* type = ptr_type_table[ptr_id_table[tuple_ptr]];
    if (!type) {
      std::cerr << "Error: Could not find an entry on tuple-type map for given pointer." << std::endl;   
      abort();
    }
    
    llvm::StructType* tuple_type; 
    if (type->isStructTy() && type->getStructNumElements() == 2) 
      tuple_type = llvm::dyn_cast<llvm::StructType>(type);
    else {
      std::cerr << "Warning: Running first on non-tuple pointer." << std::endl;
      return tuple_ptr;
    }

    std::string* first_id = nullptr;
    if (tuple_type->getElementType(0)->isPointerTy()) {
      first_id = tuple_ptr_types[ptr_id_table[tuple_ptr]].first_ptr_id; 
      assert(first_id);
    }
    
    llvm::Value* first_element_ptr = builder.CreateStructGEP(tuple_type, tuple_ptr, 0);
    llvm::LoadInst* load = builder.CreateLoad(tuple_type->getStructElementType(0), first_element_ptr, "first");
    if (first_id) ptr_id_table[load] = *first_id;

    return load;
  }

  llvm::Value* RinhaCompiler::getTupleSecond(llvm::Value* tuple_ptr) {
    if (!tuple_ptr->getType()->isPointerTy()) {
      std::cerr << "Warning: Running second on non-pointer value. " << std::endl; 
      return createUndefined();
    }

    llvm::Type* type = ptr_type_table[ptr_id_table[tuple_ptr]];
    if (!type) {
      std::cerr << "Error: Could not find an entry on tuple-type map for given pointer." << std::endl;   
      abort();
    }
    
    llvm::StructType* tuple_type; 
    if (type->isStructTy() && type->getStructNumElements() == 2) 
      tuple_type = llvm::dyn_cast<llvm::StructType>(type);
    else {
      std::cerr << "Warning: Running second on non-tuple pointer." << std::endl;
      return createUndefined();
    }

    std::string* second_id = nullptr;
    if (tuple_type->getElementType(1)->isPointerTy()) {
      second_id = tuple_ptr_types[ptr_id_table[tuple_ptr]].second_ptr_id;
      assert(second_id);
    }
    
    llvm::Value* second_element_ptr = builder.CreateStructGEP(tuple_type, tuple_ptr, 1);
    llvm::Value* load = builder.CreateLoad(tuple_type->getStructElementType(1), second_element_ptr, "load");
    if (second_id) ptr_id_table[load] = *second_id;

    return load;
  }

  llvm::Value* RinhaCompiler::print(llvm::Value* val) {
    _print(val);    
    llvm::Type* void_type = llvm::Type::getVoidTy(context);
    llvm::Function* print_nl = getExternFunction(void_type, {}, "print_nl");
    builder.CreateCall(print_nl);
    return val;
  }

  void RinhaCompiler::_print(llvm::Value* val) {
    std::string print_name = "print_undefined";
    std::vector<llvm::Type*> args = {};
    llvm::Type* type = val->getType();

    if (type->isIntegerTy(1)) {
      print_name = "print_bool";
      args = {llvm::Type::getInt8Ty(context)};
      val = builder.CreateZExt(val, type);

    } else if (type->isIntegerTy()) {
      print_name = "print_num";
      args = {val->getType()};

    } else if (type->isPointerTy()) {
      llvm::Type* ptr_type = ptr_type_table[ptr_id_table[val]];
      assert(ptr_type);

      
      if (ptr_type->isArrayTy() && ptr_type->getArrayElementType()->isIntegerTy(8)) {
        print_name = "print_str";
        args = {llvm::Type::getInt8PtrTy(context)};
      }

      else if (ptr_type->isStructTy()) {
        printTuple(val);  
        return;
      }
      /*
      ptr_type_table[val->getName().str()] << std::endl;
      llvm::Type* ptr_type = getPtrType(val);
      if (ptr_type->isStructTy()) {
        printTuple(val);
        return;
      } else if (ptr_type->isArrayTy() && ptr_type->getArrayElementType()->isIntegerTy(8)) {
        print_name = "print_str";
        args = {llvm::Type::getInt8PtrTy(context)};
      } else if (ptr_type->isPointerTy()) {

       
      }
    */
    } else if (type->isFunctionTy()) {
      print_name = "print_closure";
    } else {
      std::cerr << "Uncaught type" << std::endl;
    }

    llvm::Type* void_type = llvm::Type::getVoidTy(context);
    llvm::Function* print_fn = getExternFunction(void_type, args, print_name);

    builder.CreateCall(print_fn, {val});
  }

  bool RinhaCompiler::is32Int(llvm::Value* val) {
    return val->getType()->isIntegerTy(32);
  }

  bool RinhaCompiler::isInt(llvm::Value* val) {
    return val->getType()->isIntegerTy();
  }

  
  bool RinhaCompiler::isBool(llvm::Value* val) {
    return val->getType()->isIntegerTy(1);
  }

  void RinhaCompiler::printValName(llvm::Value* val) {
    std::cout << "Value name: " << val->getName().str()<< std::endl;
  } 
  
  // Assume tuple is well formatted
  void RinhaCompiler::printTuple(llvm::Value* tuple_ptr) {
    llvm::Type* void_type = llvm::Type::getVoidTy(context);
    llvm::Function* print_lp = getExternFunction(void_type, {}, "print_lp");
    llvm::Function* print_delim = getExternFunction(void_type, {}, "print_delim");
    llvm::Function* print_rp = getExternFunction(void_type, {}, "print_rp");
    
    builder.CreateCall(print_lp);
    llvm::Value* first = getTupleFirst(tuple_ptr);
    _print(first);
    builder.CreateCall(print_delim);
    llvm::Value* second = getTupleSecond(tuple_ptr);
    _print(second);
    builder.CreateCall(print_rp);
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