#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "common.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"

using Descriptor = llvm::Value;
using SymbolTable = std::map<std::string, Descriptor*>;

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
    uint64_t endLine;
    uint64_t beginCol;
    uint64_t endCol;
  };

  struct SymbolInfo {
    std::string text;
    Localization loc;

    SymbolInfo(const Localization& loc, std::string&& text);
  };

  struct Symbol {
    SymbolInfo info;

    Symbol(const SymbolInfo& info);
  };  

  
  struct Term : Symbol {
    virtual llvm::Value* getVal() = 0;      
  };

  struct File : Symbol {
    File(const SymbolInfo& info, const Term& term);
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
    Descriptor* descriptor;

    Call(Descriptor* _descriptor);    
  };

  struct Binary : Term {
    Term lhs;
    Term rhs;
  };

  struct Function : Term {
    
  };

  struct Let : Term {
    
  };

  struct If : Term {
    
  };

  struct Print : Term {
    
  };

  struct First : Term {
    
  };

  struct Second : Term {
    
  };

  struct Bool : Term {
    
  };

  struct Tuple : Term {
    
  };

  struct Var : Term {
    
  };
}

#endif