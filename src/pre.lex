/* just like Unix wc */
%option noyywrap
%option prefix="foo"

%x COMMENT
%x COMMENT_ALT
%x DEFINE
%x DEFINE_ALT
%x UNDEF
%{
#include <string>
#include <unordered_map>
using namespace std;

string key;
unordered_map<string, string> map;
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


"/*"         BEGIN(COMMENT);
<COMMENT>[^*]*        /* eat anything that's not a '*' */
<COMMENT>"*"+[^*/]*   /* eat up '*'s not followed by '/'s */
<COMMENT>"*"+"/"        {BEGIN(INITIAL);}

"//"    BEGIN(COMMENT_ALT);
<COMMENT_ALT>. /* om nom */
<COMMENT_ALT>[ \n]+ {BEGIN(INITIAL);}

[a-zA-Z]+ {return 3;}
. {return 4;}
%%