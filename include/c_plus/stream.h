#pragma once

/** @file c_plus/stream.h  Base class for streams.

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
#include "object.h"

iDeclareType(Block)
iDeclareType(Stream)
iDeclareType(StringList)

iBeginDeclareClass(Stream)
    uint64_t    (*seek) (iStream *, int64_t offset);
    size_t      (*read) (iStream *, size_t size, void *data_out);
    size_t      (*write)(iStream *, const void *data, size_t size);
    void        (*flush)(iStream *);
iEndDeclareClass(Stream)

struct Impl_Stream {
    iObject object;
    uint64_t size;
    uint64_t pos;
};

void        init_Stream     (iStream *);
void        deinit_Stream   (iStream *);

#define     size_Stream(d)      ((d)->size)
#define     pos_Stream(d)       ((d)->pos)
#define     atEnd_Stream(d)     ((d)->pos == (d)->size)

void        seek_Stream         (iStream *, int64_t offset);
iBlock *    read_Stream         (iStream *, size_t size);
size_t      readBlock_Stream    (iStream *, size_t size, iBlock *data_out);
iBlock *    readAll_Stream      (iStream *);
size_t      write_Stream        (iStream *, const iBlock *data);
size_t      writeData_Stream    (iStream *, const void *data, size_t size);

iStringList *readLines_Stream   (iStream *);

void        flush_Stream    (iStream *);
