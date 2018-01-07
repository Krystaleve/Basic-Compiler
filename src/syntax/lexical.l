D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
IS			(u|U|l|L)*

%{
    #include <string>
    #include <iostream>
    #include <cctype>
    #include <llvm/IR/LLVMContext.h>
    #include <llvm/IR/Type.h>
    #include <llvm/IR/Constants.h>
    #include "../ast/ast.h"
    #include "../ast/declaration.h"

    #include "syntax.h"

    extern llvm::LLVMContext globalContext;
    unsigned int line_number = 1;

    void yyerror(const char *error_str) {
        std::cerr << "Error occurred at line " << line_number << ": " << error_str <<std::endl;
    }

    // end - begin must be greater than 1
    int parseEscape(const char *str, int begin, int end, char &result) {
        if (str[begin++] != '\\') {
            result = str[begin - 1];
            return begin;
        }
        if (begin == end) {
            std::cerr << "Error occurred at line " << line_number << ": " << "Unknown escape sequence" << std::endl;
            result = '\\';
            return begin;
        }
        switch (str[begin++]) {
            case 'a': result = '\a'; return begin;
            case 'b': result = '\b'; return begin;
            case 'f': result = '\f'; return begin;
            case 'n': result = '\n'; return begin;
            case 'r': result = '\r'; return begin;
            case 't': result = '\t'; return begin;
            case 'v': result = '\v'; return begin;
            case 'x':
                if (begin == end || !isxdigit(str[begin])) {
                    std::cerr << "Error occurred at line " << line_number << ": " << "Unknown escape sequence" << std::endl;
                    result = 'x';
                } else {
                    std::string value;
                    value.push_back(str[begin++]);
                    if (begin != end && isxdigit(str[begin]))
                        value.push_back(str[begin++]);
                    result = std::stoi(value, nullptr, 16);
                }
                return begin;
            case '\'': case '\"': case '\?': case '\\':
                result = str[begin - 1];
                return begin;
            default:
                if ('0' <= str[begin - 1] && str[begin - 1] <= '7') {
                    std::string value;
                    value.push_back(str[begin - 1]);
                    for (int i = 0; i < 2 && begin != end && '0' <= str[begin] && str[begin] <= '7'; ++i)
                        value.push_back(str[begin++]);
                    result = std::stoi(value, nullptr, 8);
                } else {
                    std::cerr << "Error occurred at line " << line_number << ": " << "Unknown escape sequence" << std::endl;
                    result = str[begin - 1];
                }
                return begin;
        }
    }
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

{L}({L}|{D})*		{
    yylval.string = new std::string(yytext, yyleng);
    return IDENTIFIER;
}

(0[xX]{H}+|0[0-7]*|[1-9]{D}*){IS}? {
    bool isSigned = true, isLong = false;
    std::string input(yytext, yyleng);
    while (true) {
        if (input.back() == 'u' || input.back() == 'U')
            isSigned = false;
        else if (input.back() == 'l' || input.back() == 'L')
            isLong = true;
        else
            break;
        input.pop_back();
    }
    if (isLong)
        yylval.value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(globalContext), std::stol(input), isSigned);
    else
        yylval.value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(globalContext), std::stoi(input), isSigned);
    return INTEGER_CONSTANT;
}

'(\\.|[^\\'\n])+'	{
    char ch;
    if (parseEscape(yytext, 1, yyleng - 1, ch) != yyleng - 1)
        std::cerr << "Error occurred at line " << line_number << ": " << "Multi-character character constant" << std::endl;
    yylval.value = llvm::ConstantInt::get(llvm::Type::getInt8Ty(globalContext), ch, true);
    return INTEGER_CONSTANT;
}

\"(\\.|[^\\"])*\"	{
    std::string value;
    char ch;
    int begin = 1, end = yyleng - 1;
    while (begin != end) {
        begin = parseEscape(yytext, begin, end, ch);
        value.push_back(ch);
    }
    yylval.value = llvm::ConstantDataArray::getString(globalContext, value);
    return STRING_LITERAL;
}

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
"\n"            ++line_number;

. {
    std::cerr << "Error occurred at line " << line_number << ": Unexpected token '" << yytext << "'."<< std::endl;
    yyterminate();
}

%%
