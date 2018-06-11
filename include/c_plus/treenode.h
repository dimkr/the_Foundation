#pragma once

/** @file c_plus/treenode.h  Tree node base class.

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

#include "defs.h"
#include "class.h"
#include "list.h"

iBeginPublic

/**
 * Object that owns child objects and may have a parent. When a parent is deleted,
 * all its children are deleted first.
 */
iDeclareType(TreeNode)

struct Impl_TreeNode {
    iListNode node;
    const iClass *class;
    iTreeNode *parent;
    iList *children;
};

typedef void iAnyTreeNode;

iAnyTreeNode *  new_TreeNode(const iAnyClass *class);
void            delete_TreeNode(iAnyObject *);

#define         collect_TreeNode(d) iCollectDel(d, delete_TreeNode)

iAnyTreeNode *  parent_TreeNode(const iAnyTreeNode *);
const iList *   children_TreeNode(const iAnyTreeNode *);

void            setParent_TreeNode(iAnyTreeNode *, iAnyTreeNode *parent);

iEndPublic
