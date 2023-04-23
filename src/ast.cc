#include "ast.hh"

#include <string>
#include <vector>

NodeBinOp::NodeBinOp(NodeBinOp::Op ope, Node *leftptr, Node *rightptr) {
    type = BIN_OP;
    op = ope;
    left = leftptr;
    right = rightptr;
}

bool checkIfNum(std::string str){
    int i = 0;
 
    // Handle negative numbers
    if (str[0] == '-') {
        i = 1;
    }
 
    // Check each character of the string
    while (str[i] != '\0') {
        if (!isdigit(str[i])) {
            return false;
        }
        i++;
    }
 
    return true;
}

std::string NodeBinOp::to_string() {
 
    std::string leftStr = left->to_string();
    std::string rightStr = right->to_string();
 
    int leftVal;
    int rightVal;
 
    if(checkIfNum(rightStr) && checkIfNum(leftStr)){
        leftVal = std::stoi(leftStr);
        rightVal = std::stoi(rightStr);
 
        if(op == PLUS){
            return std::to_string(leftVal + rightVal);
        }
        else if(op == MINUS){
            return std::to_string(leftVal - rightVal);
        }
        else if(op == MULT){
            return std::to_string(leftVal * rightVal);
        }
        else if(op == DIV){
            return std::to_string(leftVal / rightVal);
        }
    }
 
    std::string out = "(";
    switch(op) {
        case PLUS: out += '+'; break;
        case MINUS: out += '-'; break;
        case MULT: out += '*'; break;
        case DIV: out += '/'; break;
    }
 
    out += ' ' + left->to_string() + ' ' + right->to_string() + ')';
 
    return out;
}

NodeInt::NodeInt(int val) {
    type = INT_LIT;
    value = val;
}

std::string NodeInt::to_string() {
    return std::to_string(value);
}


NodeStmts::NodeStmts() {
    type = STMTS;
    list = std::vector<Node*>();
}

void NodeStmts::push_back(Node *node) {
    list.push_back(node);
}

std::string NodeStmts::to_string() {
    std::string out = "(begin";
    for(auto i : list) {
        out += " " + i->to_string();
    }

    out += ')';

    return out;
}

NodeDecl::NodeDecl(std::string id, Node *expr, int data) {
    type = ASSN;
    identifier = id;
    expression = expr;
    datatype = data;
}

std::string NodeDecl::to_string() {
    std::string strType;
 
    if(datatype == 0){
        strType = "short";
    }
    else if(datatype == 1){
        strType = "int";
    }
    else if(datatype == 2){
        strType = "long";
    }
 
    std::string out = "";
    identifier = "(" + identifier;
    strType = strType + ")";
    out += "(let " + identifier + " " + strType + " " + expression->to_string() + ")";
    
    return out;
}

NodeDebug::NodeDebug(Node *expr) {
    type = DBG;
    expression = expr;
}

std::string NodeDebug::to_string() {
    return "(dbg " + expression->to_string() + ")";
}

NodeIdent::NodeIdent(std::string ident) {
    identifier = ident;

}
std::string NodeIdent::to_string() {
    return identifier;
}

NodeIfElse::NodeIfElse(Node *cond, Node *then, Node *els) {
    type = IF;
    condition = cond;
    if_cond = then;
    else_cond = els;
}

std::string NodeIfElse::to_string() {
    return "(if-else " + condition->to_string() + " " + if_cond->to_string() + " " + else_cond->to_string() + ")";
}