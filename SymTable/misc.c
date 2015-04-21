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


void display_properties(HashTable *pps) {
    
    Bucket *p;
    property_info *prop;
    
    p = pps->pListHead;
    st_output_debug_string("  Properties:\n");
    while (p != NULL) {
        prop = p->pData;
        st_output_debug_string("\t%s %s: %s;\n", acc_type_str(prop->flags), prop->name, type_str(&prop->type));
        p = p->pListNext;
    }
    
}


void dispaly_classes() {

    HashTable *class_tbl = CG(class_table);
    class_entry *ce = NULL;
    Bucket *p;
    
    st_output_debug_string("Classes:\n\n");
    p = class_tbl->pListHead;
    while (p != NULL) {
        ce = p->pDataPtr;
        st_output_debug_string("class %s {\n", ce->name);
        st_output_debug_string("  Methods:\n");
        display_functions(&ce->function_table);
        display_properties(&ce->properties_info);
        st_output_debug_string("}\n\n");
        p = p->pListNext;
    }

}


void display_variables(HashTable *vars) {

    Bucket *p;
    variable *var;
    
    p = vars->pListHead;
    while (p != NULL) {
        var = p->pData;
        p = p->pListNext;
        if (!p) {
            st_output_debug_string("%s: %s;", var->name, type_str(&var->variable_type));
            break;
        }
        st_output_debug_string("%s : %s, ", var->name, type_str(&var->variable_type));
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
        
        st_output_debug_string(")");
        st_output_debug_string(" { ");
        display_variables(fe->local_vars);
        st_output_debug_string(" }\n\n");
        p = p->pListNext;
    }
        
}


void display_globals() {
    
    HashTable *glb_tbl = CG(global_var_table);
    variable *var;
    Bucket *p;
    
    p = glb_tbl->pListHead;
    
    st_output_debug_string("Global Variables:\n\n");
    while (p != NULL) {
        var = p->pData;
        st_output_debug_string("%s: %s;\n", var->name, type_str(&var->variable_type));
    }
    
    st_output_debug_string("\n\n");

}

void display_nonclass_funcs() {

    HashTable *func_tbl = CG(function_table);
    
    st_output_debug_string("Functions:\n\n");
    display_functions(func_tbl);
    st_output_debug_string("\n\n");
    
}



