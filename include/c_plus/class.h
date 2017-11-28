#pragma once

/** @file c_plus/class.h  Class object.

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

#include "defs.h"

iDeclareType(Class)

typedef void iAnyClass;

// The mandatory members:
struct Impl_Class {
    const iClass *super;
    const char *name;
    size_t size;
    void (*deinit)(void *);
};

#define iBeginDeclareClass(className) \
    iDeclareType(className##Class) \
    struct Impl_##className##Class { \
        const iAnyClass *super; \
        const char *name; \
        size_t size; \
        void (*deinit)(void *);

#define iEndDeclareClass(className) \
    }; \
    extern i##className##Class Class_##className;

#define iEndDeclareStaticClass(className) \
    }; \
    iDeclareType(className##Class)

#define iDeclareStaticClass(className) \
    iBeginDeclareClass(className) \
    iEndDeclareStaticClass(className)

#define iDeclareClass(className) \
    iBeginDeclareClass(className) iEndDeclareClass(className)

#define iBeginDefineClass(className) \
    i##className##Class Class_##className = { \

#define iEndDefineClass(className) \
        .name = #className, \
        .size = sizeof(i##className), \
        .deinit = (void (*)(void *)) deinit_##className, \
    };

#define iDefineClass(className) \
    iBeginDefineClass(className) \
        .super = NULL, \
    iEndDefineClass(className)

#define iDefineSubclass(className, superClass) \
    iBeginDefineClass(className) \
        .super = &Class_##superClass, \
    iEndDefineClass(className)

void deinit_Class(const void *class, void *object);
void delete_Class(const void *class, void *object);