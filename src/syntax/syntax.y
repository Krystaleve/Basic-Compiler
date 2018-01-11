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

    extern int yylex();
    extern int yyerror(const char *error_str);

    #define EXPRESSION_TODO(x) x = new YacErrorExpression, std::cerr << "yac: " << YacSyntaxError("unsupported expression") << std::endl
%}

%union
{
    int token;
    std::string *string;
    llvm::Type *type;
    llvm::Value *value;
    YacDeclaratorBuilder *declarator;
    YacDeclaratorBuilderList *declarator_list;
    YacExpression *expression;
    YacExpressionList *expression_list;
    YacSyntaxTreeNode *node;
    YacSyntaxTreeNodeList *node_list;
    YacDeclaration *declaration;
    YacDeclarationList *declaration_list;
    YacFunctionDefinition *function;
    YacScope *scope;
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

%token CASE DEFAULT IF THEN ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type <type> type_specifier
%type <declarator> declarator direct_declarator abstract_declarator direct_abstract_declarator
%type <declarator_list> declarator_list
%type <token> assignment_operator
%type <expression> expression primary_expression postfix_expression unary_expression multiplicative_expression additive_expression
%type <expression> shift_expression relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression
%type <expression> logical_and_expression logical_or_expression conditional_expression assignment_expression expression_statement
%type <expression_list> argument_expression_list
%type <node> statement jump_statement
%type <node_list> statement_list
%type <declaration> function_definition parameter_declaration
%type <declaration_list> declaration declaration_list parameter_list external_declaration
%type <function> function_definition_start
%type <scope> compound_statement compound_statement_start translation_unit compound_statement_end

%nonassoc THEN
%nonassoc ELSE
%error-verbose

%start translation_unit

%%

primary_expression
	: IDENTIFIER         {
	    auto value = findInScopes(*$1);
	    if (!value) {
	       std::cerr << "yac: " << YacSyntaxError("unknown identifier `" + *$1 + "\'") << std::endl;
	       $$ = new YacErrorExpression;
	    } else
	        $$ = new YacObjectExpression(value);
    }
	| INTEGER_CONSTANT   { $$ = new YacConstantExpression($1); }
	| FLOAT_CONSTANT     { EXPRESSION_TODO($$); } // TODO
	| STRING_LITERAL     { $$ = new YacConstantExpression($1); }
	| '(' expression ')' { $$ = $2; }
	;

postfix_expression
	: primary_expression                                  { $$ = $1; }
	| postfix_expression '[' expression ']'               { EXPRESSION_TODO($$); } // TODO
	| postfix_expression '(' ')'                          { $$ = new YacCallExpression($1); }
	| postfix_expression '(' argument_expression_list ')' { $$ = new YacCallExpression($1, $3); }
	| postfix_expression INC_OP                           { EXPRESSION_TODO($$); } // TODO
	| postfix_expression DEC_OP                           { EXPRESSION_TODO($$); } // TODO
	;

argument_expression_list
	: assignment_expression                               { $$ = new YacExpressionList; $$->push_back($1); }
	| argument_expression_list ',' assignment_expression  { $$ = $1; $$->push_back($3); }
	;

unary_expression
	: postfix_expression              { $$ = $1; }
	| INC_OP unary_expression         { EXPRESSION_TODO($$); } // TODO
	| DEC_OP unary_expression         { EXPRESSION_TODO($$); } // TODO
	| '&' unary_expression            { EXPRESSION_TODO($$); } // TODO
	| unary_operator unary_expression { EXPRESSION_TODO($$); } // TODO
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
	| multiplicative_expression '*' unary_expression { $$ = new YacBinaryExpression($1, $3, '*'); }
	| multiplicative_expression '/' unary_expression { $$ = new YacBinaryExpression($1, $3, '/'); }
	| multiplicative_expression '%' unary_expression { $$ = new YacBinaryExpression($1, $3, '%'); }
	;

additive_expression
	: multiplicative_expression                         { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = new YacBinaryExpression($1, $3, '+'); }
	| additive_expression '-' multiplicative_expression { $$ = new YacBinaryExpression($1, $3, '-'); }
	;

shift_expression
	: additive_expression                           { $$ = $1; }
	| shift_expression LEFT_OP additive_expression  { $$ = new YacBinaryExpression($1, $3, LEFT_OP); }
	| shift_expression RIGHT_OP additive_expression { $$ = new YacBinaryExpression($1, $3, RIGHT_OP); }
	;

relational_expression
	: shift_expression                             { $$ = $1; }
	| relational_expression '<' shift_expression   { $$ = new YacBinaryExpression($1, $3, '<'); }
	| relational_expression '>' shift_expression   { $$ = new YacBinaryExpression($1, $3, '>'); }
	| relational_expression LE_OP shift_expression { $$ = new YacBinaryExpression($1, $3, LE_OP); }
	| relational_expression GE_OP shift_expression { $$ = new YacBinaryExpression($1, $3, GE_OP); }
	;

equality_expression
	: relational_expression                           { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = new YacBinaryExpression($1, $3, EQ_OP); }
	| equality_expression NE_OP relational_expression { $$ = new YacBinaryExpression($1, $3, NE_OP); }
	;

and_expression
	: equality_expression                    { $$ = $1; }
	| and_expression '&' equality_expression { $$ = new YacBinaryExpression($1, $3, '&'); }
	;

exclusive_or_expression
	: and_expression                             { $$ = $1; }
	| exclusive_or_expression '^' and_expression { $$ = new YacBinaryExpression($1, $3, '^'); }
	;

inclusive_or_expression
	: exclusive_or_expression                             { $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression { $$ = new YacBinaryExpression($1, $3, '|'); }
	;

logical_and_expression
	: inclusive_or_expression                               { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = new YacBinaryExpression($1, $3, AND_OP); }
	;

logical_or_expression
	: logical_and_expression                             { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = new YacBinaryExpression($1, $3, OR_OP); }
	;

conditional_expression
	: logical_or_expression                                           { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression { $$ = new YacExpression; } // TODO
	;

assignment_expression
	: conditional_expression                                     { $$ = $1; }
	| unary_expression '=' assignment_expression                 { $$ = new YacAssignmentExpression($1, $3); }
	| unary_expression assignment_operator assignment_expression { $$ = new YacCompoundAssignmentExpression($1, $3, $2); }
	;

assignment_operator
	: MUL_ASSIGN   { $$ = '*'; }
	| DIV_ASSIGN   { $$ = '/'; }
	| MOD_ASSIGN   { $$ = '%'; }
	| ADD_ASSIGN   { $$ = '+'; }
	| SUB_ASSIGN   { $$ = '-'; }
	| LEFT_ASSIGN  { $$ = LEFT_OP; }
	| RIGHT_ASSIGN { $$ = RIGHT_OP; }
	| AND_ASSIGN   { $$ = '&'; }
	| XOR_ASSIGN   { $$ = '^'; }
	| OR_ASSIGN    { $$ = '|'; }
	;

expression
    : assignment_expression                { $$ = $1; }
    | expression ',' assignment_expression { $$ = new YacExpression; } // TODO
    ;

declaration
    : type_specifier ';' { $$ = new YacDeclarationList; }
    | type_specifier declarator_list ';' {
        auto list = new YacDeclarationList;
        for (auto declarator: *$2) {
            auto node = new YacDeclaration(declarator->type($1), declarator->identifier());
            list->addNode(node);
            addToTopScope(node);
        }
        $$ = list;
    }
    ;

type_specifier
    : VOID   { $$ = llvm::Type::getVoidTy(YacSemanticAnalyzer::context()); }
    | CHAR   { $$ = llvm::Type::getInt8Ty(YacSemanticAnalyzer::context()); }
    | SHORT  { $$ = llvm::Type::getInt16Ty(YacSemanticAnalyzer::context()); }
    | INT    { $$ = llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()); }
    | LONG   { $$ = llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()); }
    | FLOAT  { $$ = llvm::Type::getFloatTy(YacSemanticAnalyzer::context()); }
    | DOUBLE { $$ = llvm::Type::getDoubleTy(YacSemanticAnalyzer::context()); }
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
	: parameter_declaration                    { $$ = new YacDeclarationList; $$->addNode($1); }
	| parameter_list ',' parameter_declaration { $$ = $1; $$->addNode($3); }
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
	| selection_statement  { $$ = new YacSyntaxTreeNode; } // TODO
	| iteration_statement  { $$ = new YacSyntaxTreeNode; } // TODO
	| jump_statement       { $$ = $1; }
	;

compound_statement_start
    : '{' {
        $$ = new YacScope;
        pushScope($$);
    }

compound_statement_end
    : '}'                                 { $$ = topScope(); popScope(); }
    | statement_list '}'                  { $$ = topScope(); if ($$) $$->addNode($1); popScope(); }
    | declaration_list '}'                { $$ = topScope(); if ($$) $$->addNode($1); popScope(); }
    | declaration_list statement_list '}' { $$ = topScope(); if ($$) { $$->addNode($1); $$->addNode($2); } popScope(); }
    ;

compound_statement
	: compound_statement_start compound_statement_end { $$ = $1; assert($1 == $2); }
	;

declaration_list
	: declaration                  { $$ = $1; }
	| declaration_list declaration {
	    $$ = $1;
	    for (auto node: $2->children)
	        $$->addNode(node);
    }
	;

statement_list
	: statement                { $$ = new YacSyntaxTreeNodeList; $$->addNode($1); }
	| statement_list statement { $$ = $1; $$->addNode($2); }
	;

expression_statement
	: ';'             { $$ = nullptr; }
	| expression ';'  { $$ = $1; }
	;

selection_statement
	: IF '(' expression ')' statement                 %prec THEN
	| IF '(' expression ')' statement ELSE statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	;

jump_statement
	: CONTINUE ';'          { $$ = new YacSyntaxTreeNode; } // TODO
	| BREAK ';'             { $$ = new YacSyntaxTreeNode; } // TODO
	| RETURN ';'            { $$ = new YacReturnStatement; }
	| RETURN expression ';' { $$ = new YacReturnStatement($2); }
	;

translation_unit
	: external_declaration                  {
	    $$ = root;
	    for (auto node: $1->children)
	        $$->addNode(node);
    }
	| translation_unit external_declaration {
	    $$ = $1;
	    for (auto node: $2->children)
	        $$->addNode(node);
    }
	;

external_declaration
	: function_definition { $$ = new YacDeclarationList; $$->addNode($1); }
	| declaration         { $$ = $1; }
	;

function_definition_start
    : type_specifier declarator '{' {
        auto type = $2->type($1);
        if (!type->isFunctionTy()) {
            std::cerr << "yac: " << YacSyntaxError("compound statement after non-function declaration") << std::endl;
            $$ = nullptr;
            pushScope(nullptr);
        } else {
            auto function = dynamic_cast<YacDeclaratorFunction *>($2);
            assert(function);
            auto params = new YacScope, body = new YacScope;
            $$ = new YacFunctionDefinition(llvm::cast<llvm::FunctionType>(type), params, body, $2->identifier());
            addToTopScope($$);
            pushScope(params);
            auto args = function->node();
            if (args)
                for (auto node: args->children) {
                    params->addNode(node);
                    addToTopScope(node);
                }
            pushScope(body);
        }
    }

function_definition
    : function_definition_start compound_statement_end {
        assert($$ == nullptr ? $2 == nullptr : $1->body == $2 && $1->params == topScope());
        popScope();
    }
    ;

%%
