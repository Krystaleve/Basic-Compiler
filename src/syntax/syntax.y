%{
    #include <string>
    #include <iostream>
    #include <llvm/IR/LLVMContext.h>
    #include <llvm/IR/DerivedTypes.h>
    #include <llvm/IR/Type.h>
    #include "../ast/ast.h"
    #include "../ast/declarator.h"

    extern int yylex();
    extern int yyerror(const char *error_str);

    extern llvm::LLVMContext globalContext;
    extern YacSyntaxTreeNodeList *root;
%}

%union
{
    int integer;
    std::string *string;
    llvm::Type *type;
    YacDeclaratorBuilder *declarator;
    YacDeclaratorBuilderList *declarator_list;
    YacSyntaxTreeNode *node;
}


%token <string> IDENTIFIER
%token <integer> INTEGER_CONSTANT
%token FLOAT_CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type <type> type_specifier
%type <declarator> declarator direct_declarator abstract_declarator direct_abstract_declarator
%type <declarator_list> declarator_list
%type <node> external_declaration function_definition declaration parameter_declaration translation_unit parameter_list

%error-verbose

%start translation_unit

%%


primary_expression
	: IDENTIFIER
	| INTEGER_CONSTANT
	| FLOAT_CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	/*| postfix_expression PTR_OP IDENTIFIER*/
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator unary_expression
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

multiplicative_expression
	: unary_expression
	| multiplicative_expression '*' unary_expression
	| multiplicative_expression '/' unary_expression
	| multiplicative_expression '%' unary_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression '=' assignment_expression
	;

expression
    : assignment_expression
    | expression ',' assignment_expression
    ;

declaration
    : type_specifier ';' { $$ = new YacSyntaxTreeNodeList; }
    | type_specifier declarator_list ';' {
        auto node_list = new YacSyntaxTreeNodeList;
        for (auto declarator: *$2)
            node_list->children.push_back(new YacDeclaration(declarator->type($1), declarator->identifier()));
        $$ = node_list;
    }
    ;

type_specifier
    : VOID   { $$ = llvm::Type::getVoidTy(globalContext); }
    | CHAR   { $$ = llvm::Type::getInt8Ty(globalContext);; }
    | SHORT  { $$ = llvm::Type::getInt16Ty(globalContext);; }
    | INT    { $$ = llvm::Type::getInt32Ty(globalContext);; }
    | LONG   { $$ = llvm::Type::getInt32Ty(globalContext);; }
    | FLOAT  { $$ = llvm::Type::getFloatTy(globalContext);; }
    | DOUBLE { $$ = llvm::Type::getDoubleTy(globalContext);; }
    ;

declarator_list
    : declarator                     { $$ = new YacDeclaratorBuilderList; $$->push_back($1); }
    | declarator_list ',' declarator { $$->push_back($3); }
    ;

declarator
    : '*' declarator        { $$ = new YacDeclaratorPointer($2); }
    | direct_declarator     { $$ = $1; }
    ;

direct_declarator
	: IDENTIFIER                                            { $$ = new YacDeclaratorIdentifier($1); }
	| '(' declarator ')'                                    { $$ = $2; }
	| direct_declarator '[' INTEGER_CONSTANT ']'            { $$ = new YacDeclaratorArray($1, $3); }
	| direct_declarator '(' parameter_list ')'              { $$ = new YacDeclaratorFunction($1, $3); }
	| direct_declarator '(' parameter_list ',' ELLIPSIS ')' { $$ = new YacDeclaratorFunction($1, $3, true); }
	| direct_declarator '(' ')'                             { $$ = new YacDeclaratorFunction($1); }
	;

parameter_list
	: parameter_declaration                    { $$ = new YacSyntaxTreeNodeList; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($1); }
	| parameter_list ',' parameter_declaration { $$ = $1; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($3); }
	;

parameter_declaration
	: type_specifier declarator          { $$ = new YacDeclaration($2->type($1), $2->identifier()); }
	| type_specifier abstract_declarator { $$ = new YacDeclaration($2->type($1), $2->identifier()); }
	| type_specifier                     { $$ = new YacDeclaration($1); }
	;

abstract_declarator
	: '*'                                { $$ = new YacDeclaratorPointer(new YacDeclaratorIdentifier); }
	| '*' abstract_declarator            { $$ = new YacDeclaratorPointer($2); }
	| direct_abstract_declarator         { $$ = $1; }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'                                    { $$ = $2; }
	| '[' ']'                                                        { $$ = new YacDeclaratorPointer(new YacDeclaratorIdentifier); }
	| direct_abstract_declarator '[' ']'                             { $$ = new YacDeclaratorPointer($1); }
	| '(' ')'                                                        { $$ = new YacDeclaratorFunction(new YacDeclaratorIdentifier); }
	| '(' parameter_list ')'                                         { $$ = new YacDeclaratorFunction(new YacDeclaratorIdentifier, $2); }
	| '(' parameter_list ',' ELLIPSIS ')'                            { $$ = new YacDeclaratorFunction(new YacDeclaratorIdentifier, $2, true); }
	| direct_abstract_declarator '(' ')'                             { $$ = new YacDeclaratorFunction($1); }
	| direct_abstract_declarator '(' parameter_list ')'              { $$ = new YacDeclaratorFunction($1, $3); }
	| direct_abstract_declarator '(' parameter_list ',' ELLIPSIS ')' { $$ = new YacDeclaratorFunction($1, $3, true); }
	;

statement
	: compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

compound_statement
	: '{' '}'
	| '{' statement_list '}'
	| '{' declaration_list '}'
	| '{' declaration_list statement_list '}'
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: ';'
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	;

jump_statement
	: CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration                  { $$ = root = new YacSyntaxTreeNodeList; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($1); };
	| translation_unit external_declaration { $$ = $1; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($2); }
	;

external_declaration
	: function_definition { $$ = $1; }
	| declaration         { $$ = $1; }
	;

function_definition
    : type_specifier declarator compound_statement { $$ = nullptr; }
    ;

%%
