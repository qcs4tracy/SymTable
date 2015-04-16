%option noyywrap nodefault noyylineno
%option outfile="tinycpp_lex.c"
%x SL_COMMENT
     
%{
#include "tinycpp_parser.h"
#include "compile.h"
#include "compile_globals.h"
#include <stdlib.h>
#include <string.h>

#define LOWCASE(c) (c >= 'a' && c <= 'z')
#define UPPERCASE(c) (c >= 'A' && c <= 'Z')
#define ISALPHA(c) (LOWCASE(c) || UPPERCASE(c))

#ifdef TOKEN_ECHO
    #define PRINT_TOKEN(token_t) printf("<%s, %s>\n", #token_t, yytext)
    #define PRINT_KW() PRINT_TOKEN(keyword)
    #define PRINT_PUNCT() PRINT_TOKEN(punctuation)
    #define PRINT_OP() PRINT_TOKEN(operator)
#else
    #define PRINT_TOKEN(token_t)
    #define PRINT_KW()
    #define PRINT_PUNCT()
    #define PRINT_OP()
#endif
    
#define YY_DECL int lex_scan(sval *slval)

#define sval_copy_value(lval, yytext, yyleng) do {         \
        slval->value.str.val = (char *) strndup(yytext, (size_t)yyleng); \
        slval->value.str.len = (int)yyleng;                           \
    } while(0)
    
    
#define HANDLE_NEWLINES(s, l)                                       \
do {                                                                \
    char *p = (s), *boundary = p+(l);                               \
    while (p < boundary) {                                          \
        if (*p == '\n' || (*p == '\r' && (*(p+1) != '\n'))) {       \
            CG(lineno)++;                                           \
        }                                                           \
        p++;                                                        \
    }                                                               \
} while(0)

#define HANDLE_NEWLINE(c) \
{ \
    if (c == '\n' || c == '\r') { \
        CG(lineno)++; \
    } \
}



%}


OCT   [0-7]
DEC   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\n\r]
LABEL [[:alpha:]]([[:alnum:]]|(_[[:alnum:]]))*

%%

"#include <iostream>" { PRINT_KW(); return T_H_INC;}
"using" { PRINT_KW(); return T_USING;}
"namespace" { PRINT_KW(); return T_NS;}
"std" { PRINT_KW(); return T_STD;}
"class" { PRINT_KW(); return T_CLASS; }
"int" { PRINT_KW(); return T_INT; }
"bool" { PRINT_KW(); return T_BOOL;}
"return" { PRINT_KW(); return T_RETURN; }
"public" { PRINT_KW(); return T_PUBLIC; }
"cin" { PRINT_KW(); return T_CIN; }
"cout" { PRINT_KW(); return T_COUT; }
"if" { PRINT_KW(); return T_IF; }
"else" { return T_ELSE;}
"while" { PRINT_KW(); return T_WHILE; }

{HP}{H}+{IS}?				{ /*hexicals*/ PRINT_TOKEN(integer); return T_INT_CONSTANT; }

{NZ}{DEC}*{IS}?	{
    /*decimals*/
    PRINT_TOKEN(integer);
    slval->value.lval = atol(yytext);
    return T_INT_CONSTANT;
}

"0"{OCT}*{IS}?				{ /*octals*/ PRINT_TOKEN(integer); return T_INT_CONSTANT; }


{LABEL} {
    char next_c;
    size_t len = 0;
    static char buff[512];
    next_c = input();				
    /*this is an invalid identifier*/
    if (next_c == '_' ) {
        strcpy(buff, yytext);
        len = yyleng;
        buff[len++] = next_c;
        next_c = input();
        while (ISALPHA(next_c) || next_c == '_' ) {
            buff[len++] = next_c;
            next_c = input();
        }
        buff[len] = '\0';
        
    } else {
        PRINT_TOKEN(identifier);
        sval_copy_value(slval, yytext, yyleng);
        slval->type = IS_STRING;
    }
    unput(next_c);
    return T_IDENTIFIER;
}


({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{
    HANDLE_NEWLINES(yytext, yyleng);
    sval_copy_value(slval, yytext, yyleng);
    slval->type = IS_STRING;
    PRINT_TOKEN(string-literal);
    return T_STRING_LITERAL;
}

">>" 				{ PRINT_OP(); return T_RIGHT_OP;}
"<<" 				{ PRINT_OP(); return T_LEFT_OP; }
"&&"				{ PRINT_OP(); return T_AND_OP; }
"||"				{ PRINT_OP(); return T_OR_OP; }
"->"				{ PRINT_OP(); return T_OBJ_PT_OPERATOR; }
"<="				{ PRINT_OP(); return T_LE_OP; }
">="				{ PRINT_OP(); return T_GE_OP; }
"=="				{ PRINT_OP(); return T_EQ_OP; }
"!="				{ PRINT_OP(); return T_NE_OP; }
"//"         		{ BEGIN(SL_COMMENT); yymore();}

";"					{ PRINT_PUNCT(); return ';'; }
"{"					{ PRINT_PUNCT(); return '{'; }
"}"					{ PRINT_PUNCT(); return '}'; }
","					{ PRINT_PUNCT(); return ','; }
":"					{ PRINT_PUNCT(); return ':'; }
"="					{ PRINT_OP(); return '='; }
"("					{ PRINT_OP(); return '('; }
")"					{ PRINT_OP(); return ')'; }
"["					{ PRINT_OP(); return '['; }
"]"					{ PRINT_OP(); return ']'; }
"."					{ PRINT_PUNCT(); return '.'; }
"!"					{ PRINT_OP(); return '!'; }
"-"					{ PRINT_OP(); return '-'; }
"+"					{ PRINT_OP(); return '+'; }
"*"					{ PRINT_OP(); return '*'; }
"/"					{ PRINT_OP(); return '/'; }
"%"					{ PRINT_OP(); return '%'; }
"<"					{ PRINT_OP(); return '<'; }
">"					{ PRINT_OP(); return '>'; }

<SL_COMMENT>[^\n]*        {/*eat up the comment*/ yymore();}
<SL_COMMENT>\n             { CG(lineno)++; BEGIN(INITIAL);  return T_COMMENT;}

{WS} {
    HANDLE_NEWLINES(yytext, yyleng);
    return T_WS;
}

. {
    printf("unrecognized token: %s\n", yytext);
    exit(255);
}

%%
