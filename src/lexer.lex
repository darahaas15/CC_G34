%option noyywrap

%{
#include "parser.hh"
#include <string>
#include <unordered_map>
    #include <cstring>
    #include <tuple>
    
    std::unordered_map<std::string, std::string> m;

    std::pair<std::string, std::string> tokenise(char* prrtext);
extern int yyerror(std::string msg);
%}

%%

"+"       { return TPLUS; }
"-"       { return TDASH; }
"*"       { return TSTAR; }
"/"       { return TSLASH; }
";"       { return TSCOL; }
"("       { return TLPAREN; }
")"       { return TRPAREN; }
"="       { return TEQUAL; }
"dbg"     { return TDBG; }
"let"     { return TLET; }
[0-9]+    { yylval.lexeme = std::string(yytext); return TINT_LIT; }
[a-zA-Z]+ { yylval.lexeme = std::string(yytext); return TIDENT; }
[ \t\n]   { /* skip */ }
.         { yyerror("unknown char"); }
"//".*                                    { /* DO NOTHING */ }
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { /* DO NOTHING */ }
%%

std::string token_to_string(int token, const char *lexeme) {
    std::string s;
    switch (token) {
        case TPLUS: s = "TPLUS"; break;
        case TDASH: s = "TDASH"; break;
        case TSTAR: s = "TSTAR"; break;
        case TSLASH: s = "TSLASH"; break;
        case TSCOL: s = "TSCOL"; break;
        case TLPAREN: s = "TLPAREN"; break;
        case TRPAREN: s = "TRPAREN"; break;
        case TEQUAL: s = "TEQUAL"; break;
        
        case TDBG: s = "TDBG"; break;
        case TLET: s = "TLET"; break;
        
        case TINT_LIT: s = "TINT_LIT"; s.append("  ").append(lexeme); break;
        case TIDENT: s = "TIDENT"; s.append("  ").append(lexeme); break;
    }

    return s;
}

std::pair<std::string, std::string> tokenise(char *prrtext) {
    //Returns {key, val}
    char* tok1, *tok2, *tok3;
    tok1 = std::strtok(prrtext, " ");
    tok2 = std::strtok(NULL, " ");
    tok3 = tok2 + std::strlen(tok2) + 1;
    std::string s2 = std::string(tok2);
    std::string s3 = std::string(tok3);
    for(char& c: s3) {
        if(c=='\\') c = ' ';
    }
    return {s2, s3};
}