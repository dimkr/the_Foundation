/** @file string.c  UTF-8 text string with copy-on-write semantics.

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

#include "c_plus/string.h"
#include "c_plus/stringlist.h"
#include "c_plus/range.h"

#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

iString *new_String(void) {
    iString *d = iMalloc(String);
    init_String(d);
    return d;
}

iString *newCStr_String(const char *cstr) {
    return newCStrN_String(cstr, strlen(cstr));
}

iString *newCStrN_String(const char *cstr, size_t size) {
    iString *d = iMalloc(String);
    initData_Block(&d->chars, cstr, size);
    return d;
}

iString *newBlock_String(const iBlock *data) {
    iString *d = iMalloc(String);
    initCopy_Block(&d->chars, data);
    return d;
}

iString *copy_String(const iString *d) {
    iString *copy = iMalloc(String);
    initCopy_Block(&copy->chars, &d->chars);
    return copy;
}

void delete_String(iString *d) {
    if (d) {
        deinit_String(d);
        free(d);
    }
}

void init_String(iString *d) {
    init_Block(&d->chars, 0);
}

void initBlock_String(iString *d, const iBlock *chars) {
    initCopy_Block(&d->chars, chars);
}

void initCStr_String(iString *d, const char *cstr) {
    initCStrN_String(d, cstr, strlen(cstr));
}

void initCStrN_String(iString *d, const char *cstr, size_t size) {
    initData_Block(&d->chars, cstr, size);
}

void initCopy_String(iString *d, const iString *other) {
    initCopy_Block(&d->chars, &other->chars);
}

void deinit_String(iString *d) {
    deinit_Block(&d->chars);
}

void clear_String(iString *d) {
    clear_Block(&d->chars);
}

void truncate_String(iString *d, size_t len) {
    const char *start = constData_Block(&d->chars);
    const char *pos = start;
    iConstForEach(String, i, d) {
        if (len-- == 0) break;
        pos = i.next;
    }
    truncate_Block(&d->chars, pos - start);
}

const char *cstr_String(const iString *d) {
    return constData_Block(&d->chars);
}

size_t length_String(const iString *d) {
    size_t len = 0;
    iConstForEach(String, i, d) len++;
    return len;
}

size_t size_String(const iString *d) {
    return size_Block(&d->chars);
}

iString *mid_String(const iString *d, size_t start, size_t count) {
    if (count == 0) return new_String();
    const char *chars = constData_Block(&d->chars);
    iRanges range = { 0, size_Block(&d->chars) };
    size_t pos = 0;
    iConstForEach(String, i, d) {
        if (pos > start && pos == start + count) {
            range.end = i.pos - chars;
            break;
        }
        else if (pos == start) {
            range.start = i.pos - chars;
            if (count == iInvalidSize) break;
        }
        else
        pos++;
    }
    iBlock *midChars = midRange_Block(&d->chars, &range);
    iString *mid = newBlock_String(midChars);
    delete_Block(midChars);
    return mid;
}

int cmpSc_String(const iString *d, const char *cstr, const iStringComparison *cs) {
    return cs->cmp(constData_Block(&d->chars), cstr);
}

iBool startsWithSc_String(const iString *d, const char *cstr, const iStringComparison *cs) {
    const size_t len = strlen(cstr);
    if (size_String(d) < len) return iFalse;
    return !cs->cmpN(cstr_String(d), cstr, len);
}

iBool endsWithSc_String(const iString *d, const char *cstr, const iStringComparison *cs) {
    const size_t len = strlen(cstr);
    if (size_String(d) < len) return iFalse;
    return !cs->cmp(constEnd_Block(&d->chars) - len, cstr);
}

void set_String(iString *d, const iString *other) {
    set_Block(&d->chars, &other->chars);
}

void setCStr_String(iString *d, const char *cstr) {
    setCStr_Block(&d->chars, cstr);
}

void format_String(iString *d, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_Block(&d->chars, format, args);
    va_end(args);
}

size_t indexOf_String(const iString *d, iChar ch) {
    iMultibyteChar mb;
    init_MultibyteChar(&mb, ch);
    return indexOfCStr_String(d, mb.bytes);
}

size_t indexOfCStr_String(const iString *d, const char *cstr) {
    const char *chars = constData_Block(&d->chars);
    const char *found = strstr(chars, cstr);
    if (found) {
        return found - chars;
    }
    return iInvalidPos;
}

void append_String(iString *d, const iString *other) {
    append_Block(&d->chars, &other->chars);
}

void appendCStr_String(iString *d, const char *cstr) {
    appendCStr_Block(&d->chars, cstr);
}

void appendRange_String(iString *d, const iRangecc *range) {
    appendData_Block(&d->chars, range->start, size_Range(range));
}

void prepend_String(iString *d, const iString *other) {
    iString pre;
    initCopy_String(&pre, other);
    append_String(&pre, d);
    set_String(d, &pre);
    deinit_String(&pre);
}

iBool nextSplit_Rangecc(const iRangecc *str, const char *separator, iRangecc *range) {
    iAssert(range->start == NULL || contains_Range(str, range->start));
    if (!range->start) {
        range->start = range->end = str->start;
    }
    else if (range->start == str->end) {
        return iFalse;
    }
    else {
        range->start = range->end + strlen(separator);
        if (range->start >= str->end) {
            return iFalse;
        }
    }
    const char *found = strstr(range->start, separator);
    range->end = (found && found < str->end? found : str->end);
    iAssert(range->start <= range->end);
    return iTrue;
}

iStringList *split_Rangecc(const iRangecc *d, const char *separator) {
    iStringList *parts = new_StringList();
    iRangecc range = iNullRange;
    while (nextSplit_Rangecc(d, separator, &range)) {
        pushBackRange_StringList(parts, &range);
    }
    return parts;
}

int toInt_String(const iString *d) {
    return atoi(cstr_String(d));
}

const char *skipSpace_CStr(const char *cstr) {
    while (*cstr && isspace(*cstr)) {
        cstr++;
    }
    return cstr;
}

const char *findAscii_Rangecc(const iRangecc *str, char ch) {
    const char *pos = strchr(str->start, ch);
    if (!pos || pos >= str->end) return NULL;
    return pos;
}

//---------------------------------------------------------------------------------------

static void decodeNextMultibyte_StringConstIterator_(iStringConstIterator *d) {
    const int rc = mbrtowc(&d->value, d->next, d->remaining, &d->mbs);
    if (rc > 0) {
        d->remaining -= rc;
        d->next += rc;
    }
    else {
        // Finished, invalid or incomplete.
        d->value = 0;
    }
}

static iBool decodePrecedingMultibyte_StringConstIterator_(iStringConstIterator *d) {
    if (!d->remaining) return iFalse;
    for (int i = 1; i <= iMin(MB_LEN_MAX, d->remaining); i++) {
        const int rc = mbrtowc(&d->value, d->next - i, i, &d->mbs);
        if (rc >= 0) {
            // Single-byte character.
            d->remaining -= rc;
            d->next -= rc;
            break;
        }
    }
    return iTrue;
}

void init_StringConstIterator(iStringConstIterator *d, const iString *str) {
    d->str = str;
    d->value = 0;
    d->pos = d->next = constData_Block(&str->chars);
    d->remaining = size_Block(&str->chars);
    iZap(d->mbs);
    // Decode the first character.
    decodeNextMultibyte_StringConstIterator_(d);
}

void next_StringConstIterator(iStringConstIterator *d) {
    d->pos = d->next;
    decodeNextMultibyte_StringConstIterator_(d);
}

void init_StringReverseConstIterator(iStringConstIterator *d, const iString *str) {
    d->str = str;
    d->value = 0;
    d->pos = d->next = constEnd_Block(&str->chars);
    d->remaining = size_Block(&str->chars);
    iZap(d->mbs);
    // Decode the first (last) character.
    decodePrecedingMultibyte_StringConstIterator_(d);
}

void next_StringReverseConstIterator(iStringConstIterator *d) {
    d->pos = d->next;
    if (!decodePrecedingMultibyte_StringConstIterator_(d)) {
        d->value = 0;
    }
}

//---------------------------------------------------------------------------------------

void init_MultibyteChar(iMultibyteChar *d, iChar ch) {
    mbstate_t mbs;
    iZap(mbs);
    const size_t count = wcrtomb(d->bytes, ch, &mbs);
    if (count != iInvalidSize) {
        d->bytes[count] = 0;
    }
    else {
        d->bytes[0] = 0;
    }
}

int iCmpStrRange(const iRangecc *range, const char *cstr) {
    const size_t clen = strlen(cstr);
    const int cmp = iCmpStrN(range->start, cstr, size_Range(range));
    if (clen == size_Range(range)) {
        return cmp;
    }
    if (cmp == 0) return (size_Range(range) < clen? -1 : 1);
    return cmp;
}

int iCmpStrCase(const char *a, const char *b) {
    return strcasecmp(a, b);
}

int iCmpStrNCase(const char *a, const char *b, size_t len) {
    return strncasecmp(a, b, len);
}

iStringComparison iCaseSensitive = {
    .cmp  = strcmp,
    .cmpN = strncmp,
};

iStringComparison iCaseInsensitive = {
    .cmp  = iCmpStrCase,
    .cmpN = iCmpStrNCase,
};
