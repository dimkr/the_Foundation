/** @file object.c  Object base class.

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

#include "lite/object.h"
#include "lite/ptrarray.h"
#include "lite/ptrset.h"

#include <stdio.h>
#include <stdlib.h>

iObject *iObject_new(size_t size, iDeinitFunc deinit) {
    LITE_ASSERT(size >= sizeof(iObject));
    iObject *d = calloc(size, 1);
    d->deinit = deinit;
    iPtrSet_init(&d->children);
    printf("new Object %p\n", d);
    return d;
}

void iObject_delete(iObject *d) {
    iObject_setParent(d, NULL);
    // Destroy children, who will remove themselves.
    while (!iPtrSet_isEmpty(&d->children)) {
        iObject_delete(iPtrSet_at(&d->children, 0));
    }
    iPtrSet_deinit(&d->children);
    if (d->deinit) {
        d->deinit(d);
    }
    free(d);
    printf("deleted Object %p\n", d);
}

void iObject_setParent(iObject *d, iObject *parent) {
    if (d->parent == parent) return;
    if (d->parent) {
        // Remove from old parent.
        iPtrSet_remove(&d->parent->children, d);
    }
    d->parent = parent;
    if (parent) {
        LITE_ASSERT(!iPtrSet_contains(&d->parent->children, d));
        iPtrSet_insert(&d->parent->children, d);
    }
}

iObject *iObject_parent(const iObject *d) {
    return d->parent;
}

const iPtrSet *iObject_children(const iObject *d) {
    return &d->children;
}
