%{
    #include <string>
    #include <iostream>

    #include "syntax.hpp"

    unsigned int orig_line = 1;

    int yyerror(const char *error_str) {
        std::cerr << "Error occurred at line " << orig_line << ": " << error_str <<std::endl;
    }
%}

%option noyywrap

%%

"extern" {
    yylval.token = CEXTERN;
    return CEXTERN;
}

"return" {
    yylval.token = CRETURN;
    return CRETURN;
}

"int" {
    yylval.token = CINT;
    return CINT;
}

"double" {
    yylval.token = CDOUBLE;
    return CDOUBLE;
}

"char" {
    yylval.token = CCHAR;
    return CCHAR;
}

"void" {
    yylval.token = CVOID;
    return CVOID;
}

"if" {
    yylval.token = CIF;
    return CIF;
}

"else" {
    yylval.token = CELSE;
    return CELSE;
}

"for" {
    yylval.token = CFOR;
    return CFOR;
}

"while" {
    yylval.token = CWHILE;
    return CWHILE;
}

/* TODO: switch, do-while, goto */

"(" {
    yylval.token = CLPAREN;
    return CLPAREN;
}

")" {
    yylval.token = CRPAREN;
    return CRPAREN;
}

"[" {
    yylval.token = CLBRACKET;
    return CLBRACKET;
}

"]" {
    yylval.token = CRBRACKET;
    return CRBRACKET;
}

"{" {
    yylval.token = CLBRACE;
    return CLBRACE;
}

"}" {
    yylval.token = CRBRACE;
    return CRBRACE;
}

"=" {
    yylval.token = CASSIGNMENT;
    return CASSIGNMENT;
}

"==" {
    yylval.token = CEQUAL;
    return CEQUAL;
}

"!=" {
    yylval.token = CNOTEQUAL;
    return CNOTEQUAL;
}

">" {
    yylval.token = CGREATER;
    return CGREATER;
}

">=" {
    yylval.token = CGREATEROREQUAL;
    return CGREATEROREQUAL;
}

"<" {
    yylval.token = CLESS;
    return CLESS;
}

"<=" {
    yylval.token = CLESSOREQUAL;
    return CLESSOREQUAL;
}

"&&" {
    yylval.token = CLOGICALAND;
    return CLOGICALAND;
}

"||" {
    yylval.token = CLOGICALOR;
    return CLOGICALOR;
}

"&" {
    yylval.token = CBITWISEAND;
    return CBITWISEAND;
}

"|" {
    yylval.token = CBITWISEOR;
    return CBITWISEOR;
}

"^" {
    yylval.token = CBITWISEXOR;
    return CBITWISEXOR;
}

"<<" {
    yylval.token = CLSHIFT;
    return CLSHIFT;
}

">>" {
    yylval.token = CRSHIFT;
    return CRSHIFT;
}

"+" {
    yylval.token = CPLUS;
    return CPLUS;
}

"-" {
    yylval.token = CMINUS;
    return CMINUS;
}

"*" {
    yylval.token = CMULTIPLE;
    return CMULTIPLE;
}

"/" {
    yylval.token = CDIVIDE;
    return CDIVIDE;
}

"%" {
    yylval.token = CMOD;
    return CMOD;
}

"#" {
    yylval.token = CSHARP;
    return CSHARP;
}

"~" {
    yylval.token = CTILDE;
    return CTILDE;
}

"." {
    yylval.token = CDOT;
    return CDOT;
}

"," {
    yylval.token = CCOMMA;
    return CCOMMA;
}

":" {
    yylval.token = CCOLON;
    return CCOLON;
}

";" {
    yylval.token = CSEMICOLON;
    return CSEMICOLON;
}

\".*\" {
    yylval.string = new std::string;

    --yyleng;
    for (int i = 1; i < yyleng; ++i) {
        if (yytext[i] == '\\') {
            switch (yytext[i + 1]) {
                case 't':
                    yylval.string->push_back('\t');
                    break;
                case 'r':
                    yylval.string->push_back('\r');
                    break;
                case 'n':
                    yylval.string->push_back('\n');
                    break;
                default:
                    break;
            }
        } else
            yylval.string->push_back(yytext[i]);
    }
    return CSTRINGVALUE;
}

[_A-Za-z][_0-9A-Za-z]* {
    yylval.string = new std::string(yytext, yyleng);
    return CIDENTIFIER;
}

[0-9]+ {
    yylval.string = new std::string(yytext, yyleng);
    return CINTVALUE;
}

[0-9]+\.[0-9]* {
    yylval.string = new std::string(yytext, yyleng);
    return CDOUBLEVALUE;
}

\'.\' {
    yylval.string = new std::string(yytext + 1, yyleng - 2);
    return CCHARVALUE;
}

\/\/.* {};

[ \t\r]* {};

"\n" {
    orig_line++;
};

^"#include ".+ {};

. {
    std::cerr << "Error occurred at line " << orig_line << ": Unexpected token '" << yytext << "'."<< std::endl;
    yyterminate();
}

%%
