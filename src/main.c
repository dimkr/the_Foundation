/**
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

#include "c_plus/array.h"
#include "c_plus/block.h"
#include "c_plus/class.h"
#include "c_plus/garbage.h"
#include "c_plus/math.h"
#include "c_plus/object.h"
#include "c_plus/ptrarray.h"
#include "c_plus/regexp.h"
#include "c_plus/string.h"
#include "c_plus/stringhash.h"
#include "c_plus/treenode.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

//---------------------------------------------------------------------------------------

typedef struct Impl_TestNode {
    iTreeNode node;
    int value;
}
TestNode;

void init_TestNode(TestNode *d, int value) {
    d->value = value;
}

void deinit_TestNode(iAnyObject *obj) {
    TestNode *d = (TestNode *) obj;
    printf("deinit TestNode: %i\n", d->value);
}

static iDefineClass(iClass, TestNode);

TestNode *new_TestNode(int value) {
    TestNode *d = new_TreeNode(&Class_TestNode);
    init_TestNode(d, value);
    return d;
}

//---------------------------------------------------------------------------------------

typedef struct Impl_SuperNode {
    TestNode testNode;
    int member;
}
SuperNode;

void init_SuperNode(SuperNode *d, int member) {
    d->member = member;
}

void deinit_SuperNode(iAnyObject *any) {
    SuperNode *d = (SuperNode *) any;
    printf("deinit SuperNode: %i\n", d->member);
}

static iDefineSubclass(iClass, SuperNode, TestNode);

SuperNode *new_SuperNode(int value, int member) {
    SuperNode *d = new_TreeNode(&Class_SuperNode);
    init_TestNode(&d->testNode, value);
    init_SuperNode(d, member);
    return d;
}

//---------------------------------------------------------------------------------------

typedef struct Impl_TestObject {
    iObject object;
    int value;
}
TestObject;

void deinit_TestObject(iAnyObject *any) {
    TestObject *d = (TestObject *) any;
    printf("deinit TestObject: %i\n", d->value);
}

static iDefineClass(iClass, TestObject);

TestObject *new_TestObject(int value) {
    TestObject *d = new_Object(&Class_TestObject);
    d->value = value;
    return d;
}

//---------------------------------------------------------------------------------------

typedef struct Impl_TestElement {
    iStringHashElement base;
    float member;
}
TestElement;

TestElement *new_TestElement(void) {
    TestElement *d = malloc(sizeof(TestElement));
    d->member = iRandomf();
    return d;
}

void printArray(const iArray *list) {
    printf("%4lu %4lu -> %-4lu : %4lu [", size_Array(list), list->range.start, list->range.end, list->allocSize);
    for (int i = 0; i < list->allocSize * list->elementSize; ++i) {
        if (i/list->elementSize < list->range.start || i/list->elementSize >= list->range.end)
            printf(" __");
        else
            printf(" %02x", list->data[i]);
    }
    printf(" ]\n");
}

static int compareElements(const void *a, const void *b) {
    return strncmp(a, b, 2);
}

int main(int argc, char *argv[]) {
    iUnused(argc);
    iUnused(argv);
    srand(time(NULL));
    setlocale(LC_CTYPE, "utf-8");
    /* Test list insertion and removal. */ {
        printf("Array insertions/removals:\n");
        iArray *list = new_Array(2);
        printArray(list);
        {
            printf("Iterating the empty array:\n");
            iForEach(Array, i, list) {
                printf("- %p\n", i.value);
            }
        }
        pushBack_Array(list, "00"); printArray(list);
        pushBack_Array(list, "11"); printArray(list);
        pushBack_Array(list, "22"); printArray(list);
        pushBack_Array(list, "33"); printArray(list);
        pushBack_Array(list, "44"); printArray(list);
        pushBack_Array(list, "55"); printArray(list);
        pushBack_Array(list, "66"); printArray(list);
        pushBack_Array(list, "77"); printArray(list);
        pushBack_Array(list, "88"); printArray(list);
        pushBack_Array(list, "99"); printArray(list);
        insert_Array(list, 7, "XX"); printArray(list);
        insert_Array(list, 7, "YY"); printArray(list);
        insert_Array(list, 8, "ZZ"); printArray(list);
        pushFront_Array(list, "aa"); printArray(list);
        pushBack_Array(list, "bb"); printArray(list);
        pushBack_Array(list, "cc"); printArray(list);
        sort_Array(list, compareElements); printArray(list);
        popBack_Array(list); printArray(list);
        popBack_Array(list); printArray(list);
        popBack_Array(list); printArray(list);
        popBack_Array(list); printArray(list);
        popBack_Array(list); printArray(list);
        popBack_Array(list); printArray(list);
        popFront_Array(list); printArray(list);
        remove_Array(list, 6); printArray(list);
        remove_Array(list, 5); printArray(list);
        remove_Array(list, 4); printArray(list);
        remove_Array(list, 3); printArray(list);
        remove_Array(list, 2); printArray(list);
        {
            printf("Iterating the array:\n");
            iConstForEach(Array, i, list) {
                printf("- %p\n", i.value);
            }
        }
        delete_Array(list);
    }
    /* Test an array of pointers. */ {
        iPtrArray *par = newPointers_PtrArray("Entry One", "Entry Two", 0);
        printf("Iterating the pointer array:\n");
        iConstForEach(PtrArray, i, par) {
            printf("- %s\n", i.ptr);
        }
        delete_PtrArray(par);
    }
    /* Test a hash. */ {
        iStringHash *h = new_StringHash();
        insertValuesCStr_StringHash(h,
              "one", iDeref(new_TestObject(1000)),
              "two", iDeref(new_TestObject(1001)), 0);
        printf("Hash has %zu elements:\n", size_StringHash(h));
        iForEach(StringHash, i, h) {
            printf("  %s: %i\n",
                   cstr_String(key_StringHashIterator(&i)),
                   ((TestObject *) i.value->object)->value);
        }
        delete_StringHash(h);
        printf("Hash deleted.\n");
    }
    /* Test tree nodes. */ {
        TestNode *a = new_TestNode(1);
        TestNode *b = new_TestNode(2);
        SuperNode *c = new_SuperNode(3, 100);
        setParent_TreeNode(b, a);
        setParent_TreeNode(c, a);
        printf("Children:\n");
        iConstForEach(List, i, a->node.children) {
            printf("- %p\n", i.value);
        }
        delete_TreeNode(b);
        delete_TreeNode(a);
    }
    /* Test reference counting. */ {
        TestObject *a = new_TestObject(123);
        TestObject *b = ref_Object(a);
        printf("deref a...\n"); deref_Object(a);
        printf("deref b...\n"); deref_Object(b);
    }
    /* Test blocks and garbage collector. */ {
        iBlock *a = collect_Block(new_Block(0));
        appendCStr_Block(a, "Hello World");
        appendCStr_Block(a, "!");
        iBlock *b = collect_Block(copy_Block(a));
        iBlock *c = collect_Block(concat_Block(a, b));
        clear_Block(a);
        printf_Block(a, "Hello %i World!", 123);
        printf("Block: %s\n", constData_Block(a));
        printf_Block(a, "What");
        pushBack_Block(a, '?');
        printf("Block: %s %s\n", constData_Block(a), constData_Block(b));
        printf("c-Block: %s\n", constData_Block(c));
        printf("mid: %s\n", constData_Block(collect_Block(mid_Block(b, 3, 4))));
        iRecycle();
    }
    /* Test Unicode strings. */ {
        iString *s = collect_String(newCStr_String("A_Äö\U0001f698a"));
        printf("String: %s length: %zu size: %zu\n", cstr_String(s), length_String(s), size_String(s)); {
            iConstForEach(String, i, s) {
                printf(" char: %04x [%lc]\n", i.value, i.value);
            }
        }
        printf("Backwards:\n"); {
            iConstReverseForEach(String, i , s) {
                printf(" char: %04x [%lc]\n", i.value, i.value);
            }
        }
        printf("Starts with: %i %i\n", startsWith_String(s, "a"), startsWithCase_String(s, "a"));
        printf("Ends with: %i %i\n", endsWith_String(s, "a"), endsWithCase_String(s, "A"));
        printf("Mid: %s\n", cstr_String(collect_String(mid_String(s, 3, 1))));
        printf("ö is at: %zu %zu\n", indexOfCStr_String(s, "ö"), indexOf_String(s, u'ö'));
        truncate_String(s, 3);
        printf("Truncated: %s\n", cstr_String(s));
    }
    /* Test regular expressions. */ {
        iString *s = newCStr_String("Hello world Äöäö, there is a \U0001f698 out there.");
        iRegExp *rx = new_RegExp("\\b(THERE|WORLD|äöäö)\\b", caseInsensitive_RegExpOption);
        iRegExpMatch match;
        while (matchString_RegExp(rx, s, &match)) {
            iString *cap = captured_RegExpMatch(&match, 1);
            printf("match: %i -> %i [%s]\n", match.range.start, match.range.end, cstr_String(cap));
            delete_String(cap);
        }
        delete_RegExp(rx);
        delete_String(s);
    }
    /* Test zlib compression. */ {
        iString *s = newCStr_String("Hello world. "
                                    "Hello world. "
                                    "Hello world. "
                                    "Hello world. "
                                    "Hello world. "
                                    "Hello world. "
                                    "Hello world. "
                                    "Hello world. "
                                    "Hello world.");
        iBlock *compr = compress_Block(&s->chars);
        printf("Original: %zu Compressed: %zu\n", size_String(s), size_Block(compr));
        iBlock *restored = decompress_Block(compr);
        printf("Restored %zu: %s\n", size_Block(restored), constData_Block(restored));
        delete_Block(restored);
        delete_Block(compr);
    }
    return 0;
}
