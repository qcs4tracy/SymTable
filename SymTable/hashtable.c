//
//  hashtable.c
//  SymTable
//
//  Created by QiuChusheng on 15/4/8.
//  Copyright (c) 2015 QiuChusheng. All rights reserved.
//
#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

#define CONNECT_TO_BUCKET_DLLIST(element, list_head)		\
    (element)->pNext = (list_head);							\
    (element)->pLast = NULL;								\
    if ((element)->pNext) {									\
        (element)->pNext->pLast = (element);				\
    }

#define CONNECT_TO_GLOBAL_DLLIST(element, ht)				\
    (element)->pListLast = (ht)->pListTail;					\
    (ht)->pListTail = (element);							\
    (element)->pListNext = NULL;							\
    if ((element)->pListLast != NULL) {						\
        (element)->pListLast->pListNext = (element);		\
    }														\
    if (!(ht)->pListHead) {									\
        (ht)->pListHead = (element);						\
    }														\
    if ((ht)->pInternalPointer == NULL) {					\
        (ht)->pInternalPointer = (element);					\
    }




#define UPDATE_DATA(ht, p, pData, nDataSize)											\
    if (nDataSize == sizeof(void*)) {													\
        if ((p)->pData != &(p)->pDataPtr) {												\
            free((p)->pData);									\
        }																				\
        memcpy(&(p)->pDataPtr, pData, sizeof(void *));									\
        (p)->pData = &(p)->pDataPtr;													\
    } else {																			\
        if ((p)->pData == &(p)->pDataPtr) {												\
            (p)->pData = (void *) malloc(nDataSize);                                    \
            (p)->pDataPtr = NULL;														\
        } else {                                                                        \
            (p)->pData = (void *) realloc((p)->pData, nDataSize);\
            /* (p)->pDataPtr is already NULL so no need to initialize it */             \
        }																				\
        memcpy((p)->pData, pData, nDataSize);											\
    }

#define INIT_DATA(ht, p, pData, nDataSize);								\
    if (nDataSize == sizeof(void*)) {									\
        memcpy(&(p)->pDataPtr, pData, sizeof(void *));					\
        (p)->pData = &(p)->pDataPtr;									\
    } else {															\
        (p)->pData = (void *) malloc(nDataSize);\
        if (!(p)->pData) {												\
            return FAILURE;												\
        }																\
        memcpy((p)->pData, pData, nDataSize);							\
        (p)->pDataPtr=NULL;												\
    }


#define ST_HASH_IF_FULL_DO_RESIZE(ht)				\
    if ((ht)->nNumOfElements > (ht)->nTableSize) {	\
        st_hash_do_resize(ht);					\
    }


static int st_hash_do_resize(HashTable *ht);

/*
 * the hash function
 */
ulong st_hash_func(char *arKey, uint nKeyLength)
{
    return st_inline_hash_func(arKey, nKeyLength);
}


/*
 * rehash the hash table.
 */
int st_hash_rehash(HashTable *ht)
{
    Bucket *p;
    uint nIndex;
    
    memset(ht->arBuckets, 0, ht->nTableSize * sizeof(Bucket *));
    p = ht->pListHead;
    while (p != NULL) {
        nIndex = p->h & ht->nTableMask;
        CONNECT_TO_BUCKET_DLLIST(p, ht->arBuckets[nIndex]);
        ht->arBuckets[nIndex] = p;
        p = p->pListNext;
    }
    return SUCCESS;
}


/*
 * rehash function
 */
static int st_hash_do_resize(HashTable *ht)
{
    Bucket **t;
    
    if ((ht->nTableSize << 1) > 0) {	/* Let's double the table size */
        t = (Bucket **) realloc(ht->arBuckets, (ht->nTableSize << 1) * sizeof(Bucket *));
        if (t) {
            ht->arBuckets = t;
            ht->nTableSize = (ht->nTableSize << 1);
            ht->nTableMask = ht->nTableSize - 1;
            st_hash_rehash(ht);
            return SUCCESS;
        }
        return FAILURE;
    }
    return SUCCESS;
}


/*
 * initialization of hash table.
 */
