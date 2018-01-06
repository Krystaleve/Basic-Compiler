%{
    #include <iostream>

    extern int yylex();
    extern int yyerror(const char *error_str);
%}

%union
{
 std::string *string;
 int token;
}

%error-verbose

%start translation_unit

%%


%%
