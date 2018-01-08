%{
    #include <string>
    #include <iostream>
    #include <llvm/IR/LLVMContext.h>
    #include <llvm/IR/DerivedTypes.h>
    #include <llvm/IR/Type.h>
    #include <llvm/IR/Constants.h>
    #include "../ast/ast.h"
    #include "../ast/declaration.h"
    #include "../ast/expression.h"
    #include "../ast/statement.h"
    #include "../ast/type.h"

    extern int line_number;
    extern int yylex();
    extern int yyerror(const char *error_str);

    extern llvm::LLVMContext globalContext;
    extern YacSyntaxTreeNodeList *root;
%}

%union
{
    std::string *string;
    llvm::Type *type;
    llvm::Value *value;
    YacDeclaratorBuilder *declarator;
    YacDeclaratorBuilderList *declarator_list;
    YacExpression *expression;
    YacExpressionList *expression_list;
    YacSyntaxTreeNode *node;
}


%token <string> IDENTIFIER
%token <value> INTEGER_CONSTANT STRING_LITERAL
%token FLOAT_CONSTANT SIZEOF
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
%type <expression> expression primary_expression postfix_expression unary_expression multiplicative_expression additive_expression
%type <expression> shift_expression relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression
%type <expression> logical_and_expression logical_or_expression conditional_expression assignment_expression expression_statement
%type <expression_list> argument_expression_list
%type <node> statement compound_statement statement_list declaration_list jump_statement
%type <node> external_declaration function_definition declaration parameter_declaration translation_unit parameter_list

%error-verbose

%start translation_unit

%%

primary_expression
	: IDENTIFIER         { $$ = new YacIdentifierExpression($1); }
	| INTEGER_CONSTANT   { $$ = new YacConstantExpression($1); }
	| FLOAT_CONSTANT     { $$ = new YacExpression; } // TODO
	| STRING_LITERAL     { $$ = new YacConstantExpression($1); }
	| '(' expression ')' { $$ = $2; }
	;

postfix_expression
	: primary_expression                                  { $$ = $1; }
	| postfix_expression '[' expression ']'               { $$ = new YacExpression; } // TODO
	| postfix_expression '(' ')'                          { $$ = new YacCallExpression($1); }
	| postfix_expression '(' argument_expression_list ')' { $$ = new YacCallExpression($1, $3); } // TODO
	| postfix_expression INC_OP                           { $$ = new YacExpression; } // TODO
	| postfix_expression DEC_OP                           { $$ = new YacExpression; } // TODO
	;

argument_expression_list
	: assignment_expression                               { $$ = new YacExpressionList; $$->push_back($1); }
	| argument_expression_list ',' assignment_expression  { $$ = $1; $$->push_back($3); }
	;

unary_expression
	: postfix_expression              { $$ = $1; }
	| INC_OP unary_expression         { $$ = new YacExpression; } // TODO
	| DEC_OP unary_expression         { $$ = new YacExpression; } // TODO
	| '&' unary_expression            { $$ = new YacExpression; } // TODO
	| unary_operator unary_expression { $$ = new YacExpression; } // TODO
	;

unary_operator
	: '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

multiplicative_expression
	: unary_expression                               { $$ = $1; }
	| multiplicative_expression '*' unary_expression { $$ = new YacExpression; } // TODO
	| multiplicative_expression '/' unary_expression { $$ = new YacExpression; } // TODO
	| multiplicative_expression '%' unary_expression { $$ = new YacExpression; } // TODO
	;

additive_expression
	: multiplicative_expression                         { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = new YacExpression; } // TODO
	| additive_expression '-' multiplicative_expression { $$ = new YacExpression; } // TODO
	;

shift_expression
	: additive_expression                           { $$ = $1; }
	| shift_expression LEFT_OP additive_expression  { $$ = new YacExpression; } // TODO
	| shift_expression RIGHT_OP additive_expression { $$ = new YacExpression; } // TODO
	;

relational_expression
	: shift_expression                             { $$ = $1; }
	| relational_expression '<' shift_expression   { $$ = new YacExpression; } // TODO
	| relational_expression '>' shift_expression   { $$ = new YacExpression; } // TODO
	| relational_expression LE_OP shift_expression { $$ = new YacExpression; } // TODO
	| relational_expression GE_OP shift_expression { $$ = new YacExpression; } // TODO
	;

equality_expression
	: relational_expression                           { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = new YacExpression; } // TODO
	| equality_expression NE_OP relational_expression { $$ = new YacExpression; } // TODO
	;

and_expression
	: equality_expression                    { $$ = $1; }
	| and_expression '&' equality_expression { $$ = new YacExpression; } // TODO
	;

exclusive_or_expression
	: and_expression                             { $$ = $1; }
	| exclusive_or_expression '^' and_expression { $$ = new YacExpression; } // TODO
	;

