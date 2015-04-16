//
//  misc.c
//  SymTable
//
//  Created by QiuChusheng on 4/15/15.
//  Copyright (c) 2015 QiuChusheng. All rights reserved.
//

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "compile.h"
#include "compile_globals.h"

void st_output_debug_string(char *format, ...)
{
    va_list args;
    
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

}


#define BASIC_TYPE_STR(t, r)    \
    switch (t) {   \
        case BT_INT:            \
            r = "int";          \
            break;              \
        case BT_BOOL:           \
            r = "bool";         \
            break;              \
        default:                \
            r = "unknown";      \
    }


void display_functions (HashTable *func_tbl);

char *type_str(var_type *type) {
    
    static char buff[128] = {0};
    char *bt, *pos = buff;
    int n;
    
    switch(type->var_type) {
            
        case IS_BASIC_TYPE:
            BASIC_TYPE_STR(type->type.bt_, bt);
            return bt;
            
        case IS_CLASS_TYPE:
            return type->type.class_->name;

        case IS_ARRAY_TYPE:
            
            if (!type->type.array_t_.class_) {
                BASIC_TYPE_STR(type->type.array_t_.bt_, bt);
                pos += sprintf(buff, bt);
            } else {
                pos += snprintf(buff, type->type.array_t_.class_->name_length, type->type.array_t_.class_->name);
            }
            
            for (n = 0; n < type->type.array_t_.dim; n++) {
                pos += sprintf(pos, "[%d]", type->type.array_t_.dimensions[n]);
            }
            *pos = '\0';
            return buff;
    }

    return "";
}


char *acc_type_str(uint acc_flag) {
    
    switch (acc_flag & CLASS_ACC_PPP) {
            
        case CLASS_ACC_PUBLIC:
            return "public";
            
        case CLASS_ACC_PRIVATE:
            return "private";
            
        case CLASS_ACC_PROTETCTED:
            return "protected";
            
        default:
            break;
    }
    
    return "";
}


void dispaly_classes() {

    HashTable *class_tbl = CG(class_table);
    class_entry *ce = NULL;
    Bucket *p;
    
    p = class_tbl->pListHead;
    while (p != NULL) {
        ce = p->pDataPtr;
        st_output_debug_string("class %s {\n", ce->name);
        
        display_functions(&ce->function_table);
        
        /*function & property output*/
        
        st_output_debug_string("}");
        p = p->pListNext;
    }

}


void display_functions (HashTable *func_tbl) {
    
    Bucket *p;
    function_entry *fe;
    int i;
    
    p = func_tbl->pListHead;
    while (p != NULL) {
        fe = p->pData;
        
        st_output_debug_string("\t%s %s %s (", acc_type_str(fe->fn_flags), type_str(&fe->return_type),
                               fe->function_name);
        
        for (i = 0; i < fe->num_args; i++) {
            if (i == fe->num_args - 1) {
                st_output_debug_string("%s: %s", fe->args_info[i].name, type_str(&fe->args_info[i].arg_type));
                break;
            }
            st_output_debug_string("%s : %s, ", fe->args_info[i].name, type_str(&fe->args_info[i].arg_type));
        }
        
        /*TODO: local variables*/
        
        st_output_debug_string(");\n");
        
        p = p->pListNext;
    }
    
}


void display_variables(HashTable *var_tbl, char *banner) {
    
    

}










