//
//  compile_globals.c
//  SymTable
//
//  Created by QiuChusheng on 4/14/15.
//  Copyright (c) 2015 QiuChusheng. All rights reserved.
//

#include "compile_globals.h"
#include <stdlib.h>
#include <string.h>

compile_globals_t compile_globals;
#define GLOBAL_TABLE_SIZE 256

void init_compile_globals() {
    
    memset(&compile_globals, 0, sizeof(compile_globals_t));
    
    //allocate memory for hash tables: class, function and global variables
    compile_globals.class_table = (HashTable *) malloc(sizeof(HashTable));
    compile_globals.function_table = (HashTable *) malloc(sizeof(HashTable));
    compile_globals.global_var_table = (HashTable *) malloc(sizeof(HashTable));
    
    //initialize hash tables
    st_hash_init(CG(class_table), GLOBAL_TABLE_SIZE, NULL, NULL);
    st_hash_init(CG(function_table), GLOBAL_TABLE_SIZE, NULL, NULL);
    st_hash_init(CG(global_var_table), GLOBAL_TABLE_SIZE, NULL, NULL);
    
    compile_globals.lineno = 0;
    compile_globals.filename = NULL;
    compile_globals.active_class = NULL;
    compile_globals.active_function = NULL;
    compile_globals.active_acc_flag = CLASS_ACC_NONE;
    
}


void shutdown_compile() {
    
    /*
     destroy the hashtables and free the memory
     */
    
    //st_hash_destroy(CG(class_table));
    
    free((void *)CG(class_table));
    free((void *)CG(function_table));
    free((void *)CG(global_var_table));

}









