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

iList *children_Object_(iObject *d) {
    if (!d->children) {
        d->children = new_List();
    }
    return d->children;
}

void *new_Object(const iClass *class) {
    iAssert(class != NULL);
    iAssert(class->instanceSize >= sizeof(iObject));
    iObject *d = calloc(class->instanceSize, 1);
    d->class = class;
    printf("new Object %p\n", d);
    return d;
}

void delete_Object(iAnyObject *any) {
    iObject *d = (iObject *) any;
    setParent_Object(d, NULL);
    // Destroy children, who will remove themselves.
    while (!isEmpty_List(d->children)) {
        delete_Object(front_List(d->children));
    }
    delete_List(d->children);
    deinit_Class(d->class, d);
    free(d);
    printf("deleted Object %p\n", d);
}

void setParent_Object(iAnyObject *any, iAnyObject *parent) {
    iObject *d = (iObject *) any;
    if (d->parent == parent) return;
    if (d->parent) {
        // Remove from old parent.
        iAssert(d->parent->children);
        remove_List(d->parent->children, d);
    }
    d->parent = parent;
    if (parent) {
        pushBack_List(children_Object_(d->parent), d);
    }
}

iAnyObject *parent_Object(const iAnyObject *d) {
    return ((const iObject *) d)->parent;
}

const iList *children_Object(const iAnyObject *d) {
    return children_Object_(iConstCast(iObject *, d));
}
