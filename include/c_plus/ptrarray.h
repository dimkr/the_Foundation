#pragma once

/** @file c_plus/ptrarray.h  Array of pointers.

@authors Copyright (c) 2017 Jaakko Keränen <jaakko.keranen@iki.fi>

@par License

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

#include "array.h"

typedef iArray iPtrArray;

iPtrArray * new_PtrArray            (void);
iPtrArray * newPointers_PtrArray    (void *ptr, ...); // NULL-terminated
void        delete_PtrArray         (iPtrArray *);

#define     init_PtrArray(d)        init_Array(d, sizeof(void *))
#define     deinit_PtrArray(d)      deinit_Array(d)

#define     isEmpty_PtrArray(d)     isEmpty_Array(d)
#define     size_PtrArray(d)        size_Array(d)

void **         data_PtrArray       (iPtrArray *);
const void **   constData_PtrArray  (const iPtrArray *);
void *          at_PtrArray         (const iPtrArray *, size_t pos);
void            set_PtrArray        (iPtrArray *, size_t pos, const void *ptr);

void        pushBack_PtrArray       (iPtrArray *, const void *ptr);
void        pushFront_PtrArray      (iPtrArray *, const void *ptr);
iBool       take_PtrArray           (iPtrArray *, size_t pos, void **outPtr);
void        insert_PtrArray         (iPtrArray *, size_t pos, const void *ptr);

#define     resize_PtrArray(d, s)   resize_Array(d, s)

iDeclareIterator(PtrArray, iPtrArray *)
iDeclareConstIterator(PtrArray, const iPtrArray *)

#define index_PtrArrayIterator(d)               index_ArrayIterator(d)
#define index_PtrArrayConstIterator(d)          index_ArrayConstIterator(d)
#define index_PtrArrayReverseIterator(d)        index_ArrayReverseIterator(d)
#define index_PtrArrayReverseConstIterator(d)   index_ArrayReverseConstIterator(d)

struct IteratorImpl_PtrArray {
    union {
        void **value; // pointer to array element
        iArrayIterator iter;
    };
    void *ptr; // array element
};
struct ConstIteratorImpl_PtrArray {
    union {
        const void * const *value; // pointer to array element
        iArrayConstIterator iter;
    };
    const void *ptr; // array element
};
