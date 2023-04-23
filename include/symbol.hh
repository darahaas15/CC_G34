#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <set>
#include <string>
#include "ast.hh"
#include <stack>


// Basic symbol table, just keeping track of prior existence and nothing else
struct SymbolTable {
    std::set<std::pair<std::string, int>> table;
    SymbolTable *outer;

    SymbolTable(SymbolTable *outer);

    bool contains(std::string key, int type);
    void insert(std::string key, int type);
};

struct SymbolStack {
    std::stack<SymbolTable> stack;

    SymbolStack *outer;
    SymbolStack();

    void enterScope();
    void exitScope();

    void insert(std::string key, int type);
    bool contains(std::string key, int type);  // checks only in the current scope

    bool containsScopeConflict(std::string key, int type);  //checks recursively all the way to the top
    
};


#endif