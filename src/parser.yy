%define api.value.type { ParserValue }

%code requires {
#include <iostream>
#include <vector>
#include <string>

#include "parser_util.hh"
#include "symbol.hh"

}

%code {

#include <cstdlib>

extern int yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolStack symbol_table;

int yyerror(std::string msg);
int yyerrortype(std::string msg, int type);

int maxType = -1;
bool exists = false;
}


%token TPLUS TDASH TSTAR TSLASH
%token <lexeme> TINT_LIT TIDENT
%token INT TLET TDBG
%token TSCOL TLPAREN TRPAREN TEQUAL TLBRACE TRBRACE
%token TCOLON TSHORT TINT TLONG
%token TIF TELSE

%type <node> Expr Stmt
%type <stmts> Program StmtList

%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program :                
        { final_values = nullptr; }
        | StmtList 
        { final_values = $1; }
	    ;

StmtList : Stmt                
         { $$ = new NodeStmts(); $$->push_back($1); }
	     | StmtList Stmt 
         { $$->push_back($2); }
	     ;

Stmt : TLET TIDENT TCOLON 
       {
        maxType = 0;

       } 
       TSHORT TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2, 0)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, 0);

            $$ = new NodeDecl($2, $7, 0);
            maxType = 3;
        }
     }
     |TLET TIDENT TCOLON 
      {
        maxType = 1;
      }
      TINT TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2, 1)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, 1);
 
            $$ = new NodeDecl($2, $7, 1);
            maxType = 3;
 
        }
     }
     | TLET TIDENT TCOLON 
     {
        maxType = 2;
     }
     | TLET TIDENT TCOLON 
     {
        maxType = 2;
     }
     TLONG TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2, 2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, 2);
 
            $$ = new NodeDecl($2, $7, 2);
            maxType = 3;
 
        }
     }
     | TDBG Expr TSCOL
     { 
        $$ = new NodeDebug($2);
     }
     | TIF Expr TLBRACE
     {
        symbol_table.enterScope();
     }
     StmtList TRBRACE
     {
        symbol_table.exitScope();
     }
     TELSE TLBRACE
     {
        symbol_table.enterScope();
     }
     StmtList TRBRACE
     {
         $$ = new NodeIfElse($2, $5, $11);
         symbol_table.exitScope();
     }
     ;

Expr : TINT_LIT               
     { $$ = new NodeInt(stoi($1)); }
     | TIDENT 
     { 
        if(symbol_table.contains($1, 0)){
            $$ = new NodeIdent($1); 
            exists = true;
            // printf("0 %d\n", maxType);
        }
        else if(symbol_table.contains($1, 1)){
            $$ = new NodeIdent($1); 
            exists = true;
            // printf("1 %d\n", maxType);
 
            if(maxType < 1){
                yyerrortype("int can't be coerced to ", maxType);
            }
        }
        else if(symbol_table.contains($1, 2)){
            $$ = new NodeIdent($1); 
            exists = true;
            // printf("2 %d\n", maxType);
 
            if(maxType < 2){
                yyerrortype("long can't be coerced to ", maxType);
            }
        }
        if(!exists){
            yyerror("using undeclared variable.\n");
        }             
 
     }
     | Expr TPLUS Expr
     { $$ = new NodeBinOp(NodeBinOp::PLUS, $1, $3); }
     | Expr TDASH Expr
     { $$ = new NodeBinOp(NodeBinOp::MINUS, $1, $3); }
     | Expr TSTAR Expr
     { $$ = new NodeBinOp(NodeBinOp::MULT, $1, $3); } 
     | Expr TSLASH Expr
     { $$ = new NodeBinOp(NodeBinOp::DIV, $1, $3); }
     | TLPAREN Expr TRPAREN { $$ = $2; }
     ;

%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}

int yyerrortype(std::string msg, int type) {
    if(type == 0){
        std::cerr << "Error! " << msg << "short" << std::endl;
    }
    if(type == 1){
        std::cerr << "Error! " << msg << "int" << std::endl;
    }
}
