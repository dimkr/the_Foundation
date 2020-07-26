#pragma once

/** @file the_Foundation/tlsrequest.h  TLS request over a Socket (using OpenSSL)

@authors Copyright (c) 2020 Jaakko Keränen <jaakko.keranen@iki.fi>

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

#include "audience.h"
#include "object.h"
#include "string.h"

iDeclareType(TlsRequest)
iDeclareClass(TlsRequest)
iDeclareObjectConstruction(TlsRequest)

iDeclareNotifyFunc    (TlsRequest, ReadyRead)
iDeclareNotifyFunc    (TlsRequest, Finished)

iDeclareAudienceGetter(TlsRequest, readyRead)
iDeclareAudienceGetter(TlsRequest, finished)

enum iTlsRequestStatus {
    initialized_TlsRequestStatus,
    submitted_TlsRequestStatus,
    finished_TlsRequestStatus,
    error_TlsRequestStatus
};

void    setUrl_TlsRequest           (iTlsRequest *, const iString *hostName, uint16_t port);
void    setContent_TlsRequest       (iTlsRequest *, const iBlock *content);

void    submit_TlsRequest           (iTlsRequest *);
void    cancel_TlsRequest           (iTlsRequest *);
void    waitForFinished_TlsRequest  (iTlsRequest *);
iBlock *readAll_TlsRequest          (iTlsRequest *);

size_t                  receivedBytes_TlsRequest    (const iTlsRequest *);
enum iTlsRequestStatus  status_TlsRequest           (const iTlsRequest *);