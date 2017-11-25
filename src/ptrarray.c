/** @file ptrarray.c  Array of pointers.

@authors Copyright (c) 2017 Jaakko Keränen <jaakko.keranen@iki.fi>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

<small>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</small>
*/

#include "lite/ptrarray.h"

#include <stdarg.h>

iPtrArray *new_PtrArray(void) {
    return new_Array(sizeof(void *));
}

iPtrArray *newPointers_PtrArray(void *ptr, ...) {
    iPtrArray *d = new_PtrArray();
    pushBack_PtrArray(d, ptr);
    va_list args;
    va_start(args, ptr);
    void *next;
    while ((next = va_arg(args, void *)) != NULL) {
        pushBack_PtrArray(d, next);
    }
    va_end(args);
    return d;
}

void delete_PtrArray(iPtrArray *d) {
    delete_Array(d);
}

void **data_PtrArray(const iPtrArray *d) {
    return data_Array(d);
}

void *at_PtrArray(const iPtrArray *d, size_t pos) {
    void *ptr;
    memcpy(&ptr, at_Array(d, pos), sizeof(void *));
    return ptr;
}

void pushBack_PtrArray(iPtrArray *d, const void *ptr) {
    pushBack_Array(d, &ptr);
}

void pushFront_PtrArray(iPtrArray *d, const void *ptr) {
    pushFront_Array(d, &ptr);
}

iBool take_PtrArray(iPtrArray *d, size_t pos, void **outPtr) {
    return take_Array(d, pos, &outPtr);
}

void insert_PtrArray(iPtrArray *d, size_t pos, const void *ptr) {
    insert_Array(d, pos, &ptr);
}

//---------------------------------------------------------------------------------------

void init_PtrArrayIterator(iPtrArrayIterator *d, iPtrArray *array) {
    d->array = array;
    d->pos = 0;
    d->value = (!isEmpty_PtrArray(array)? at_PtrArray(array, 0) : NULL);
}

void next_PtrArrayIterator(iPtrArrayIterator *d) {
    if (++d->pos < size_Array(d->array)) {
        d->value = at_PtrArray(d->array, d->pos);
    }
    else {
        d->value = NULL;
    }
}

void init_PtrArrayConstIterator(iPtrArrayConstIterator *d, const iPtrArray *array) {
    d->array = array;
    const size_t size = size_Array(array);
    d->value = (size > 0? at_PtrArray(array, 0) : NULL); // element
    d->next  = (size > 1? at_Array   (array, 1) : NULL); // pointer to next element
}

void next_PtrArrayConstIterator(iPtrArrayConstIterator *d) {
    if (d->next >= (const void * const *) constEnd_Array(d->array)) {
        d->value = NULL;
    }
    else {
        d->value = *d->next++;
    }
}
