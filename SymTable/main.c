//
//  main.c
//  SymTable
//
//  Created by QiuChusheng on 15/4/8.
//  Copyright (c) 2015年 QiuChusheng. All rights reserved.
//

#include "hashtable.h"
#include <stdio.h>

int main(int argc, const char * argv[]) {
    
    HashTable ht;
    int i = 0;
    st_hash_init(&ht, 256, st_inline_hash_func, NULL);
    st_hash_add(&ht, "hash", sizeof("hash"), &i, sizeof(void *), NULL);
    
    return 0;
}