inclusive_or_expression
	: exclusive_or_expression                             { $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression { $$ = new YacExpression; } // TODO
	;

logical_and_expression
	: inclusive_or_expression                               { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = new YacExpression; } // TODO
	;

logical_or_expression
	: logical_and_expression                             { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = new YacExpression; } // TODO
	;

conditional_expression
	: logical_or_expression                                           { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression { $$ = new YacExpression; } // TODO
	;

assignment_expression
	: conditional_expression                                     { $$ = $1; }
	| unary_expression '=' assignment_expression                 { $$ = new YacAssignmentExpression($1, $3); }
	| unary_expression assignment_operator assignment_expression { $$ = new YacExpression; } // TODO
	;

assignment_operator
	: MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression
    : assignment_expression                { $$ = $1; }
    | expression ',' assignment_expression { $$ = new YacExpression; } // TODO
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
	| direct_declarator '[' ']'                             { $$ = new YacDeclaratorArray($1, 0); }
	| direct_declarator '[' INTEGER_CONSTANT ']'            { $$ = new YacDeclaratorArray($1, llvm::cast<llvm::ConstantInt>($3)->getLimitedValue()); }
	| direct_declarator '(' parameter_list ')'              { $$ = new YacDeclaratorFunction($1, $3); }
	| direct_declarator '(' parameter_list ',' ELLIPSIS ')' { $$ = new YacDeclaratorFunction($1, $3, true); }
	| direct_declarator '(' ')'                             { $$ = new YacDeclaratorFunction($1); }
	;

parameter_list
	: parameter_declaration                    { $$ = new YacSyntaxTreeNodeList; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($1); }
	| parameter_list ',' parameter_declaration { $$ = $1; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($3); }
	;

parameter_declaration
	: type_specifier declarator          { $$ = new YacDeclaration(castToParameterType($2->type($1)), $2->identifier()); }
	| type_specifier abstract_declarator { $$ = new YacDeclaration(castToParameterType($2->type($1)), $2->identifier()); }
	| type_specifier                     { $$ = new YacDeclaration(castToParameterType($1)); }
	;

abstract_declarator
	: '*'                                { $$ = new YacDeclaratorPointer(new YacDeclaratorIdentifier); }
	| '*' abstract_declarator            { $$ = new YacDeclaratorPointer($2); }
	| direct_abstract_declarator         { $$ = $1; }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'                                    { $$ = $2; }
	| '[' ']'                                                        { $$ = new YacDeclaratorArray(new YacDeclaratorIdentifier, 0); }
	| '[' INTEGER_CONSTANT ']'                                       { $$ = new YacDeclaratorArray(new YacDeclaratorIdentifier, llvm::cast<llvm::ConstantInt>($2)->getLimitedValue()); }
	| direct_abstract_declarator '[' ']'                             { $$ = new YacDeclaratorArray($1, 0); }
	| '(' ')'                                                        { $$ = new YacDeclaratorFunction(new YacDeclaratorIdentifier); }
	| '(' parameter_list ')'                                         { $$ = new YacDeclaratorFunction(new YacDeclaratorIdentifier, $2); }
	| '(' parameter_list ',' ELLIPSIS ')'                            { $$ = new YacDeclaratorFunction(new YacDeclaratorIdentifier, $2, true); }
	| direct_abstract_declarator '(' ')'                             { $$ = new YacDeclaratorFunction($1); }
	| direct_abstract_declarator '(' parameter_list ')'              { $$ = new YacDeclaratorFunction($1, $3); }
	| direct_abstract_declarator '(' parameter_list ',' ELLIPSIS ')' { $$ = new YacDeclaratorFunction($1, $3, true); }
	;

statement
	: compound_statement   { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement  { $$ = new YacSyntaxEmptyNode; } // TODO
	| iteration_statement  { $$ = new YacSyntaxEmptyNode; } // TODO
	| jump_statement       { $$ = $1; }
	;

compound_statement
	: '{' '}'                                 { $$ = nullptr; }
	| '{' statement_list '}'                  { $$ = $2; }
	| '{' declaration_list '}'                { $$ = $2; }
	| '{' declaration_list statement_list '}' {
	    $$ = $2;
	    auto &first = dynamic_cast<YacSyntaxTreeNodeList *>($$)->children;
	    auto &second = dynamic_cast<YacSyntaxTreeNodeList *>($3)->children;
	    first.insert(first.end(), second.begin(), second.end());
    }
	;

declaration_list
	: declaration                  { $$ = new YacSyntaxTreeNodeList; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($1); }
	| declaration_list declaration { $$ = $1; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($2); }
	;

statement_list
	: statement                { $$ = new YacSyntaxTreeNodeList; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($1); }
	| statement_list statement { $$ = $1; dynamic_cast<YacSyntaxTreeNodeList *>($$)->children.push_back($2); }
	;

expression_statement
	: ';'             { $$ = nullptr; }
	| expression ';'  { $$ = $1; }
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
	: CONTINUE ';'          { $$ = new YacSyntaxEmptyNode; } // TODO
	| BREAK ';'             { $$ = new YacSyntaxEmptyNode; } // TODO
	| RETURN ';'            { $$ = new YacReturnStatement; }
	| RETURN expression ';' { $$ = new YacReturnStatement($2); }
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
    : type_specifier declarator compound_statement {
        auto type = $2->type($1);
        if (!type->isFunctionTy()) {
            std::cerr << "Error occurred at line " << line_number << ": " << "Unexpected compound statement after non-function declaration" << std::endl;
            $$ = new YacSyntaxEmptyNode;
        } else {
            std::vector<YacDeclaration *> params;
            auto node_list = dynamic_cast<YacDeclaratorFunction *>($2);
            if (node_list->node()) {
                for (auto node: dynamic_cast<YacSyntaxTreeNodeList *>(node_list->node())->children)
                    params.push_back(dynamic_cast<YacDeclaration *>(node));
            }
            $$ = new YacFunctionDefinition(llvm::cast<llvm::FunctionType>(type), $2->identifier(), std::move(params), $3);
        }
    }
    ;

%%
