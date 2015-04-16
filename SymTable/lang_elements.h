//
//  lang_elements.h
//  SymTable
//
//  Created by QiuChusheng on 4/14/15.
//  Copyright (c) 2015 QiuChusheng. All rights reserved.
//

#ifndef __SymTable__lang_elements__
#define __SymTable__lang_elements__

#include "st_types.h"
#include "hashtable.h"


#define CLASS_ACC_PRIVATE 0x1
#define CLASS_ACC_PROTETCTED 0x02
#define CLASS_ACC_PUBLIC 0x04
#define CLASS_ACC_NONE 0
#define CLASS_ACC_PPP (CLASS_ACC_PRIVATE | CLASS_ACC_PROTETCTED | CLASS_ACC_PUBLIC)

#define TYPE_INT 0x01
#define TYPE_BOOL 0x02
#define TYPE_CLASS 0x10
#define BASIC_TYPE_MASK (TYPE_INT | TYPE_BOOL)

//class entry
typedef struct _class_entry {
    
    u_char type;
    char *name;
    uint name_length;
    struct _class_entry *parent;
    uint ce_flags;
    
    HashTable function_table;
    HashTable properties_info;
    
    char *file_name;
    uint line_start;
    uint line_end;
    
} class_entry;


//basic types
typedef enum _basic_type {
    BT_UNKNOWN,
    BT_INT,
    BT_BOOL,
} basic_type;

#define IS_CLASS_TYPE 0
#define IS_BASIC_TYPE 1
#define IS_ARRAY_TYPE 2
#define DIM_SIZE 8

//type for varibale
typedef struct _var_type {
    
    u_char var_type;
    union {
        basic_type bt_;
        class_entry *class_;
        struct {
            basic_type bt_;
            class_entry *class_;
            int dim;
            int dimensions[DIM_SIZE];
        } array_t_;
    } type;
    
} var_type;


//varibale with name and type
typedef struct _variable {
    
    char *name;
    int name_len;
    ulong hash_value;
    var_type variable_type;
    
} variable;


//argument info is just like variable
typedef struct _arg_info {
    
    char *name;
    uint name_len;
    ulong hash_value;
    uint arg_index;
    var_type arg_type;
    
} arg_info;


//property info
typedef struct _property_info {
    uint flags;
    char *name;
    uint name_len;
    ulong h;
    class_entry *ce;
    var_type type;
} property_info;


//function representation
typedef struct _function_entry {
    
    u_char type;            //function type
    char *function_name;    //function name
    uint fn_flags;          //access flags
    class_entry *scope;     // is it a method for a class
    
    uint num_args;          // # of arguments to this function
    arg_info *args_info;    //arguments information array
    
    HashTable *local_vars;    //local variables
    
    var_type return_type;
    
    /*
     opcode *opcodes;
     uint last, size;
     */
    
    int backpatch_count;
    
    char *filename;
    uint line_start;
    uint line_end;
    
} function_entry;

#endif /* defined(__SymTable__lang_elements__) */