int _st_hash_init(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor) {

    uint i = 3;
    Bucket **tmp;

    if (nSize >= 0x80000000) {
        /* prevent overflow */
        ht->nTableSize = 0x80000000;
    } else {
        while ((1U << i) < nSize) {
            i++;
        }
        ht->nTableSize = 1 << i;
    }

    ht->nTableMask = ht->nTableSize - 1;
    ht->pDestructor = pDestructor;
    ht->arBuckets = NULL;
    ht->pListHead = NULL;
    ht->pListTail = NULL;
    ht->nNumOfElements = 0;
    ht->nNextFreeElement = 0;
    ht->pInternalPointer = NULL;
    //ht->nApplyCount = 0;
    //ht->bApplyProtection = 1;
    
    tmp = (Bucket **) calloc(ht->nTableSize, sizeof(Bucket *));
    
    if (!tmp) {
        return FAILURE;
    }
    
    ht->arBuckets = tmp;
    return SUCCESS;
}



int _st_hash_add_or_update(HashTable *ht, char *arKey, uint nKeyLength, void *pData, uint nDataSize, void **pDest, int flag)
{
    ulong h;
    uint nIndex;
    Bucket *p;
    
    if (nKeyLength <= 0) {
        /*empty key*/
        return FAILURE;
    }
    
    h = st_inline_hash_func(arKey, nKeyLength);
    nIndex = h & ht->nTableMask;
    
    p = ht->arBuckets[nIndex];
    while (p != NULL) {
        if ((p->h == h) && (p->nKeyLength == nKeyLength)) {
            if (!memcmp(p->arKey, arKey, nKeyLength)) {
                if (flag & HASH_ADD) {//the key must not exist if HASH_ADD flag set
                    return FAILURE;
                }
                
                if (ht->pDestructor) {
                    ht->pDestructor(p->pData);
                }
                
                UPDATE_DATA(ht, p, pData, nDataSize);
                
                if (pDest) {
                    *pDest = p->pData;
                }
                
                return SUCCESS;
            }
        }
        p = p->pNext;
    }
    
    p = (Bucket *) malloc(sizeof(Bucket) - 1 + nKeyLength);
    if (!p) {
        return FAILURE;
    }
    memcpy(p->arKey, arKey, nKeyLength);
    p->nKeyLength = nKeyLength;
    INIT_DATA(ht, p, pData, nDataSize);
    p->h = h;
    CONNECT_TO_BUCKET_DLLIST(p, ht->arBuckets[nIndex]);
    if (pDest) {
        *pDest = p->pData;
    }
    
    CONNECT_TO_GLOBAL_DLLIST(p, ht);
    ht->arBuckets[nIndex] = p;
    
    ht->nNumOfElements++;
    ST_HASH_IF_FULL_DO_RESIZE(ht);		/* If the Hash table is full, resize it */
    return SUCCESS;
}


/*
 * destroy the hash table
 */

void st_hash_destroy(HashTable *ht)
{
    Bucket *p, *q;
    
    p = ht->pListHead;
    while (p != NULL) {
        q = p;
        p = p->pListNext;
        if (ht->pDestructor) {
            ht->pDestructor(q->pData);
        }
        if (q->pData != &q->pDataPtr) {
            //free the data
            free(q->pData);
        }
        //free the Bucket
        free(q);
    }
    
    //free the buckets
    free(ht->arBuckets);
    
}


/*
 * clean the hash table but not free the buckets array
 */
void st_hash_clean(HashTable *ht)
{
    Bucket *p, *q;
    
    p = ht->pListHead;
    while (p != NULL) {
        q = p;
        p = p->pListNext;
        if (ht->pDestructor) {
            ht->pDestructor(q->pData);
        }
        if (q->pData != &q->pDataPtr) {
            free(q->pData);
        }
        free(q);
    }
    memset(ht->arBuckets, 0, ht->nTableSize*sizeof(Bucket *));
    ht->pListHead = NULL;
    ht->pListTail = NULL;
    ht->nNumOfElements = 0;
    ht->nNextFreeElement = 0;
    ht->pInternalPointer = NULL;
    
}


/* Get the hash value of given key
 */
ulong st_get_hash_value(char *arKey, uint nKeyLength)
{
    return st_inline_hash_func(arKey, nKeyLength);
}



