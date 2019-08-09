/** @file path.c  File path manipulation.

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

#include "the_Foundation/path.h"
#include "the_Foundation/string.h"

#include <unistd.h>
#include <unicase.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined (iPlatformWindows)
#   include <direct.h>
#   define mkdir(path, attr) _mkdir(path)
#endif

#if defined (iPlatformCygwin)
#   include <sys/cygwin.h>
#endif

iString *cwd_Path(void) {
    char *cwd = getcwd(NULL, 0);
#if defined (iPlatformCygwin)
    iString *str = unixToWindows_Path(cwd);
    free(cwd);
    return str;
#else
    if (cwd) {
        iBlock block;
        const size_t len = strlen(cwd);
        initPrealloc_Block(&block, cwd, len, len + 1);
        iString *d = newBlock_String(&block);
        deinit_Block(&block);
        return d;
    }
    return new_String();
#endif
}

iBool setCwd_Path(const iString *path) {
    return !chdir(cstr_String(path));
}

iBool isAbsolute_Path(const iString *d) {
#if !defined (iPlatformWindows)
    if (startsWith_String(d, "~")) {
        return iTrue;
    }
#endif
    if (startsWith_String(d, iPathSeparator)) {
        return iTrue;
    }
#if defined (iPlatformWindows) || defined (iPlatformCygwin)
    /* Check for drive letters. */
    if (size_String(d) >= 3) {
        iStringConstIterator i;
        init_StringConstIterator(&i, d);
        const iChar drive = uc_toupper(i.value);
        if (drive < 'A' || drive > 'Z') {
            return iFalse;
        }
        next_StringConstIterator(&i);
        if (i.value != ':') {
            return iFalse;
        }
        next_StringConstIterator(&i);
        return i.value == '\\' || i.value == '/';
    }
#endif
    return iFalse;
}

iString *makeAbsolute_Path(const iString *d) {
    iString *abs;
    iString *path = copy_String(d);
    clean_Path(path);
    if (isAbsolute_Path(path)) {
        abs = copy_String(path);
    }
    else {
        abs = cwd_Path();
        append_Path(abs, path);
        clean_Path(abs);
    }
    delete_String(path);
    return abs;
}

#define iPathMaxSegments 128

static iBool splitSegments_Path_(const iRangecc *path, iRangecc *segments,
                                 size_t *count, iBool *changed) {
    iRangecc seg = iNullRange;
    while (nextSplit_Rangecc(path, iPathSeparator, &seg)) {
        if (*count > 0 && size_Range(&seg) == 0) {
            /* Skip repeated slashes. */
            *changed = iTrue;
            continue;
        }
#if !defined (iPlatformWindows)
        if (*count == 0 && !iCmpStrRange(&seg, "~")) {
            const char *home = getenv("HOME");
            if (home && iCmpStrN(home, "~", 1)) {
                if (!splitSegments_Path_(&(iRangecc){ home, home + strlen(home) },
                                         segments, count, changed)) {
                    return iFalse;
                }
                *changed = iTrue;
                continue;
            }
        }
#endif
        if (!iCmpStrRange(&seg, ".")) {
            *changed = iTrue;
            continue; // No change in directory.
        }
        if (!iCmpStrRange(&seg, "..")) {
            if (*count > 0 && iCmpStrRange(&segments[*count - 1], "..")) {
                (*count)--; // Go up a directory.
                *changed = iTrue;
                continue;
            }
        }
        if (*count == iPathMaxSegments) {
            iAssert(*count < iPathMaxSegments);
            return iFalse; // Couldn't clean it.
        }
        segments[(*count)++] = seg;
    }
    return iTrue;
}

void clean_Path(iString *d) {
    if (isEmpty_String(d)) return;
#if defined (iPlatformWindows) || defined (iPlatformCygwin)
    /* Use the correct separators. */
    replace_Block(&d->chars, '/', '\\');
#endif
    iRangecc segments[iPathMaxSegments];
    size_t count = 0;
    iBool changed = iFalse;
    splitSegments_Path_(&range_String(d), segments, &count, &changed);
    /* Recompose the remaining segments. */
    if (changed) {
        if (count == 0) {
            set_String(d, &iStringLiteral("."));
            return;
        }
        iString cleaned;
        init_String(&cleaned);
        for (size_t i = 0; i < count; ++i) {
            if (i != 0 || (isAbsolute_Path(d)
#if defined (iPlatformWindows) || defined (iPlatformCygwin)
                && startsWith_String(d, iPathSeparator)
#endif
                    )) {
                appendCStr_String(&cleaned, iPathSeparator);
            }
            appendRange_String(&cleaned, segments + i);
        }
        set_String(d, &cleaned);
        deinit_String(&cleaned);
    }
}

void append_Path(iString *d, const iString *path) {
    if (isAbsolute_Path(path)) {
        set_String(d, path);
    }
    else {
        if (!endsWith_String(d, iPathSeparator)) {
            appendCStr_String(d, iPathSeparator);
        }
        append_String(d, path);
    }
}

iString *concat_Path(const iString *d, const iString *path) {
    iString *base = copy_String(d);
    append_Path(base, path);
    return base;
}

iBool mkdir_Path(const iString *path) {
    return mkdir(cstr_String(path), 0755) == 0;
}

iBool rmdir_Path(const iString *path) {
    return rmdir(cstr_String(path)) == 0;
}

#if defined (iPlatformCygwin)
iString *unixToWindows_Path(const char *cstr) {
    uint16_t *winPath = cygwin_create_path(CCP_POSIX_TO_WIN_W, cstr);
    if (winPath) {
        iString *str = newUtf16_String(winPath);
        free(winPath);
        return str;
    }
    return new_String();
}
#endif
