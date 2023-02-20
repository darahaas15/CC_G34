/* just like Unix wc */
%option noyywrap
%option prefix="foo"

%x COMMENT
%x COMMENT_ALT
%x DEFINE
%x DEFINE_ALT
%x UNDEF
%x IFDEF
%x ELIF
%x ELSE
%x ENDIF
%{
#include <string>
#include <unordered_map>
using namespace std;

string key;
unordered_map<string, string> map;
bool skip_section = false;
bool elif_executed = false;
%}
%%

"#def " {BEGIN(DEFINE); return 1;}
<DEFINE>[a-zA-Z]+ {key = yytext; map[key]="1"; return 1;}
<DEFINE>[\n]+ {BEGIN(INITIAL); return 1;}
<DEFINE>" " {BEGIN(DEFINE_ALT); return 1;}
<DEFINE_ALT>[^\\\n]+ {if(map[key] == "1") map[key] = ""; map[key] += yytext; return 5;}
<DEFINE_ALT>"\\\n" {return 1;}
<DEFINE_ALT>[\n]+ {BEGIN(INITIAL); return 1;}

"#undef " {BEGIN(UNDEF); return 2;}
<UNDEF>[a-zA-Z]+ {map.erase(yytext); return 2;}
<UNDEF>[ \n]+ {BEGIN(INITIAL); return 2;}

"#ifdef " {BEGIN(IFDEF); return 6;}
<IFDEF>[a-zA-Z]+ {skip_section = (map[yytext] != "1"); return 6;}
<IFDEF>[ \n]+ {BEGIN(INITIAL); return 6;}

"#elif " {BEGIN(ELIF); return 7;}
<ELIF>[a-zA-Z]+ {elif_executed = skip_section || elif_executed || (map[yytext] == "1"); skip_section = !elif_executed; return 7;}
<ELIF>[ \n]+ {BEGIN(INITIAL); return 7;}

"#else" {BEGIN(ELSE); return 8;}
<ELSE>[ \n]+ {skip_section = !elif_executed && skip_section; BEGIN(INITIAL); return 8;}

"#endif" {BEGIN(ENDIF); return 9;}
<ENDIF>[ \n]+ {skip_section = false; elif_executed = false; BEGIN(INITIAL); return 9;}

"/*"         BEGIN(COMMENT);
<COMMENT>[^*]*        /* eat anything that's not a '*' */
<COMMENT>"*"+[^*/]*   /* eat up '*'s not followed by '/'s */
<COMMENT>"*"+"/"        {BEGIN(INITIAL);}

"//"    BEGIN(COMMENT_ALT);
<COMMENT_ALT>. /* om nom */
<COMMENT_ALT>[ \n]+ {BEGIN(INITIAL);}

[a-zA-Z]+ {if (!skip_section) return 3;}
. {if (!skip_section) return 4;}
%%
