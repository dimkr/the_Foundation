/** @file map.c  Map of sorted unique integer keys.

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

#include "c_plus/map.h"

#include <stdlib.h>

iDefineTypeConstructionArgs(Map, (iMapNodeCmpFunc cmp), cmp)

enum iMapNodeFlag {
    red_MapNodeFlag = 0,
    black_MapNodeFlag = 1,
};

enum iRotation {
    left_Rotation = 0,
    right_Rotation = 1,
};

#define grandParent_MapNode_(d)         ((d)->parent? (d)->parent->parent : NULL)
#define isLeftChild_MapNode_(d)         ((d) == (d)->parent->child[0])
#define isRed_MapNode_(d)               ((d) && (d)->flags == red_MapNodeFlag)
#define isBlack_MapNode_(d)             (!(d) || (d)->flags == black_MapNodeFlag)
#define isChildBlack_MapNode_(d, ch)    (!(d) || !(d)->child[ch] || (d)->child[ch]->flags == black_MapNodeFlag)
#define isChildRed_MapNode_(d, ch)      (!isChildBlack_MapNode_(d, ch))

static iMapNode *sibling_MapNode_(iMapNode *d) {
    if (!d->parent) return NULL;
    return d->parent->child[isLeftChild_MapNode_(d)? 1 : 0];
}

static iMapNode *uncle_MapNode_(iMapNode *d) {
    if (!grandParent_MapNode_(d)) return NULL;
    return sibling_MapNode_(d->parent);
}

//static void replace_MapNode_(iMapNode *d, iMapNode *other) {
//    // Replace this node.
//    if (d->parent) {
//        d->parent->child[d->parent->child[0] == d? 0 : 1] = other;
//    }
//}

//static int childIndex_MapNode_(const iMapNode *d) {
//    iAssert(d->parent);
//    return d->parent->child[0] == d? 0 : 1;
//}

static int verify_MapNode_(iMapNode *d) {
    if (!d) return 1;
    if (d->child[0]) iAssert(d->key > d->child[0]->key);
    if (d->child[1]) iAssert(d->key < d->child[1]->key);
    if (isRed_MapNode_(d)) {
        iAssert(isChildBlack_MapNode_(d, 0));
        iAssert(isChildBlack_MapNode_(d, 1));
    }
    iAssert(!d->child[0] || d->child[0]->parent == d);
    iAssert(!d->child[1] || d->child[1]->parent == d);
    int bd0 = verify_MapNode_(d->child[0]);
    int bd1 = verify_MapNode_(d->child[1]);
    iAssert(bd0 == bd1);
    return bd0 + (isBlack_MapNode_(d)? 1 : 0);
}

static iMapNode *adjacent_MapNode_(iMapNode *d, int side) {
    if (!d->child[side]) return NULL;
    for (d = d->child[side]; d->child[side ^ 1]; d = d->child[side ^ 1]) {}
    return d;
}

static iMapNode **downLink_MapNode_(iMapNode *d) {
    if (!d->parent) return NULL;
    return d->parent->child + (d->parent->child[0] == d? 0 : 1);
}

static void rotate_MapNode_(iMapNode *d, enum iRotation dir) {
    iAssert(d);
    const int side = (dir == left_Rotation? 1 : 0);
    iMapNode **downLink = downLink_MapNode_(d);
    iMapNode *newD = d->child[side];
    iAssert(newD != NULL);
    d->child[side] = newD->child[side ^ 1];
    if (d->child[side]) d->child[side]->parent = d;
    newD->child[side ^ 1] = d;
    newD->parent = d->parent;
    if (downLink) *downLink = newD;
    d->parent = newD;
}

static void setChild_MapNode_(iMapNode *d, int index, iMapNode *child) {
    if (d) {
        d->child[index] = child;
    }
    if (child) {
        child->parent = d;
    }
}

static void replaceNode_Map_(iMap *d, iMapNode *node, iMapNode *replacement) {
    iMapNode **parentLink = downLink_MapNode_(node);
    if (parentLink) *parentLink = replacement; else d->root = replacement;
    replacement->parent = node->parent;
}

static void repairAfterInsert_MapNode_(iMapNode *node) {
    // Restore the required red/black properties.
    while (node) {
        if (!node->parent) {
            node->flags = black_MapNodeFlag;
            break;
        }
        else if (node->parent->flags == black_MapNodeFlag) {
            break;
        }
        else if (isRed_MapNode_(uncle_MapNode_(node))) {
            node->parent->flags = black_MapNodeFlag;
            uncle_MapNode_(node)->flags = black_MapNodeFlag;
            grandParent_MapNode_(node)->flags = red_MapNodeFlag;
            node = grandParent_MapNode_(node);
        }
        else {
            iMapNode *p = node->parent;
            iMapNode *g = grandParent_MapNode_(node);
            if (g->child[0] && node == g->child[0]->child[1]) {
                rotate_MapNode_(p, left_Rotation);
                node = node->child[0];
            }
            else if (g->child[1] && node == g->child[1]->child[0]) {
                rotate_MapNode_(p, right_Rotation);
                node = node->child[1];
            }
            p = node->parent;
            g = grandParent_MapNode_(node);
            rotate_MapNode_(g, node == p->child[0]? right_Rotation : left_Rotation);
            p->flags = black_MapNodeFlag;
            g->flags = red_MapNodeFlag;
            break;
        }
    }
}

static iMapNode *insertNode_Map_(iMap *d, iMapNode *insert) {
    iMapNode *root = d->root;
    while (root) {
        const int cmp = d->cmp(insert->key, root->key);
        if (cmp == 0) {
            // Replace this node; keys must be unique.
            replaceNode_Map_(d, root, insert);
            setChild_MapNode_(insert, 0, root->child[0]);
            setChild_MapNode_(insert, 1, root->child[1]);
            insert->flags = root->flags;
            return root; // The old node.
        }
        const int side = (cmp < 0? 0 : 1);
        if (root->child[side]) {
            root = root->child[side];
        }
        else {
            root->child[side] = insert;
            break;
        }
    }
    insert->parent = root;
    insert->child[0] = insert->child[1] = NULL;
    insert->flags = red_MapNodeFlag;
    return NULL; // New node added.
}

void init_Map(iMap *d, iMapNodeCmpFunc cmp) {
    d->size = 0;
    d->root = NULL;
    d->cmp = cmp;
}

void deinit_Map(iMap *d) {
    iUnused(d);
}

iBool contains_Map(const iMap *d, iMapKey key) {
    return value_Map(d, key) != NULL;
}

iMapNode *value_Map(const iMap *d, iMapKey key) {
    iMapNode *node = d->root;
    while (node) {
        const int cmp = d->cmp(key, node->key);
        if (cmp == 0) {
            return node;
        }
        else {
            node = node->child[cmp > 0? 1 : 0];
        }
    }
    return node;
}

void clear_Map(iMap *d) {
    d->size = 0;
    d->root = NULL;
}

static void updateRoot_Map_(iMap *d, iMapNode *node) {
    iMapNode *newRoot = node;
    while (newRoot->parent) {
        newRoot = newRoot->parent;
    }
    d->root = newRoot;
    iAssert(isBlack_MapNode_(d->root));
    verify_MapNode_(d->root);
}

iMapNode *insert_Map(iMap *d, iMapNode *node) {
    iMapNode *old = insertNode_Map_(d, node);
    if (old) {
        // The root may have been replaced.
        iAssert(isBlack_MapNode_(d->root));
        verify_MapNode_(d->root);
        return old;
    }
    repairAfterInsert_MapNode_(node);
    iAssert(node->flags == black_MapNodeFlag ||
            (isChildBlack_MapNode_(node, 0) && isChildBlack_MapNode_(node, 1)));
    updateRoot_Map_(d, node);
    d->size++;
    return NULL; // no previous node replaced
}

iMapNode *remove_Map(iMap *d, iMapKey key) {
    return removeNode_Map(d, value_Map(d, key));
}

static void repairAfterRemoval_Map_(iMap *d, iMapNode *node) {
    iAssert(node->flags == black_MapNodeFlag);
    while (node->parent) { // Not the root?
        iMapNode *s = sibling_MapNode_(node);
        if (isRed_MapNode_(s)) {
            node->parent->flags = red_MapNodeFlag;
            s->flags = black_MapNodeFlag;
            rotate_MapNode_(node->parent, node == node->parent->child[0]
                            ? left_Rotation : right_Rotation);
        }
        s = sibling_MapNode_(node);
        if (isBlack_MapNode_(node->parent) &&
                isBlack_MapNode_(s) &&
                isChildBlack_MapNode_(s, 0) &&
                isChildBlack_MapNode_(s, 1)) {
            if (s) s->flags = red_MapNodeFlag;
            node = node->parent;
            continue;
        }
        if (isRed_MapNode_(node->parent) &&
                isBlack_MapNode_(s) &&
                isChildBlack_MapNode_(s, 0) &&
                isChildBlack_MapNode_(s, 1)) {
            if (s) s->flags = red_MapNodeFlag;
            node->parent->flags = black_MapNodeFlag;
        }
        else {
            if (isBlack_MapNode_(s)) {
                const int side = isLeftChild_MapNode_(node)? 0 : 1;
                if (isChildBlack_MapNode_(s, side ^ 1) &&
                        isChildRed_MapNode_(s, side)) {
                    s->flags = red_MapNodeFlag;
                    if (s->child[side]) {
                        s->child[side]->flags = black_MapNodeFlag;
                    }
                    rotate_MapNode_(s, side? left_Rotation : right_Rotation);
                }
            }
            s = sibling_MapNode_(node);
            iAssert(s != NULL);
            if (s) s->flags = node->parent->flags;
            node->parent->flags = black_MapNodeFlag;

            const int side = isLeftChild_MapNode_(node)? 1 : 0;
            if (s && s->child[side]) s->child[side]->flags = black_MapNodeFlag;
            rotate_MapNode_(node->parent, side? left_Rotation : right_Rotation);
        }
        break;
    }
}

static void removeNodeWithZeroOrOneChild_Map_(iMap *d, iMapNode *node) {
    iAssert(!node->child[0] || !node->child[1]);
    iMapNode *child = node->child[node->child[0]? 0 : 1];
    if (node == d->root) {
        d->root = child;
        if (!child) return; // Tree became empty.
    }
    if (child) {
        replaceNode_Map_(d, node, child);
        if (node->flags == black_MapNodeFlag) {
            if (child->flags == red_MapNodeFlag) {
                child->flags = black_MapNodeFlag;
            }
            else {
                repairAfterRemoval_Map_(d, child);
            }
        }
    }
    else {
        if (node->flags == black_MapNodeFlag) {
            repairAfterRemoval_Map_(d, node);
        }
        if (node->parent) {
            *downLink_MapNode_(node) = NULL;
        }
    }
    // Update root after repairing the tree.
    iAssert(d->root);
    while (d->root->parent) d->root = d->root->parent;
}

static void swapNodes_Map_(iMap *d, iMapNode *node, iMapNode *other) {
    iAssert(d->root != other); // never replacing with the root
    // node and other will swap places in the tree.
    if (d->root == node) {
        d->root = other;
    }
    /* Swap colors. */ {
        int nf = node->flags;
        node->flags = other->flags;
        other->flags = nf;
    }
    iMapNode *npar = node->parent,  *nc0 = node ->child[0], *nc1 = node ->child[1];
    iMapNode *opar = other->parent, *oc0 = other->child[0], *oc1 = other->child[1];
    const int nside = (node->parent && isLeftChild_MapNode_(node))? 0 : 1;
    const int oside = isLeftChild_MapNode_(other)? 0 : 1;
    if (npar == other) npar = node;
    if (nc0  == other) nc0  = node;
    if (nc1  == other) nc1  = node;
    if (opar == node)  opar = other;
    if (oc0  == node)  oc0  = other;
    if (oc1  == node)  oc1  = other;
    // Set downlinks first.
    setChild_MapNode_(npar, nside, other);
    setChild_MapNode_(opar, oside, node);
    // Set all children.
    setChild_MapNode_(other, 0, nc0);
    setChild_MapNode_(other, 1, nc1);
    setChild_MapNode_(node,  0, oc0);
    setChild_MapNode_(node,  1, oc1);
}