/* Returns SUCCESS if found and FAILURE if not. The pointer to the
 * data is returned in pData. The reason is that there's no reason
 * someone using the hash table might not want to have NULL data
 */
int st_hash_find(HashTable *ht, char *arKey, uint nKeyLength, void **pData)
{
    ulong h;
    uint nIndex;
    Bucket *p;
    
    h = st_inline_hash_func(arKey, nKeyLength);
    nIndex = h & ht->nTableMask;
    
    p = ht->arBuckets[nIndex];
    while (p != NULL) {
        if ((p->h == h) && (p->nKeyLength == nKeyLength)) {
            if (!memcmp(p->arKey, arKey, nKeyLength)) {
                *pData = p->pData;
                return SUCCESS;
            }
        }
        p = p->pNext;
    }
    return FAILURE;
}


/*
 * the numeric key find
 */
int st_hash_index_find(HashTable *ht, ulong h, void **pData)
{
    uint nIndex;
    Bucket *p;
    
    nIndex = h & ht->nTableMask;
    
    p = ht->arBuckets[nIndex];
    while (p != NULL) {
        if ((p->h == h) && (p->nKeyLength == 0)) {
            *pData = p->pData;
            return SUCCESS;
        }
        p = p->pNext;
    }
    return FAILURE;
}



/*
 * check if given numeric key exists or not
 */
int st_hash_index_exists(HashTable *ht, ulong h)
{
    uint nIndex;
    Bucket *p;
    
    nIndex = h & ht->nTableMask;
    
    p = ht->arBuckets[nIndex];
    while (p != NULL) {
        if ((p->h == h) && (p->nKeyLength == 0)) {
            return SUCCESS;
        }
        p = p->pNext;
    }
    return FAILURE;
}



/*
 * check if a given key exist
 */
int st_hash_exists(HashTable *ht, char *arKey, uint nKeyLength)
{
    ulong h;
    uint nIndex;
    Bucket *p;
    
    h = st_inline_hash_func(arKey, nKeyLength);
    nIndex = h & ht->nTableMask;
    
    p = ht->arBuckets[nIndex];
    while (p != NULL) {
        if ((p->h == h) && (p->nKeyLength == nKeyLength)) {
            if (!memcmp(p->arKey, arKey, nKeyLength)) {
                return SUCCESS;
            }
        }
        p = p->pNext;
    }
    return FAILURE;
}


/*
 * deletion of element given key
 */
int st_hash_del_key_or_index(HashTable *ht, char *arKey, uint nKeyLength, ulong h, int flag)
{
    uint nIndex;
    Bucket *p;
    
    if (flag == HASH_DEL_KEY) {
        h = st_inline_hash_func(arKey, nKeyLength);
    }
    nIndex = h & ht->nTableMask;
    
    p = ht->arBuckets[nIndex];
    while (p != NULL) {
        if ((p->h == h)
            && (p->nKeyLength == nKeyLength)
            && ((p->nKeyLength == 0) /* Numeric index (short circuits the memcmp() check) */
                || !memcmp(p->arKey, arKey, nKeyLength))) { /* String index */

                if (p == ht->arBuckets[nIndex]) {
                    ht->arBuckets[nIndex] = p->pNext;
                } else {
                    p->pLast->pNext = p->pNext;
                }
                
                if (p->pNext) {
                    p->pNext->pLast = p->pLast;
                }
                
                if (p->pListLast != NULL) {
                    p->pListLast->pListNext = p->pListNext;
                } else {
                    /* Deleting the head of the list */
                    ht->pListHead = p->pListNext;
                }
                
                if (p->pListNext != NULL) {
                    p->pListNext->pListLast = p->pListLast;
                } else {
                    ht->pListTail = p->pListLast;
                }
                
                if (ht->pInternalPointer == p) {
                    ht->pInternalPointer = p->pListNext;
                }
                
                if (ht->pDestructor) {
                    ht->pDestructor(p->pData);
                }
                
                if (p->pData != &p->pDataPtr) {
                    free(p->pData);
                }
                
                free(p);
                ht->nNumOfElements--;
                return SUCCESS;
            }
        p = p->pNext;
    }
    return FAILURE;
}


//Return number of elements in the table
int st_hash_num_elements(HashTable *ht)
{
    return ht->nNumOfElements;
}



