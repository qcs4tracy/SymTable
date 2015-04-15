//
//  compile.h
//  SymTable
//
//  Created by QiuChusheng on 4/14/15.
//  Copyright (c) 2015 QiuChusheng. All rights reserved.
//

#ifndef __SymTable__compile__
#define __SymTable__compile__

#include "st_types.h"
#include <stdio.h>

typedef union _svalue {
    long lval;                 /* long value */
    double dval;               /* double value */
    struct {
        char *val;
        int len;               /* this will always be set for strings */
    } str;                     /* string (always has length) */
} svalue;


#define IS_LONG 0x01
#define IS_DOUBLE 0x02
#define IS_STRING 0x04

#define IS_CONST    (1<<0)
#define IS_VAR      (1<<1)
#define IS_TMP_VAR  (1<<2)
#define IS_UNUSED   (1<<3)

typedef struct _sval_struct {
    u_char type;
    svalue value;
} sval;

typedef struct _snode {
    int op_type;
    union {
        sval constant;
        uint var;
        int opline_num; /*  Needs to be signed */
        //function *op_array;
        //_op *jmp_addr;
        struct {
            sval constant; /* placeholder for sval */
            uint var;	/* dummy */
            uint type;
        } EA;
    } u;
} snode;

void do_begin_class_declaration(snode *class_name);
void do_end_class_declaration();
void do_scope_declaration();
void do_type_class(snode *type);
void do_begin_function_declaration(snode *return_type, snode *function_name);
void do_end_function_declaration();
void do_simple_var_decl(snode *type, snode *var_name, int is_arg);
void do_add_array_dim(snode *dim);
void do_begin_array_dim(snode *first_dim);
void do_end_array_decl(snode *var_name, int is_arg);


/*should declare somewhere, flex can not generate header file correctly*/
void yyrestart (FILE *input_file);
int lex_scan(sval *slval);

/*wrapper for yylex*/
int tc_lex(snode *snode);
int yyparse();


#endif /* defined(__SymTable__compile__) */
