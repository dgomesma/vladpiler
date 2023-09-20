#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "common.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"

// Using aliases because there types might be subject to change
using Descriptor = llvm::Value;
using SymbolTable = std::map<std::string, Descriptor*>;

// Keeps track of scoped symbols
struct SymbolTableStack {
  std::vector<SymbolTable> symbol_tables;

  SymbolTableStack();
  Descriptor* getDescriptor(const std::string& symbol) const;
  Descriptor* getDescriptorInCurScope(const std::string& symbol) const;
  void insertDescriptor(const std::string& symbol, Descriptor *descriptor);
  void pushScope();
  void popScope();
};

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
  };

  struct Binary : Term {
    std::unique_ptr<Term> lhs;
    std::unique_ptr<Term> rhs;
    BinOp binop;

    Binary(Term* _lhs, Term* _rhs, BinOp _binop);
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
  };

  struct Let : Term {
    std::unique_ptr<Parameter> parameter;
    std::unique_ptr<Term> val;
    std::unique_ptr<Term> next;

    Let(Parameter* _parameter,
      Term* _val, 
      Term* _next);
  };

  struct If : Term {
    std::unique_ptr<Term> condition;
    std::unique_ptr<Term> then;
    std::unique_ptr<Term> orElse; // Reminder: else is a reserved keyword ;) 

    If(Term* _condition, 
      Term* _then, 
      Term* _orElse);
  };

  struct Print : Term {
    std::unique_ptr<Term> arg;

    Print(Term* _arg);
  };

  struct First : Term {
    std::unique_ptr<Term> arg;

    First(Term* _arg);
  };

  struct Second : Term {
    std::unique_ptr<Term> arg;

    Second(Term* _arg);
  };

  struct Bool : Term {
    bool val;

    Bool(bool _val);
  };

  struct Tuple : Term {
    std::unique_ptr<Term> first;
    std::unique_ptr<Term> second;

    Tuple(Term* first, Term* second);
  };

  struct Var : Term {
    std::string name;

    Var(std::string&& name);
  };
}

namespace Compiler {
   int compile(const std::string& input_file, const std::string& output_file);

  /*  This class is necessary to keep context info for the parser
      since there is no other trivial way to pass info to the parser.
  */
  struct Context {
    static Context* context;

    llvm::LLVMContext llvm_context;
    llvm::IRBuilder<> llvm_builder;
    std::unique_ptr<llvm::Module> llvm_module;
    std::unique_ptr<llvm::raw_fd_ostream> ostream;
    llvm::Function* main_fn;
    SymbolTableStack symtbl_stack;
    const std::string& filename;

    Context(const std::string& input_file, const std::string& output_file);
    void beginCodegen();
    void endCodegen();
    // Prints out the code
    void printOut();
    void createMainFn();
    void linkExternPrint();
  };

  extern Context* ctx;
}

#endif