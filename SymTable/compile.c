//
//  compile.c
//  SymTable
//
//  Created by QiuChusheng on 4/14/15.
//  Copyright (c) 2015 QiuChusheng. All rights reserved.
//

#include "compile_globals.h"
#include "compile.h"
#include "tinycpp_parser.h"
#include <string.h>
#include <stdlib.h>

#define DEFAULT_TABLE_SIZE 64
#define USER_DEFINE_CLASS 0x01



static void initialize_class_entry_data(class_entry *ce) {
    
    st_hash_init(&ce->function_table, DEFAULT_TABLE_SIZE, NULL, NULL);
    st_hash_init(&ce->properties_info, DEFAULT_TABLE_SIZE, NULL, NULL);
    
    ce->parent = NULL;
    ce->type = USER_DEFINE_CLASS;
    ce->ce_flags = 0;
}

//the begining of class definition
void do_begin_class_declaration(snode *class_name) {
    
    char *name = class_name->u.constant.value.str.val;
    int len = class_name->u.constant.value.str.len;
    class_entry *new_ce;
    
    if (st_symtable_find(CG(class_table), name, len, NULL) == SUCCESS) {
        /*error duplicate declarations of class*/
    }
    
    new_ce = malloc(sizeof(class_entry));
    initialize_class_entry_data(new_ce);

    new_ce->line_start = CG(lineno);
    new_ce->name = name;
    new_ce->name_length = len;
    
    //enter the class scope, default is private scope before `public` keyword is seen
    if (CG(active_acc_flag) & CLASS_ACC_NONE) {
        CG(active_acc_flag) = CLASS_ACC_PRIVATE;
    }
    
    st_hash_update(CG(class_table), name, len, &new_ce, sizeof(class_entry *), NULL);
    CG(active_class) = new_ce;
    
}

/*called when the end of class definition is reached*/
void do_end_class_declaration() {
    
    //set end line
    class_entry *ce = CG(active_class);
    ce->line_end = CG(lineno);
    
    //out of class definition scope
    CG(active_class) = NULL;
    CG(active_acc_flag) = CLASS_ACC_NONE;
    
}


void do_scope_declaration() {
    //Tiny CPP only has keyword `public`
    CG(active_acc_flag) = CLASS_ACC_PUBLIC;
}

void do_type_class(snode *type) {
    type->u.EA.type = TYPE_CLASS;
}


void do_begin_function_declaration(snode *return_type, snode *function_name) {
    
    function_entry fe;
    char *name = function_name->u.constant.value.str.val;
    int name_len = function_name->u.constant.value.str.len;
    class_entry *ce = NULL;
    int is_method = CG(active_class)? 1: 0;
    HashTable *target_tbl = is_method? &CG(active_class)->function_table: CG(function_table);
    
    if (return_type->u.EA.type & TYPE_CLASS) {
        
        char *class_type = NULL;
        int len;
        
        class_type = return_type->u.constant.value.str.val;
        len = return_type->u.constant.value.str.len;
        
        if (st_hash_find(CG(class_table), class_type, len, (void **)&ce) == FAILURE) {
            //unknown class type
        }
        
        fe.return_type.var_type = IS_CLASS_TYPE;
        fe.return_type.type.class_ = ce;
        
    } else if (return_type->u.EA.type & TYPE_INT){
        fe.return_type.var_type = IS_BASIC_TYPE;
        fe.return_type.type.bt_ = BT_INT;
    }
    
    if (is_method) {
        fe.fn_flags |= CG(active_acc_flag);
    }
    
    fe.function_name = name;
    fe.args_info = NULL;
    fe.num_args = 0;
    fe.scope = is_method? CG(active_class): NULL;
    fe.line_start = CG(lineno);
    
    //initialize hash table for local variables
    fe.local_vars = malloc(sizeof(HashTable));
    st_hash_init(fe.local_vars, DEFAULT_TABLE_SIZE, NULL, NULL);
    
    if (st_hash_add(target_tbl, name, name_len, &fe, sizeof(fe), (void **)&CG(active_function)) == FAILURE) {
        //redefine function
    }

}

void do_begin_array_dim(snode *first_dim) {
    
    int dim;
    
    CG(dim) = 1;
    if (!first_dim) {
        CG(dimensions)[0] = 0;
        return;
    }
    
    dim = (int)first_dim->u.constant.value.lval;
    CG(dimensions)[0] = dim;
    
}


void do_add_array_dim(snode *dim) {
    
    CG(dimensions)[CG(dim)] = (int)dim->u.constant.value.lval;
    CG(dim)++;
}


