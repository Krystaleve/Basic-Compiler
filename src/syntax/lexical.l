D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+

%{
    #include <string>
    #include <iostream>
    #include <llvm/IR/LLVMContext.h>
    #include <llvm/IR/Type.h>
    #include "../ast/ast.h"
    #include "../ast/declaration.h"

    #include "syntax.h"

    unsigned int orig_line = 1;

    int yyerror(const char *error_str) {
        std::cerr << "Error occurred at line " << orig_line << ": " << error_str <<std::endl;
    }

    #define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
    #define SAVE_INTEGER yylval.integer = std::stoi(std::string(yytext, yyleng))
%}

%option noyywrap

%%
"auto"			return AUTO;
"break"			return BREAK;
"case"			return CASE;
"char"			return CHAR;
"const"			return CONST;
"continue"		return CONTINUE;
"default"		return DEFAULT;
"do"			return DO;
"double"		return DOUBLE;
"else"			return ELSE;
"enum"			return ENUM;
"extern"		return EXTERN;
"float"			return FLOAT;
"for"			return FOR;
"goto"			return GOTO;
"if"			return IF;
"int"			return INT;
"long"			return LONG;
"register"		return REGISTER;
"return"		return RETURN;
"short"			return SHORT;
"signed"		return SIGNED;
"sizeof"		return SIZEOF;
"static"		return STATIC;
"struct"		return STRUCT;
"switch"		return SWITCH;
"typedef"		return TYPEDEF;
"union"			return UNION;
"unsigned"		return UNSIGNED;
"void"			return VOID;
"volatile"		return VOLATILE;
"while"			return WHILE;

0[xX]{H}+		{ SAVE_INTEGER; return INTEGER_CONSTANT; }
0[0-7]*	    	{ SAVE_INTEGER; return INTEGER_CONSTANT; }
[1-9]{D}*		{ SAVE_INTEGER; return INTEGER_CONSTANT; }

{L}({L}|{D})*		{ SAVE_TOKEN; return IDENTIFIER; }

\"(\\.|[^\\"])*\"	return STRING_LITERAL;

"..."			return ELLIPSIS;
">>="			return RIGHT_ASSIGN;
"<<="			return LEFT_ASSIGN;
"+="			return ADD_ASSIGN;
"-="			return SUB_ASSIGN;
"*="			return MUL_ASSIGN;
"/="			return DIV_ASSIGN;
"%="			return MOD_ASSIGN;
"&="			return AND_ASSIGN;
"^="			return XOR_ASSIGN;
"|="			return OR_ASSIGN;
">>"			return RIGHT_OP;
"<<"			return LEFT_OP;
"++"			return INC_OP;
"--"			return DEC_OP;
"->"			return PTR_OP;
"&&"			return AND_OP;
"||"			return OR_OP;
"<="			return LE_OP;
">="			return GE_OP;
"=="			return EQ_OP;
"!="			return NE_OP;
";"			    return ';';
("{"|"<%")		return '{';
("}"|"%>")		return '}';
","		    	return ',';
":"		    	return ':';
"="		    	return '=';
"("		    	return '(';
")"		    	return ')';
("["|"<:")		return '[';
("]"|":>")		return ']';
"."			    return '.';
"&"			    return '&';
"!"			    return '!';
"~"			    return '~';
"-"			    return '-';
"+"			    return '+';
"*"			    return '*';
"/"			    return '/';
"%"			    return '%';
"<"			    return '<';
">"			    return '>';
"^"			    return '^';
"|"			    return '|';
"?"			    return '?';

[ \t\v\f]		;
"\n"            ++orig_line;

. {
    std::cerr << "Error occurred at line " << orig_line << ": Unexpected token '" << yytext << "'."<< std::endl;
    yyterminate();
}

%%
