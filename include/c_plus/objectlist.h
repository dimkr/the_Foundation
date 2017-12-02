#pragma once

/** @file c_plus/objectlist.h  List of objects.

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

#include "list.h"
#include "object.h"

iDeclareClass(ObjectList)
iDeclareType(ObjectList)
iDeclareType(ObjectListNode)

struct Impl_ObjectList {
    iObject object;
    iList list;
};

struct Impl_ObjectListNode {
    iListNode node;
    iObject *object;
};

#define next_ObjectListNode(d)  ((iObjectListNode *) ((d)? (d)->node.next : NULL))
#define prev_ObjectListNode(d)  ((iObjectListNode *) ((d)? (d)->node.prev : NULL))

/**
 * Constructs a new list of objects.
 *
 * ObjectList is itself an Object.
 *
 * ObjectList owns its nodes, so deleting the list will delete all the nodes and
 * release references to the corresponding objects.
 *
 * @return ObjectList instance.
 */
iObjectList *   new_ObjectList          (void);

void            init_ObjectList         (iObjectList *);
void            deinit_ObjectList       (iObjectList *);

#define         isEmpty_ObjectList(d)   isEmpty_List(&(d)->list)
#define         size_ObjectList(d)      size_List(&(d)->list)

iObject *       front_ObjectList        (const iObjectList *);
iObject *       back_ObjectList         (const iObjectList *);

#define         begin_ObjectList(d)         ((iObjectListNode *) (d)->list.root.next)
#define         end_ObjectList(d)           ((iObjectListNode *) &(d)->list.root)
#define         constBegin_ObjectList(d)    ((const iObjectListNode *) (d)->list.root.next)
#define         constEnd_ObjectList(d)      ((const iObjectListNode *) &(d)->list.root)

void            clear_ObjectList        (iObjectList *);

iAnyObject *    pushBack_ObjectList     (iObjectList *, iAnyObject *object);
iAnyObject *    pushFront_ObjectList    (iObjectList *, iAnyObject *object);
iAnyObject *    insertAfter_ObjectList  (iObjectList *, iObjectListNode *after, iObject *object);
iAnyObject *    insertBefore_ObjectList (iObjectList *, iObjectListNode *before, iObject *object);
void            removeNode_ObjectList   (iObjectList *, iObjectListNode *node);
void            popFront_ObjectList     (iObjectList *);
void            popBack_ObjectList      (iObjectList *);

iDeclareIterator(ObjectList, iObjectList *)
void            remove_ObjectListIterator(iObjectListIterator *);
struct IteratorImpl_ObjectList {
    iObjectListNode *value;
    iObjectListNode *next;
    iObject *object;
    iObjectList *list;
};

iDeclareConstIterator(ObjectList, const iObjectList *)
struct ConstIteratorImpl_ObjectList {
    const iObjectListNode *value;
    const iObject *object;
    const iObjectList *list;
};
