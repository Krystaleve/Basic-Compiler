D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
IS			(u|U|l|L)*
FS			(f|F|l|L)

%{
    #include <string>
    #include <iostream>
    #include <cctype>
    #include <llvm/IR/LLVMContext.h>
    #include <llvm/IR/Type.h>
    #include <llvm/IR/Constants.h>
    #include "../ast/ast.h"
    #include "../ast/declaration.h"
    #include "../ast/expression.h"
    #include "../ast/context.h"

    #include "syntax.h"

    void yyerror(const char *error_str) {
        std::cerr << "yac: " << YacSyntaxError(error_str) << std::endl;
    }

    // end - begin must be greater than 1
    int parseEscape(const char *str, int begin, int end, char &result) {
        if (str[begin++] != '\\') {
            result = str[begin - 1];
            return begin;
        }
        if (begin == end) {
            yyerror("unexpected ending of escape sequence");
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
                    if (begin == end)
                        yyerror("unexpected ending of escape sequence");
                    else
                        yyerror("unknown escape sequence");
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
            	    yyerror("unknown escape sequence");
                    result = str[begin - 1];
                }
                return begin;
        }
    }

    #define NC column_number += yyleng
%}

%%

"/*"			{
    char c, prev = 0;
	while ((c = yyinput()) != 0 && (c != '/' || prev != '*')) {
	    if (c == '\n') {
	        ++line_number;
	        column_number = 1;
	    } else
	        ++column_number;

		prev = c;
	}
	if (c == 0)
	    yyerror("unterminated comment");
}
"//"[^\n]*      NC;

"#"[ \t\f]+{D}+[ \t\f]+\"(\\.|[^\\"\n])*\"([ \t\f]+[1234])*	{
    // TODO: Implements trace back
    NC;
    int begin = 1, end = yyleng;
    line_number = 0;
    column_number = 1;
    while (begin < end && isspace(yytext[begin])) ++begin;
    while (begin < end && isdigit(yytext[begin]))
        line_number = line_number * 10 + (yytext[begin++] - '0');
    --line_number;
    input.clear();
    while (begin < end && yytext[begin] != '\"') ++begin;
    ++begin;
    while (begin < end && yytext[end] != '\"') --end;
    while (begin < end) {
        char ch;
        begin = parseEscape(yytext, begin, end, ch);
        input.push_back(ch);
    }
}

"#"[^\n]*       NC;

"auto"			NC; return AUTO;
"break"			NC; return BREAK;
"case"			NC; return CASE;
"char"			NC; return CHAR;
"const"			NC; return CONST;
"continue"		NC; return CONTINUE;
"default"		NC; return DEFAULT;
"do"			NC; return DO;
"double"		NC; return DOUBLE;
"else"			NC; return ELSE;
"enum"			NC; return ENUM;
"extern"		NC; return EXTERN;
"float"			NC; return FLOAT;
"for"			NC; return FOR;
"goto"			NC; return GOTO;
"if"			NC; return IF;
"int"			NC; return INT;
"long"			NC; return LONG;
"register"		NC; return REGISTER;
"return"		NC; return RETURN;
"short"			NC; return SHORT;
"signed"		NC; return SIGNED;
"sizeof"		NC; return SIZEOF;
"static"		NC; return STATIC;
"struct"		NC; return STRUCT;
"switch"		NC; return SWITCH;
"typedef"		NC; return TYPEDEF;
"union"			NC; return UNION;
"unsigned"		NC; return UNSIGNED;
"void"			NC; return VOID;
"volatile"		NC; return VOLATILE;
"while"			NC; return WHILE;

{L}({L}|{D})*		{
    NC;
    yylval.string = new std::string(yytext, yyleng);
    return IDENTIFIER;
}

(0[xX]{H}+|0[0-7]*|[1-9]{D}*){IS}? {
    NC;
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
        yylval.value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()), std::stol(input), isSigned);
    else
        yylval.value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()), std::stoi(input), isSigned);
    return INTEGER_CONSTANT;
}

'(\\.|[^\\'\n])+'	{
    NC;
    char ch;
    if (parseEscape(yytext, 1, yyleng - 1, ch) != yyleng - 1)
        yyerror("multi-character character constant");
    yylval.value = llvm::ConstantInt::get(llvm::Type::getInt8Ty(YacSemanticAnalyzer::context()), ch, true);
    return INTEGER_CONSTANT;
}

\"(\\.|[^\\"\n])*\"	{
    NC;
    std::string value;
    char ch;
    int begin = 1, end = yyleng - 1;
    while (begin != end) {
        begin = parseEscape(yytext, begin, end, ch);
        value.push_back(ch);
    }
    yylval.value = llvm::ConstantDataArray::getString(YacSemanticAnalyzer::context(), value);
    return STRING_LITERAL;
}

{D}+{E}{FS}?		    NC; return(FLOAT_CONSTANT);
{D}*"."{D}+({E})?{FS}?	NC; return(FLOAT_CONSTANT);
{D}+"."{D}*({E})?{FS}?	NC; return(FLOAT_CONSTANT);

"..."			NC; return ELLIPSIS;
">>="			NC; return RIGHT_ASSIGN;
"<<="			NC; return LEFT_ASSIGN;
"+="			NC; return ADD_ASSIGN;
"-="			NC; return SUB_ASSIGN;
"*="			NC; return MUL_ASSIGN;
"/="			NC; return DIV_ASSIGN;
"%="			NC; return MOD_ASSIGN;
"&="			NC; return AND_ASSIGN;
"^="			NC; return XOR_ASSIGN;
"|="			NC; return OR_ASSIGN;
">>"			NC; return RIGHT_OP;
"<<"			NC; return LEFT_OP;
"++"			NC; return INC_OP;
"--"			NC; return DEC_OP;
"->"			NC; return PTR_OP;
"&&"			NC; return AND_OP;
"||"			NC; return OR_OP;
"<="			NC; return LE_OP;
">="			NC; return GE_OP;
"=="			NC; return EQ_OP;
"!="			NC; return NE_OP;
";"			    NC; return ';';
("{"|"<%")		NC; return '{';
("}"|"%>")		NC; return '}';
","		    	NC; return ',';
":"		    	NC; return ':';
"="		    	NC; return '=';
"("		    	NC; return '(';
")"		    	NC; return ')';
("["|"<:")		NC; return '[';
("]"|":>")		NC; return ']';
"."			    NC; return '.';
"&"			    NC; return '&';
"!"			    NC; return '!';
"~"			    NC; return '~';
"-"			    NC; return '-';
"+"			    NC; return '+';
"*"			    NC; return '*';
"/"			    NC; return '/';
"%"			    NC; return '%';
"<"			    NC; return '<';
">"			    NC; return '>';
"^"			    NC; return '^';
"|"			    NC; return '|';
"?"			    NC; return '?';

[ \t\v\f]		NC;
"\n"            ++line_number; column_number = 1;

. {
    NC;
    yyerror((std::string("unexpected character \'") + *yytext + "\' (" + std::to_string((int)*yytext) + ")").c_str());
}

%%

int yywrap(void)
{
	return 1;
}

