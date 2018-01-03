/** @file address.c  Network address.

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

#include "c_plus/address.h"
#include "c_plus/mutex.h"
#include "c_plus/string.h"
#include "c_plus/thread.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

struct Impl_Address {
    iObject object;
    iMutex mutex;
    iString hostName;
    iString service;
    iThread *pending;
    int count;
    struct addrinfo *info;
    iAudience *lookupFinished;
};

iDefineAudienceGetter(Address, lookupFinished)

static iThreadResult runLookup_Address_(iThread *thd) {
    iAddress *d = userData_Thread(thd);
    const struct addrinfo hints = {
        .ai_family   = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM
    };
    int rc = getaddrinfo(cstr_String(&d->hostName),
                         !isEmpty_String(&d->service)? cstr_String(&d->service) : NULL,
                         &hints,
                         &d->info);
    iGuardMutex(&d->mutex,
        if (rc == 0) {
            const struct addrinfo *at = d->info;
            for (d->count = 0; at; at = at->ai_next, d->count++) {}
        }
        else {
            iWarning("[Address] host lookup failed with error: %s\n", gai_strerror(rc));
        }
        iReleasePtr(&d->pending); // get rid of this thread
    );
    iNotifyAudience(d, lookupFinished, AddressLookupFinished);
    return 0;
}

iDefineObjectConstruction(Address)

iAddress *newSockAddr_Address(const void *sockAddr, size_t sockAddrSize) {
    iAddress *d = iNew(Address);
    init_Address(d);
    d->count = 1;
    d->info = calloc(1, sizeof(struct addrinfo));
    d->info->ai_addrlen = (socklen_t) sockAddrSize;
    d->info->ai_addr = malloc(sockAddrSize);
    d->info->ai_socktype = SOCK_STREAM;
    memcpy(d->info->ai_addr, sockAddr, sockAddrSize);
    return d;
}

void init_Address(iAddress *d) {
    init_Mutex(&d->mutex);
    init_String(&d->hostName);
    init_String(&d->service);
    d->pending = NULL;
    d->info = NULL;
    d->count = -1;
    d->lookupFinished = NULL;
}

void deinit_Address(iAddress *d) {
    if (d->pending) {
        join_Thread(d->pending);
    }
    if (d->info) freeaddrinfo(d->info);
    deinit_String(&d->service);
    deinit_String(&d->hostName);
    deinit_Mutex(&d->mutex);
    delete_Audience(d->lookupFinished);
}

const iString *hostName_Address(const iAddress *d) {
    return &d->hostName;
}

int count_Address(const iAddress *d) {
    int count;
    iGuardMutex(&d->mutex, count = d->count);
    return count;
}

iSocketParameters socketParameters_Address(const iAddress *d) {
    iSocketParameters sp = { .family = 0 };
    iGuardMutex(&d->mutex, {
        if (d->info) {
            sp.family   = d->info->ai_family;
            sp.type     = d->info->ai_socktype;
            sp.protocol = d->info->ai_protocol;
        }
    });
    return sp;
}

int protocol_Address(const iAddress *d) {
    int ver = 0;
    iGuardMutex(&d->mutex, {
        if (d->info) {
            ver = (d->info->ai_family == AF_INET6? 6 : 4);
        }
    });
    return ver;
}

iBool isValid_Address(const iAddress *d) {
    return count_Address(d) >= 0;
}

iBool isHostFound_Address(const iAddress *d) {
    return count_Address(d) > 0;
}

iBool isPending_Address(const iAddress *d) {
    return d->pending != NULL;
}

void lookupHostCStr_Address(iAddress *d, const char *hostName, uint16_t port) {
    iGuardMutex(&d->mutex, {
        if (!d->pending) {
            setCStr_String(&d->hostName, hostName);
            if (port) {
                format_String(&d->service, "%i", port);
            }
            else {
                clear_String(&d->service);
            }
            if (d->info) {
                freeaddrinfo(d->info);
                d->info = NULL;
            }
            d->count = -1;
            d->pending = new_Thread(runLookup_Address_);
            setUserData_Thread(d->pending, d);
            start_Thread(d->pending);
        }
    });
}

void waitForFinished_Address(const iAddress *d) {
    // Prevent the thread from being deleted while we're checking.
    guardJoin_Thread(d->pending, &d->mutex);
}

#if 0
static const iAny *inAddr_addrinfo_(const struct addrinfo *d) {
    if (d->ai_family == AF_INET) { // IPv4
        return &((const struct sockaddr_in *) d->ai_addr)->sin_addr;
    }
    // IPv6
    return &((const struct sockaddr_in6 *) d->ai_addr)->sin6_addr;
}
#endif

void getSockAddr_Address(const iAddress *d, struct sockaddr **addr_out, socklen_t *addrSize_out) {
    iGuardMutex(&d->mutex, {
        if (!d->info) {
            *addr_out = NULL;
            *addrSize_out = 0;
        }
        else {
            *addr_out = d->info->ai_addr;
            *addrSize_out = d->info->ai_addrlen;
        }
    });
}

static size_t sockAddrSize_addrinfo_(const struct addrinfo *d) {
    if (d->ai_family == AF_INET) {
        return sizeof(struct sockaddr_in);
    }
    return sizeof(struct sockaddr_in6);
}

iString *toString_Address(const iAddress *d) {
    iString *str = new_String();
    iGuardMutex(&d->mutex, {
        if (d->info) {
            char hbuf[NI_MAXHOST];
            char sbuf[NI_MAXSERV];
            if (!getnameinfo(d->info->ai_addr,
                             sockAddrSize_addrinfo_(d->info),
                             hbuf, sizeof(hbuf),
                             sbuf, sizeof(sbuf),
                             NI_NUMERICHOST | NI_NUMERICSERV)) {
                if (iCmpStr(sbuf, "0")) {
                    format_String(str, "%s port:%s", hbuf, sbuf);
                }
                else {
                    setCStr_String(str, hbuf);
                }
            }
        }
    });
    return str;
}

iDefineClass(Address)