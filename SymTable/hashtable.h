//
//  hashtable.h
//  SymTable
//
//  Created by QiuChusheng on 15/4/8.
//  Copyright (c) 2015年 QiuChusheng. All rights reserved.
//
#ifndef __SymTable__hashtable__
#define __SymTable__hashtable__

#include "st_types.h"


#define HASH_KEY_IS_STRING 1
#define HASH_KEY_IS_LONG 2
#define HASH_KEY_NON_EXISTANT 3

#define HASH_UPDATE 		(1<<0)
#define HASH_ADD			(1<<1)
#define HASH_NEXT_INSERT	(1<<2)

#define HASH_DEL_KEY 0
#define HASH_DEL_INDEX 1

typedef ulong (*hash_func_t)(char *arKey, uint nKeyLength);
typedef void (*dtor_func_t)(void *pDest);

struct _hashtable;

typedef struct bucket {
    ulong h;						/* Used for numeric indexing */
    uint nKeyLength;
    void *pData;
    void *pDataPtr;
    struct bucket *pListNext;
    struct bucket *pListLast;
    struct bucket *pNext;
    struct bucket *pLast;
    char arKey[1]; /* Must be last element: struct hack*/
} Bucket;

typedef struct _hashtable {
    uint nTableSize;
    uint nTableMask;
    uint nNumOfElements;
    ulong nNextFreeElement;
    Bucket *pInternalPointer;	/* Used for element traversal */
    Bucket *pListHead;
    Bucket *pListTail;
    Bucket **arBuckets;
    dtor_func_t pDestructor;
    st_bool persistent;
    unsigned char nApplyCount;
    st_bool bApplyProtection;
} HashTable;


typedef struct _st_hash_key {
    char *arKey;
    uint nKeyLength;
    ulong h;
} st_hash_key;

int _st_hash_init(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor);


static inline ulong st_inline_hash_func(char *arKey, uint nKeyLength)
{
    register ulong hash = 5381;
    
    /* variant with the hash unrolled eight times */
    for (; nKeyLength >= 8; nKeyLength -= 8) {
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
    }
    switch (nKeyLength) {
        case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 1: hash = ((hash << 5) + hash) + *arKey++; break;
        case 0: break;
        default: break;
    }
    return hash;
}


/* startup/shutdown */
int _st_hash_init(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor);
void st_hash_destroy(HashTable *ht);
void st_hash_clean(HashTable *ht);

#define st_hash_init(ht, nSize, pHashFunction, pDestructor) \
            _st_hash_init((ht), (nSize), (pHashFunction), (pDestructor))


/* additions/updates/changes */
int _st_hash_add_or_update(HashTable *ht, char *arKey, uint nKeyLength, void *pData, uint nDataSize, void **pDest, int flag);

#define st_hash_update(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
    _st_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_UPDATE)

#define st_hash_add(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
    _st_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_ADD)


/* deletion operations*/
int st_hash_del_key_or_index(HashTable *ht, char *arKey, uint nKeyLength, ulong h, int flag);

#define st_hash_del(ht, arKey, nKeyLength) \
    st_hash_del_key_or_index(ht, arKey, nKeyLength, 0, HASH_DEL_KEY)

#define st_hash_index_del(ht, h) \
    st_hash_del_key_or_index(ht, NULL, 0, h, HASH_DEL_INDEX)


ulong st_get_hash_value(char *arKey, uint nKeyLength);


/* Data retreival */
int st_hash_find(HashTable *ht, char *arKey, uint nKeyLength, void **pData);
int st_hash_index_find(HashTable *ht, ulong h, void **pData);

/* Misc */
int st_hash_exists(HashTable *ht, char *arKey, uint nKeyLength);
int st_hash_index_exists(HashTable *ht, ulong h);


static inline int st_symtable_update(HashTable *ht, char *arKey, uint nKeyLength, void *pData, uint nDataSize, void **pDest)					\
{
    return st_hash_update(ht, arKey, nKeyLength, pData, nDataSize, pDest);
}


static inline int st_symtable_del(HashTable *ht, char *arKey, uint nKeyLength)
{
    return st_hash_del(ht, arKey, nKeyLength);
}


static inline int st_symtable_find(HashTable *ht, char *arKey, uint nKeyLength, void **pData)
{
    return st_hash_find(ht, arKey, nKeyLength, pData);
}


static inline int st_symtable_exists(HashTable *ht, char *arKey, uint nKeyLength)
{
    return st_hash_exists(ht, arKey, nKeyLength);
}

#endif /* defined(__SymTable__hashtable__) */
