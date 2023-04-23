#include "symbol.hh"

SymbolTable::SymbolTable(SymbolTable *outer) {
    this->outer = outer;
}

bool SymbolTable::contains(std::string key, int type) {
    if (table.find(std::make_pair(key, type)) != table.end()) {
        return true;
    } else if (outer != NULL) {
        return outer->contains(key, type);
    } else {
        return false;
    }
}

void SymbolTable::insert(std::string key, int type) {
    table.insert(std::make_pair(key, type));
}

SymbolStack::SymbolStack() {
    stack.push(SymbolTable(NULL));
}

void SymbolStack::enterScope() {
    stack.push(SymbolTable(&stack.top()));
}

void SymbolStack::exitScope() {
    stack.pop();
}

void SymbolStack::insert(std::string key, int type) {
    stack.top().insert(key, type);
}

bool SymbolStack::contains(std::string key, int type) {
    return stack.top().contains(key, type);
}

//checks recursively all the way to the top
bool SymbolStack::containsScopeConflict(std::string key, int type) {
    SymbolTable *current = &stack.top();
    while (current != NULL) {
        if (current->contains(key, type)) {
            return true;
        }
        current = current->outer;
    }
    return false;
}

