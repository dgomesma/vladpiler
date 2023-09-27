#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "common.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace AST {

  enum class BinOp {
    PLUS,
    MINUS,
    MULT,
    DIV,
    MOD,
    EQ,
    NEQ,
    GT,
    LT,
    GTE,
    LTE,
    AND,
    OR
  };

  struct Localization {
    uint64_t beginLine;
  };

  /* Will consider smarter ways to include this data inside of
     Symbols without requiring passing an argument.
  struct SymbolInfo {
    std::string text;
    Localization loc;

    SymbolInfo(const Localization& loc, std::string&& text);
  };
  */

  struct Symbol {
  };  
  
  struct Term : virtual Symbol {
    virtual llvm::Value* getVal() {return nullptr;};
    Term() = default;
  };

  struct File : Symbol {
    std::string filename;
    std::unique_ptr<Term> term;

    File(
      const std::string& filename,
      Term* term
    );

    void compile();
  };

  struct Int : Term {
    int32_t value;    
    Int(int64_t _value);
    llvm::Value* getVal() override;
  };

  struct Str : Term {
    std::unique_ptr<std::string> str;
    Str(std::string* _str);
    llvm::Value* getVal() override;
  };

  struct Arguments: Symbol {
    std::vector<std::unique_ptr<Term>> args;

    Arguments();
  };

  struct Call : Term {
    const std::string callee;
    std::unique_ptr<Arguments> args;

    Call(std::string* _callee, Arguments* _args);    

    llvm::Value* getVal() override;
  };

  struct Binary : Term {
    std::unique_ptr<Term> lhs;
    std::unique_ptr<Term> rhs;
    BinOp binop;

    Binary(Term* _lhs, Term* _rhs, BinOp _binop);

    llvm::Value* getVal() override;
  };

  struct Parameter: Symbol {
    std::unique_ptr<std::string> identifier;

    Parameter(std::string* _identifier);
    Parameter(Parameter* parameter);
  };

  struct Parameters : Symbol {
    std::vector<std::unique_ptr<Parameter>> params;

    Parameters();
  };

  struct Function : Term {
    std::unique_ptr<Parameters> parameters;
    std::unique_ptr<Term> value;

    Function(Parameters* _parameters, Term* _value);

    llvm::Value* getVal() override;

  };

  struct Let : Term {
    std::unique_ptr<Parameter> parameter;
    std::unique_ptr<Term> val;
    std::unique_ptr<Term> next;

    Let(Parameter* _parameter,
      Term* _val, 
      Term* _next);

    llvm::Value* getVal() override;
  };

  struct If : Term {
    std::unique_ptr<Term> condition;
    std::unique_ptr<Term> then;
    std::unique_ptr<Term> orElse; // Reminder: else is a reserved keyword ;) 

    If(Term* _condition, 
      Term* _then, 
      Term* _orElse);

    llvm::Value* getVal() override;
  };


  struct Print : Term {
    std::unique_ptr<Term> arg;

    Print(Term* _arg);

    llvm::Value* getVal() override;
  };

  struct First : Term {
    std::unique_ptr<Term> arg;

    First(Term* _arg);

    llvm::Value* getVal() override;
  };

  struct Second : Term {
    std::unique_ptr<Term> arg;

    Second(Term* _arg);

    llvm::Value* getVal() override;
  };

  struct Bool : Term {
    bool val;

    Bool(bool _val);

    llvm::Value* getVal() override;
  };

  struct Tuple : Term {
    std::unique_ptr<Term> first;
    std::unique_ptr<Term> second;

    Tuple(Term* first, Term* second);

    llvm::Value* getVal() override;
  };

  struct Var : Term {
    std::unique_ptr<std::string> name;

    Var(std::string* name);

    llvm::Value* getVal() override;
  };
}

namespace Compiler {

    struct ClosureSignature {
      std::vector<std::string> params;
      AST::Term* fn_body;
    };

    
    struct EitherValOrClosure {
      llvm::Value* val;
      ClosureSignature* closureSig;
    };

  // Keeps track of scoped symbols
  class SymbolTableStack {

    using SymbolTable = std::unordered_map<std::string, EitherValOrClosure>;

    std::vector<SymbolTable> symbol_tables;
  public:
    SymbolTableStack();
    // Get functions may return nullptr if a corresponding value is not found
    EitherValOrClosure* getValue(const std::string& id);
    void insertValue(const std::string& name, llvm::Value* value);
    void insertClosure(const std::string& name, ClosureSignature* closure_sig);
    void pushScope();
    void popScope();
  };

  int compile(const std::string& input_file, const std::string& output_file);
  const std::string& get_rinha_filename();
  void set_ast_file(AST::File* file);

  class RinhaCompiler {
  private:

    // Fortunately tuples are immutable or else we would have problems
    struct TuplePtrIds{
      std::string* first_ptr_id;    // Assign nullptr if first/second  is not a ptr
      std::string* second_ptr_id;
    };