iMapNode *removeNode_Map(iMap *d, iMapNode *node) {
    if (!node) return NULL;
    d->size--;
    if (node->child[0] && node->child[1]) {
        // It would be much simpler to just swap the values, but the node itself
        // is the value, so we have to swap the nodes.
        iMapNode *pred = adjacent_MapNode_(node, 0);
        swapNodes_Map_(d, node, pred);
    }
    removeNodeWithZeroOrOneChild_Map_(d, node);
    iAssert(isBlack_MapNode_(d->root));
    verify_MapNode_(d->root);
    return node;
}

//---------------------------------------------------------------------------------------

enum iMapIteratorDir {
    down_MapIteratorDir,
    up_MapIteratorDir,
};

static const iMapNode *firstInOrder_MapNode_(const iMapNode *d) {
    if (!d) return NULL;
    while (d->child[0]) {
        d = d->child[0];
    }
    return d;
}

static const iMapNode *nextInOrder_MapNode_(const iMapNode *d, int *dir) {
    if (!d) return NULL;
    // Switch to the next sibling.
    switch (*dir) {
        case down_MapIteratorDir:
            if (d->child[1]) {
                return firstInOrder_MapNode_(d->child[1]);
            }
            *dir = up_MapIteratorDir;
            if (!d->parent) return NULL;
            if (d->parent->child[0] == d) {
                return d->parent;
            }
            break;
        case up_MapIteratorDir:
            if (d->child[1]) {
                *dir = down_MapIteratorDir;
                return firstInOrder_MapNode_(d->child[1]);
            }
            break;
    }
    *dir = up_MapIteratorDir;
    // Go back up until there's a node on the right.
    for (;; d = d->parent) {
        if (!d->parent) return NULL; // Reached the root.
        if (d->parent->child[0] == d && d->parent->child[1]) return d->parent;
    }
}

void init_MapIterator(iMapIterator *d, iMap *map) {
    d->map = map;
    d->dir = down_MapIteratorDir;
    d->value = iConstCast(iMapNode *, firstInOrder_MapNode_(map->root));
    // The current node may be deleted, so keep the next one in a safe place.
    d->next = iConstCast(iMapNode *, nextInOrder_MapNode_(d->value, &d->dir));
}

void next_MapIterator(iMapIterator *d) {
    d->value = d->next;
    d->next = iConstCast(iMapNode *, nextInOrder_MapNode_(d->value, &d->dir));
}

iMapNode *remove_MapIterator(iMapIterator *d) {
    // This will potentially invalidate the next pointer as the structure of the
    // tree is altered!
    //return removeNode_Map(d->map, d->value);
}
