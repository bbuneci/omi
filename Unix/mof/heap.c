/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#include <stdio.h>
#include "heap.h"
#include "types.h"
#include <pal/intsafe.h>

#define MAGIC 0x41E25C10

typedef struct _MOF_Block
{
    unsigned int magic;
    unsigned int size;
    struct _MOF_Block* next;
    struct _MOF_Block* prev;
}
MOF_Block;

void* MOF_Malloc(MOF_Heap* self, size_t size)
{
    MOF_Block* p;

    if (!self)
        return NULL;

    size_t allocSize = 0;
    if (SizeTAdd(sizeof(MOF_Block), size, &allocSize) != S_OK)
        return NULL;

    p = (MOF_Block*)PAL_Malloc(allocSize);

    if (!p)
        return NULL;

    if (self->head)
    {
        p->prev = NULL;
        p->next = self->head;
        self->head->prev = p;
        self->head = p;
    }
    else
    {
        p->prev = NULL;
        p->next = NULL;
        self->head = p;
    }

    p->magic = MAGIC;
    p->size = (unsigned int)size;

    /* Fill memory with a non-zero character */
    memset(p + 1, 0xAA, size);

    return p + 1;
}

void* MOF_Calloc(MOF_Heap* self, size_t count, size_t size)
{
    void* ptr;

    if (!self)
        return NULL;

    size_t allocSize = 0;
    if (SizeTMult(count, size, &allocSize) != S_OK)
        return NULL;

    ptr = MOF_Malloc(self, allocSize);

    if (!ptr)
        return NULL;

    memset(ptr, 0, count * size);
    return ptr;
}

void* MOF_Realloc(MOF_Heap* self, void* ptr, size_t size)
{
    MOF_Block* p;

    if (!self)
        return NULL;

    if (!ptr)
        return MOF_Malloc(self, size);

    p = (MOF_Block*)ptr - 1;
    MOF_ASSERT(p->magic == MAGIC);

#if 0
    /* Fill released part with a non-zero pattern */
    if (p->size > size)
        memset((char*)ptr + size, 0xDD, p->size - size);
#endif

    p = (MOF_Block*)PAL_Realloc(p, sizeof(MOF_Block) + size);

    if (!p)
        return NULL;

#if 0
    /* Fill with a non-zero pattern */
    if (size > p->size)
        memset((char*)(p + 1) + p->size, 0xFF, size - p->size);
#endif

    if (p->prev)
        p->prev->next = p;
    else
        self->head = p;

    if (p->next)
        p->next->prev = p;

    p->size = (unsigned int)size;

    return p + 1;
}

void MOF_Free(MOF_Heap* self, void* ptr)
{
    MOF_Block* p;

    if (!self || !ptr)
        return;

    p = (MOF_Block*)ptr - 1;
    MOF_ASSERT(p->magic == MAGIC);

    if (p->prev)
        p->prev->next = p->next;
    else
        self->head = p->next;

    if (p->next)
        p->next->prev = p->prev;

    /* Fill released memory with 0xDD characters */
    memset(p, 0xDD, sizeof(MOF_Block) + p->size);

    PAL_Free(p);
}

void MOF_Release(MOF_Heap* self)
{
    MOF_Block* p;

    if (!self)
        return;

    for (p = self->head; p; )
    {
        MOF_Block* next = p->next;
        PAL_Free(p);
        p = next;
    }

    self->head = NULL;
}

char* MOF_Strdup(MOF_Heap* self, const char* str)
{
    char* p;
    size_t n;

    if (!self || !str)
        return  NULL;

    n = strlen(str);
    p = (char*)MOF_Malloc(self, n + 1);

    if (!p)
        return NULL;

    memcpy(p, str, n + 1);

    return p;
}
