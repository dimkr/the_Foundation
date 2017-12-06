/** @file list.c  Doubly linked list.

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

#include "c_plus/list.h"

#include <stdlib.h>

#define iListNode_(ptr) ((iListNode *) (ptr))

static iListNode *nextNode_List_(const iList *d, const iListNode *node) {
    if (!node || node->next == &d->root) return NULL;
    return node->next;
}

static iListNode *prevNode_List_(const iList *d, const iListNode *node) {
    if (!node || node->prev == &d->root) return NULL;
    return node->prev;
}

iDefineTypeConstruction(List)

void init_List(iList *d) {
    clear_List(d);
}

void deinit_List(iList *d) {
    // List doesn't own any memory, so nothing needs to be deinitialized.
    iUnused(d);
}

size_t size_List(const iList *d) {
    return d? d->size : 0;
}

void *front_List(const iList *d) {
    return d->size > 0? d->root.next : NULL;
}

void *back_List(const iList *d) {
    return d->size > 0? d->root.prev : NULL;
}

void clear_List(iList *d) {
    d->root.next = d->root.prev = &d->root;
    d->size = 0;
}

iAny *pushBack_List(iList *d, iAny *node) {
    return insertBefore_List(d, &d->root, node);
}

iAny *pushFront_List(iList *d, iAny *node) {
    return insertAfter_List(d, &d->root, node);
}

static void insertAfter_ListNode_(iListNode *d, iListNode *after) {
    d->next = after->next;
    after->next->prev = d;
    d->prev = after;
    after->next = d;
}

static void insertBefore_ListNode_(iListNode *d, iListNode *before) {
    d->prev = before->prev;
    before->prev->next = d;
    d->next = before;
    before->prev = d;
}

iAny *insertAfter_List(iList *d, iAny *after, iAny *node) {
    iAssert(node);
    if (!after) after = d->root.prev;
    insertAfter_ListNode_(node, after);
    d->size++;
    return node;
}

iAny *insertBefore_List(iList *d, iAny *before, iAny *node) {
    iAssert(node);
    if (!before) before = d->root.next;
    insertBefore_ListNode_(node, before);
    d->size++;
    return node;
}

iAny *popFront_List(iList *d) {
    if (d->size == 0) return NULL;
    return remove_List(d, front_List(d));
}

iAny *popBack_List(iList *d) {
    if (d->size == 0) return NULL;
    return remove_List(d, back_List(d));
}

static void remove_ListNode_(iListNode *d) {
    d->next->prev = d->prev;
    d->prev->next = d->next;
    d->next = d->prev = NULL;
}

iAny *remove_List(iList *d, iAny *node) {
    iAssert(node);
    iAssert(d->size > 0);
    d->size--;
    remove_ListNode_(node);
    return node;
}

static void swapNodes_ListNode_(iListNode *d, iListNode *other) {
    iAssert(d != other);
    if (d->next == other) {
        remove_ListNode_(other);
        insertBefore_ListNode_(other, d);
    }
    else if (d->prev == other) {
        remove_ListNode_(other);
        insertAfter_ListNode_(other, d);
    }
    else {
        iListNode *mark = d->prev;
        remove_ListNode_(d);
        insertAfter_ListNode_(d, other);
        remove_ListNode_(other);
        insertAfter_ListNode_(other, mark);
    }
}

static iListNode *quicksortPartition_ListNode_(iListNode *low, iListNode *high,
                                               iListCompareFunc cmp) {
    iListNode *pivot = high;
    iListNode *i = low->prev;
    for (iListNode *j = low; j != high; j = j->next) {
        if (cmp(j, pivot) < 0) {
            i = i->next;
            swapNodes_ListNode_(i, j); {
                // Iteration must continue unaffected.
                iListNode *k = i; i = j; j = k;
            }
        }
    }
    if (cmp(high, i->next) < 0) {
        swapNodes_ListNode_(i->next, high);
    }
    return i->next;
}

static void quicksort_ListNode_(iListNode *low, iListNode *high,
                                iListCompareFunc cmp) {
    if (low != high) {
        iListNode *p = quicksortPartition_ListNode_(low, high, cmp);
        quicksort_ListNode_(low, p->prev, cmp);
        quicksort_ListNode_(p->next, high, cmp);
    }
}

void sort_List(iList *d, iListCompareFunc cmp) {
    if (size_List(d) >= 2) {
        quicksort_ListNode_(front_List(d), back_List(d), cmp);
    }
}

//---------------------------------------------------------------------------------------

#define init_ListIterator_(d, list) \
    { d->list  = list; \
      d->value = nextNode_List_(list, &list->root); \
      d->next  = nextNode_List_(list, d->value); }

#define next_ListIterator_(d) \
    { d->value = d->next; \
      d->next  = nextNode_List_(d->list, d->value); }

void init_ListIterator(iListIterator *d, iList *list) {
    init_ListIterator_(d, list);
}

void next_ListIterator(iListIterator *d) {
    next_ListIterator_(d);
}

void init_ListConstIterator(iListConstIterator *d, const iList *list) {
    init_ListIterator_(d, list);
}

void next_ListConstIterator(iListConstIterator *d) {
    next_ListIterator_(d);
}

//---------------------------------------------------------------------------------------

#define init_ListReverseIterator_(d, list) \
    { d->list  = list; \
      d->value = prevNode_List_(list, &list->root); \
      d->next  = prevNode_List_(list, d->value); }

#define next_ListReverseIterator_(d) \
    { d->value = d->next; \
      d->next  = prevNode_List_(d->list, d->value); }

void init_ListReverseIterator(iListReverseIterator *d, iList *list) {
    init_ListReverseIterator_(d, list);
}

void next_ListReverseIterator(iListReverseIterator *d) {
    next_ListReverseIterator_(d);
}

void init_ListReverseConstIterator(iListReverseConstIterator *d, const iList *list) {
    init_ListReverseIterator_(d, list);
}

void next_ListReverseConstIterator(iListReverseConstIterator *d) {
    next_ListReverseIterator_(d);
}
