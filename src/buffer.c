/** @file buffer.c  Memory stream.

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

#include "c_plus/buffer.h"
#include "c_plus/string.h"

static iBeginDefineClass(Buffer)
    .super  = &Class_Stream,
    .seek   = (long (*)(iStream *, long))                   seek_Buffer,
    .read   = (size_t (*)(iStream *, size_t, void *))       read_Buffer,
    .write  = (size_t (*)(iStream *, const void *, size_t)) write_Buffer,
    .flush  = (void (*)(iStream *))                         flush_Buffer,
iEndDefineClass(Buffer)

iDefineObjectConstruction(Buffer)

void init_Buffer(iBuffer *d) {
    init_Stream(&d->stream);
    init_Block(&d->block, 0);
    d->data = NULL;
    d->mode = readWrite_BufferMode;
}

void deinit_Buffer(iBuffer *d) {
    deinit_Block(&d->block);
}

iBool open_Buffer(iBuffer *d, const iBlock *data) {
    if (isOpen_Buffer(d) || !data) return iFalse;
    set_Block(&d->block, data); // this copy of the data will be accessed
    d->data = &d->block;
    d->mode = readOnly_BufferMode;
    setSize_Stream(&d->stream, size_Block(data));
    return iTrue;
}

iBool openData_Buffer(iBuffer *d, iBlock *data) {
    if (isOpen_Buffer(d) || !data) return iFalse;
    clear_Block(&d->block);
    d->data = data;
    d->mode = readWrite_BufferMode;
    setSize_Stream(&d->stream, size_Block(data));
    return iTrue;
}

iBool openEmpty_Buffer(iBuffer *d) {
    if (isOpen_Buffer(d)) return iFalse;
    clear_Block(&d->block);
    d->data = &d->block;
    d->mode = readWrite_BufferMode;
    setSize_Stream(&d->stream, 0);
    return iTrue;
}

void close_Buffer(iBuffer *d) {
    if (isOpen_Buffer(d)) {
        d->data = NULL;
        clear_Block(&d->block);
    }
}

long seek_Buffer(iBuffer *d, long offset) {
    if (isOpen_Buffer(d)) {
        return iMin(offset, (long) size_Block(d->data));
    }
    return pos_Stream(&d->stream);
}

size_t read_Buffer(iBuffer *d, size_t size, void *data_out) {
    if (isOpen_Buffer(d)) {
        if (atEnd_Buffer(d)) return 0;
        const iRanges range = { pos_Buffer(d), iMin(pos_Buffer(d) + size, size_Block(d->data)) };
        memcpy(data_out, constBegin_Block(d->data) + range.start, size_Range(&range));
        return size_Range(&range);
    }
    return 0;
}

size_t write_Buffer(iBuffer *d, const void *data, size_t size) {
    if (isOpen_Buffer(d) && (d->mode & readWrite_BufferMode)) {
        setSubData_Block(d->data, pos_Buffer(d), data, size);
        return size;
    }
    return 0;
}

void flush_Buffer(iBuffer *d) {
    iUnused(d);
}
