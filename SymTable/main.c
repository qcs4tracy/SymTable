//
//  main.c
//  SymTable
//
//  Created by QiuChusheng on 15/4/8.
//  Copyright (c) 2015年 QiuChusheng. All rights reserved.
//

#include "compile.h"
#include "compile_globals.h"

int main(int argc, const char * argv[]) {
    
    int i;
    init_compile_globals();
    
    if (argc < 2) {
        /*read from stdin*/
        yyparse();
        return 0;
    }
    
    for (i = 1; i < argc; ++i)
    {
        FILE *f = fopen(argv[i], "r");
        if(!f) {
            perror(argv[i]);
            continue;
        }
        
        yyrestart(f);
        
        if (yyparse() == 0) {
            printf("Successfully parsed.\n");
        }
        fclose(f);
        
    }
    
    return 0;
}