void do_simple_var_decl(snode *type, snode *var_name, int is_arg) {
    
    class_entry *ce = NULL, **ce_ptr = NULL;
    int is_class_type = type->u.EA.type & TYPE_CLASS;
    basic_type bt = BT_UNKNOWN;
    
    if (is_class_type) {
        
        char *class_type = NULL;
        int class_len;
        
        class_type = type->u.constant.value.str.val;
        class_len = type->u.constant.value.str.len;
        
        if (st_hash_find(CG(class_table), class_type, class_len, (void **)&ce_ptr) == SUCCESS) {
            ce = *ce_ptr;
        } else {
            //undeclared class type
        }
        
    } else if (type->u.EA.type & TYPE_INT){
        bt = BT_INT;
    }
    
    if (is_arg) {
    
        function_entry *fe = CG(active_function);
        arg_info *arg;
        
        fe->num_args++;
        fe->args_info = realloc(fe->args_info, fe->num_args * sizeof(arg_info));
        arg = &fe->args_info[fe->num_args - 1];
        arg->name = var_name->u.constant.value.str.val;
        arg->name_len = var_name->u.constant.value.str.len;
        arg->arg_index = fe->num_args;
        arg->hash_value = st_get_hash_value(arg->name, arg->name_len);
        
        if (is_class_type) {
            arg->arg_type.var_type = IS_CLASS_TYPE;
            arg->arg_type.type.class_ = ce;
        } else {
            arg->arg_type.var_type = IS_BASIC_TYPE;
            arg->arg_type.type.bt_ = bt;
        }
       
    } else {
        
        if (CG(active_class) && !CG(active_function)) {//class property reached
            property_info pp;
            pp.name = var_name->u.constant.value.str.val;
            pp.name_len = var_name->u.constant.value.str.len;
            pp.h = st_get_hash_value(pp.name, pp.name_len);
            pp.flags |= CG(active_acc_flag);
            pp.ce = CG(active_class);
            if (is_class_type) {
                pp.type.var_type = IS_CLASS_TYPE;
                pp.type.type.class_ = ce;
            } else {
                pp.type.var_type = IS_BASIC_TYPE;
                pp.type.type.bt_ = bt;
            }
            
            st_hash_update(&CG(active_class)->properties_info, pp.name, pp.name_len, &pp, sizeof(pp), NULL);
            return;
        }
    
        variable var;
        var.name = var_name->u.constant.value.str.val;
        var.name_len = var_name->u.constant.value.str.len;
        var.hash_value = st_get_hash_value(var.name, var.name_len);
        
        if (is_class_type) {
            var.variable_type.var_type = IS_CLASS_TYPE;
            var.variable_type.type.class_ = ce;
        } else {
            var.variable_type.var_type = IS_BASIC_TYPE;
            var.variable_type.type.bt_ = bt;
        }
        
        if (CG(active_function)) {//function local variable
            st_hash_update(CG(active_function)->local_vars, var.name, var.name_len, &var, sizeof(var), NULL);
        } else {//global variable
            st_hash_update(CG(global_var_table), var.name, var.name_len, &var, sizeof(var), NULL);
        }
    
    }
    
}


void do_end_array_decl(snode *var_name, int is_arg) {

    char *name = var_name->u.constant.value.str.val;
    int len = var_name->u.constant.value.str.len;
    int i;
    var_type *vt;
    
    if (is_arg) {
        arg_info *arg = &CG(active_function)->args_info[CG(active_function)->num_args - 1];
        vt = &arg->arg_type;
    } else {
        
        if (CG(active_class) && !CG(active_function)) {//class property reached
            property_info *pp;
            
            st_hash_find(&CG(active_class)->properties_info, name, len, (void **)&pp);
            vt = &pp->type;
            
        } else {
            variable *var;
            
            if (CG(active_function)) {//function local variable
                st_hash_find(CG(active_function)->local_vars, name, len, (void **)&var);
            } else {//global variable
                st_hash_find(CG(global_var_table), name, len, (void **)&var);
            }
            
            vt = &var->variable_type;
        }
    }
    
    vt->var_type = IS_ARRAY_TYPE;
    vt->type.array_t_.dim = CG(dim);
    for (i = 0; i < CG(dim); i++) {
        vt->type.array_t_.dimensions[i] = CG(dimensions)[i];
    }
    
    CG(dim) = 0;
}


void do_end_function_declaration() {
    
    CG(active_function)->line_end = CG(lineno);
    CG(active_function) = NULL;

}


#define T_EOF 0
int tc_lex(snode *snode_) {
    int retval;

again:
    snode_->u.constant.type = IS_LONG;
    retval = lex_scan(&snode_->u.constant);
    switch (retval) {
        case T_COMMENT:
        case T_WS:
            goto again;
        
        case T_INT:
            snode_->u.EA.type = TYPE_INT;
            break;

        case T_EOF:
            return T_EOF;
    }
    
    snode_->op_type = IS_CONST;
    return retval;
}






