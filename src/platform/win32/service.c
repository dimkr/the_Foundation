/** @file win32/service.c  TCP server socket (Windows Sockets)

@authors Copyright (c) 2017-2023 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "the_Foundation/service.h"
#include "the_Foundation/socket.h"
#include "the_Foundation/string.h"
#include "the_Foundation/thread.h"
#include "pipe.h"
#include "wide.h"

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <ws2tcpip.h>

struct Impl_Service {
    iObject object;
    uint16_t port;
    SOCKET fd;
    iPipe stop;
    iThread *listening;
    iAudience *incomingAccepted;
};

iDefineAudienceGetter(Service, incomingAccepted)

iDefineObjectConstructionArgs(Service, (uint16_t port), port)

static iThreadResult listen_Service_(iThread *thd) {
    iService *d = userData_Thread(thd);
    while (d->fd >= 0) {
        /* Wait for activity. */
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(d->fd, &fds);
        FD_SET(output_Pipe(&d->stop), &fds);
        if (select(0, &fds, NULL, NULL, NULL) == -1) {
            break;
        }
        if (FD_ISSET(output_Pipe(&d->stop), &fds)) {
            break;
        }
        if (FD_ISSET(d->fd, &fds)) {
            struct sockaddr_storage addr;
            int size = sizeof(addr);
            int incoming = accept(d->fd, (struct sockaddr *) &addr, &size);
            if (incoming < 0) {
                iWarning("[Service] error on accept: %s\n",
                         errorMessage_Windows_(WSAGetLastError()));
                break;
            }
            iSocket *socket = newExisting_Socket(incoming, &addr, size);
            iNotifyAudienceArgs(d, incomingAccepted, ServiceIncomingAccepted, socket);
            iRelease(socket);
        }
    }
    iReleasePtr(&d->listening);
    return 0;
}

void init_Service(iService *d, uint16_t port) {
    d->port = port;
    d->fd = NULL;
    d->listening = NULL;
    init_Pipe(&d->stop);
    d->incomingAccepted = new_Audience();
}

void deinit_Service(iService *d) {
    close_Service(d);
    deinit_Pipe(&d->stop);
    iAssert(d->listening == NULL);
    iAssert(!d->fd);
    delete_Audience(d->incomingAccepted);
}

iBool isOpen_Service(const iService *d) {
    return d->fd >= 0;
}

iBool open_Service(iService *d) {
    if (isOpen_Service(d)) return iFalse;
    /* Set up the socket. */ {
        struct addrinfo *info, hints = {
            .ai_socktype = SOCK_STREAM,
            .ai_family   = AF_INET, /* listen on IPv4 */
            .ai_flags    = AI_PASSIVE,
        };
        iString *port = new_String();
        format_String(port, "%i", d->port);
        int rc = getaddrinfo(NULL, cstr_String(port), &hints, &info);
        delete_String(port);
        if (rc) {
            iWarning("[Service] failed to look up address: %s\n", gai_strerror(rc));
            return iFalse;
        }
        d->fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (d->fd < 0) {
            iWarning("[Service] failed to open socket: %s\n", errorMessage_Windows_(WSAGetLastError()));
            return iFalse;
        }
        rc = bind(d->fd, info->ai_addr, info->ai_addrlen);
        if (rc < 0) {
            closesocket(d->fd);
            d->fd = NULL;
            iWarning("[Service] failed to bind address: %s\n", errorMessage_Windows_(WSAGetLastError()));
            return iFalse;
        }
        rc = listen(d->fd, 10);
        if (rc < 0) {
            closesocket(d->fd);
            d->fd = NULL;
            iWarning("[Service] failed to listen: %s\n", errorMessage_Windows_(WSAGetLastError()));
            return iFalse;
        }
    }
    d->listening = new_Thread(listen_Service_);
    setUserData_Thread(d->listening, d);
    start_Thread(d->listening);
    return iTrue;
}

void close_Service(iService *d) {
    if (d->listening) {
        /* Signal the listening thread to stop. */
        writeByte_Pipe(&d->stop, 1);
        closesocket(d->fd);
        d->fd = NULL;
        join_Thread(d->listening);
        iAssert(d->listening == NULL);
    }
}

iDefineClass(Service)