    struct ClosureInstanceNode {
      llvm::Function* fn;
      std::map<llvm::Type*,std::shared_ptr<ClosureInstanceNode>> children;
    };
 
    static RinhaCompiler* singleton;

    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Module module;

    const std::string& filename;
    std::unique_ptr<AST::File> ast_root;
    llvm::Type* const default_type;

    llvm::IRBuilder<>::InsertPoint externInsertPoint;
    SymbolTableStack symtbl_stack;
    std::map<std::string, llvm::Function*> extern_fn_table;

    /*  So, I did a little bit of a hack here. The idea is to implement a way
        to associate a "unique identifier" with every pointer value since only
        instructions have names, and the same pointer might correspond to
        multiple instructions with different names.

        So, for every instruction/value that corresponds to the same pointer,
        we do a ptr_id_table[val] = id. We need to make sure this always gets
        done every time a pointer is moved from one place to another so that
        the pointer ID can always be retrieved from the value (necessary for
        printing).

        Then, with this ID, you can access the pointee type, and if we're talking
        about a struct, we can access the pointer IDs of the struct members (if
        if has pointers).
    
        This could be better designed, but right now I'm in a rush.

        //TODO: Improve this pointer-type record-keeping.
    */
    std::map<llvm::Value*, std::string> ptr_id_table;
    std::map<std::string, llvm::Type*> ptr_type_table;
    std::map<std::string, TuplePtrIds> tuple_ptr_types;

    enum class SpecialValue {
      UNDEFINED = 1,
      CLOSURE
    };

    std::map<llvm::Value*, SpecialValue> special_value_table;

    std::map<llvm::Value*, ClosureSignature> closure_table;
    std::map<std::string, std::map<llvm::Type*, std::shared_ptr<ClosureInstanceNode>>> closure_cache;
   
    void printType(llvm::Type* val);
    void printType(llvm::Value* val);

    RinhaCompiler(const std::string& input_file);
    // llvm::Function* lookForCosureInstance(const ClosureSignature& closure_sig, const std::vector<llvm::Value*>& args);
    llvm::Function* _getCachedClosure(const std::vector<llvm::Type*>& args, uint64_t args_it, std::shared_ptr<ClosureInstanceNode> instance_it);
    llvm::Function* getCachedClosure(const std::string& name, const std::vector<llvm::Type*> args);
    llvm::FunctionType* getDefaultFnType(uint32_t n_args);
    llvm::Function* createMain();
    llvm::Value* createTupleDescriptor(llvm::Value* tuple);
    llvm::Value* createUndefined();
    
    inline bool is32Int(llvm::Value*);
    inline bool isInt(llvm::Value*);    
    inline bool isBool(llvm::Value*);

    void printValName(llvm::Value* val);
    void printTuple(llvm::Value* tuple);
    void _print(llvm::Value*);
    llvm::Type* getPtrType(llvm::Value*);
  public:
    enum class insert_point_loc_t {
      EXTERN,
      MAIN
    };

    static RinhaCompiler& initialize(const std::string& input_file);
    static bool isInitialized();
    static RinhaCompiler& getSingleton();

    // Prints code to the given output file
    void printCode(const std::string& out_file);


    // Declare an extern function at the beginning of the module
    llvm::Function* getExternFunction(llvm::Type* ret, const std::vector<llvm::Type*>& args, const std::string& name);
    llvm::Value* getVariable(const std::string& name);

    llvm::Value* createBool(bool value);
    llvm::Value* createInt(int32_t value);
    llvm::Value* createStr(const std::string& str);
    llvm::Value* createTuple(llvm::Value* value1, llvm::Value* value2);

    llvm::Value* createAdd(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createMinus(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createMult(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createDiv(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createMod(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createEq(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createNeq(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createGt(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createLt(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createGte(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createLte(llvm::Value* value1, llvm::Value* value2);
    llvm::Value* createAnd(AST::Term* value1, AST::Term* value2);
    llvm::Value* createOr(AST::Term* value1, AST::Term* value2);
  
    bool isClosure(llvm::Value* val);
    llvm::Value* assignClosure(const std::string& name, llvm::Value* val);
    llvm::Value* createAnonClosure(const std::vector<std::string>& params, AST::Term* fn_body);
    llvm::Value* createClosureVal();
    llvm::Value* callClosure(const std::string& name, std::vector<llvm::Value*>& args);

    void createVoidReturn();
    void createReturn(llvm::Value* val);
    void createReturn(uint32_t val);
    llvm::Value* createIfElse(AST::Term* cond, AST::Term* then, AST::Term* orElse);
    void createVariable(const std::string& identifier, llvm::Value* val);

    llvm::Value* getTupleFirst(llvm::Value* tuple);
    llvm::Value* getTupleSecond(llvm::Value* tuple);
    llvm::Value* print(llvm::Value*);
  };

}

#endif