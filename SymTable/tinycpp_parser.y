%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "compile.h"

//Substitute yylex and Define Semantic Value Type as snode
#define YYERROR_VERBOSE
#define YYSTYPE snode
#define yylex tc_lex

void yyerror(const char *s, ...);

%}

%pure_parser

%token T_H_INC
%token T_USING T_NS T_STD
%token T_INT T_BOOL
%token T_CLASS
%token T_PUBLIC
%token T_RETURN
%token T_CIN T_COUT
%token T_WHILE
%token T_IF T_ELSE
%token T_LEFT_OP T_RIGHT_OP T_GE_OP T_LE_OP T_EQ_OP T_NE_OP T_AND_OP T_OR_OP T_OBJ_PT_OPERATOR
%token T_IDENTIFIER T_INT_CONSTANT T_STRING_LITERAL
%token ';' '{' '}' ',' ':' '=' '('	')'	'['	']'	'.' '!'	'-'	'+'	'*'	'/'	'%'	'<'	'>' 
%token T_COMMENT T_WS

%nonassoc T_LEFT_OP T_RIGHT_OP
%left T_OR_OP
%left T_AND_OP
%nonassoc T_EQ_OP T_NE_OP 
%nonassoc '>' '<' T_GE_OP T_LE_OP
%left '+' '-' T_OBJ_PT_OPERATOR
%left '*' '/' '%'
%nonassoc UNARY
%nonassoc IFX
%left T_ELSE

%start program

%%

program: T_H_INC T_USING T_NS T_STD ';' external_definition_list { printf("yes\n"); }
;

external_definition_list: external_definition
						| external_definition_list external_definition
;


external_definition: function_definition
					| declaration
;

declaration: class_specifier ';'
			| declarator ';'
;

unary_operator: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
;

expr: term
	| expr '+' expr
	| expr '-' expr 
	| expr '*' expr
	| expr '/' expr
	| expr '%' expr
	| expr '>' expr
	| expr '<' expr
	| expr T_EQ_OP expr
	| expr T_NE_OP expr
	| expr T_LE_OP expr
	| expr T_GE_OP expr
	| expr T_OR_OP expr
	| expr T_AND_OP expr
;

term: primary_expr
	| unary_operator term %prec UNARY;

primary_expr: variable
			| T_INT_CONSTANT
			| '(' expr ')'
			| function_call
;

variable: callable_var
		| dereferencable '.' member_name
		| dereferencable T_OBJ_PT_OPERATOR member_name
;

dereferencable: variable
			| '(' expr ')'
;

callable_var: T_IDENTIFIER
			| dereferencable '[' expr ']'
			| dereferencable '.' member_name argument_list
			| dereferencable T_OBJ_PT_OPERATOR member_name argument_list
;

member_name: T_IDENTIFIER
;

argument_list: '(' ')'
			| '(' non_empty_arg_list ')'
;

non_empty_arg_list: non_empty_arg_list ',' argument
			| argument
;

argument: T_STRING_LITERAL
		| expr
;

class_identifier: T_IDENTIFIER 
;

function_identifier: T_IDENTIFIER
;

object_identifier: T_IDENTIFIER
;

simple_type_name: class_identifier { do_type_class(&$1); $$ = $1; }
            |	T_INT
            |   T_BOOL
;

type_specifier: simple_type_name
;

function_declaration: type_specifier function_identifier { do_begin_function_declaration(&$1, &$2); } argument_declaration_list {/*need `;` to be a complete declaration of function*/}
;

argument_declaration_list: '(' ')'
			|	'(' non_empty_arg_decl_list ')'
;

non_empty_arg_decl_list: argument_declaration
			| non_empty_arg_decl_list ',' argument_declaration
;

argument_declaration: simple_type_name object_identifier { do_simple_var_decl(&$1, &$2, 1); }

| simple_type_name object_identifier '[' ']' { do_simple_var_decl(&$1, &$2, 1); do_begin_array_dim(NULL); } array_ops { do_end_array_decl(&$2, 1); }

| simple_type_name object_identifier '[' T_INT_CONSTANT ']' { do_simple_var_decl(&$1, &$2, 1); do_begin_array_dim(&$4); } array_ops { do_end_array_decl(&$2, 1); }
;

function_call: function_identifier argument_list
;

array_op: '[' T_INT_CONSTANT ']' { do_add_array_dim(&$2); }
;

array_ops: array_ops array_op
		|	/*empty*/
;

declarator: type_specifier object_identifier { do_simple_var_decl(&$1, &$2, 0); } array_ops { do_end_array_decl(&$2, 0); }
;

assignment_stmt: variable '=' expr
;

function_definition: function_declaration '{' inner_statement_list '}' { do_end_function_declaration(); }
;

optional_expr:/* empty */	
			| expr
;

inner_statement: declarator ';'
				| stmt
				| T_RETURN optional_expr ';'
;

inner_statement_list: /*empty*/
				| inner_statement_list inner_statement
;

member_declaration: scope_decorator declarator ';' 
				| scope_decorator function_definition
;

scope_decorator: /*empty*/
    | T_PUBLIC ':' { do_scope_declaration(); }
;

member_list: /*empty*/
			| member_list member_declaration
;

class_specifier: T_CLASS class_identifier { do_begin_class_declaration(&$2); } extends_from '{' member_list '}' { do_end_class_declaration(); }
;

extends_from: /*empty*/
    | ':' T_PUBLIC class_identifier {}
;


stmt: compound_stmt
	| assignment_stmt ';'
	| T_CIN T_RIGHT_OP variable ';'
	| T_COUT T_LEFT_OP expr ';'
	| if_stmt
	| T_WHILE '(' expr ')' stmt
;

compound_stmt: '{' stmt_list '}'
;

stmt_list: stmt_list stmt
		| /*empty*/
;


if_stmt_without_else: T_IF '(' expr ')' stmt
;

if_stmt: if_stmt_without_else %prec IFX
		| if_stmt_without_else T_ELSE stmt
;

%%


void yyerror(const char *s, ...) {
    
    va_list ap;
    va_start(ap, s);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    exit(255);
    
}