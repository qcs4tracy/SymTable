//
//  compile_globals.h
//  SymTable
//
//  Created by QiuChusheng on 4/14/15.
//  Copyright (c) 2015 QiuChusheng. All rights reserved.
//

#ifndef __SymTable__compile_globals__
#define __SymTable__compile_globals__

#include "hashtable.h"
#include "lang_elements.h"


typedef struct _compile_globals {
    
    /*backpatch stack*/
    
    HashTable *class_table;
    HashTable *function_table;
    HashTable *global_var_table;
    
    class_entry *active_class;
    function_entry *active_function;
    uint active_acc_flag;
    
    char *filename;
    uint lineno;
    
    /*array dimension*/
    int dim;
    int dimensions[DIM_SIZE];
    
} compile_globals_t;

extern compile_globals_t compile_globals;

#define CG(v) compile_globals.v

void init_compile_globals();
//void parse_error(char *fmt, ...);


#endif /* defined(__SymTable__compile_globals__) */
