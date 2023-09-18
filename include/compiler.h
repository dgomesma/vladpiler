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
    virtual llvm::Value* getVal() = 0;      
  };

  struct File : Symbol {
    std::string filename;
    std::unique_ptr<Term> term;

    File(
      const std::string& filename,
      std::unique_ptr<Term> term
    );
  };

  struct Int : Term {
    int64_t value;    
    Int(int64_t _value);
  };

  struct Str : Term {
    std::string str;
    Str(std::string&& _str);
  };

  struct Call : Term {
    const std::string callee;
    std::vector<Term> args;

    Call(std::string&& _callee, std::vector<Term>&& _args);    
  };

  struct Binary : Term {
    std::unique_ptr<Term> lhs;
    std::unique_ptr<Term> rhs;
    BinOp binop;

    Binary(std::unique_ptr<Term> _lhs, std::unique_ptr<Term> _rhs, BinOp _binop);
  };

  struct Parameter: Symbol {
    std::string identifier;
    Descriptor* descriptor;

    Parameter(std::string&& _identifier, Descriptor* _descriptor=nullptr);
  };

  struct Parameters : Symbol {
    std::vector<Parameter> params;

    Parameters(std::vector<Parameter>&& _params);
    Parameters(Parameters&& _params);
  };

  struct Function : Term {
    Parameters parameters;
    std::unique_ptr<Term> value;

    Function(Parameters&& _parameters, std::unique_ptr<Term> _value);
  };

  struct Let : Term {
    Parameter parameter;
    std::unique_ptr<Term> val;
    std::unique_ptr<Term> next;

    Let(Parameter&& _parameter,
      std::unique_ptr<Term> _val, 
      std::unique_ptr<Term> _next);
  };

  struct If : Term {
    std::unique_ptr<Term> condition;
    std::unique_ptr<Term> then;
    std::unique_ptr<Term> orElse; // Reminder: else is a reserved keyword ;) 

    If(std::unique_ptr<Term> _condition, 
      std::unique_ptr<Term> _then, 
      std::unique_ptr<Term> _orElse);
  };

  struct Print : Term {
    std::unique_ptr<Term> arg;

    Print(std::unique_ptr<Term> _arg);
  };

  struct First : Term {
    std::unique_ptr<Term> arg;

    First(std::unique_ptr<Term> _arg);
  };

  struct Second : Term {
    std::unique_ptr<Term> arg;

    Second(std::unique_ptr<Term> _arg);
  };

  struct Bool : Term {
    bool val;

    Bool(bool _val);
  };

  struct Tuple : Term {
    std::unique_ptr<Term> first;
    std::unique_ptr<Term> second;

    Tuple(std::unique_ptr<Term> first, std::unique_ptr<Term> second);
  };

  struct Var : Term {
    std::string name;

    Var(std::string&& name);
  };
}

namespace Compiler {
   int compile(const std::string& filename);

  /*  This class is necessary to keep context info for the parser
      since there is no other trivial way to pass info to the parser.
  */
  struct Context {
    static Context* context;

    llvm::LLVMContext llvm_context;
    std::unique_ptr<llvm::Module> llvm_module;
    std::unique_ptr<llvm::raw_fd_ostream> ostream;
    SymbolTableStack symtbl_stack;
    const std::string& filename;

    Context(const std::string& _filename);
    // Prints out the code
    void printOut();
  };

  extern Context* context;
}

#endif