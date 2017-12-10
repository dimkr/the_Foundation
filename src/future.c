/** @file future.c  Future value.

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

#include "c_plus/future.h"
#include "c_plus/threadpool.h"

iDefineClass(Future)
iDefineObjectConstruction(Future)

void init_Future(iFuture *d) {
    initHandler_Future(d, NULL);
}

void initHandler_Future(iFuture *d, iFutureResultAvailable resultAvailable) {
    init_ObjectList(&d->threads);
    init_Mutex(&d->mutex);
    cnd_init(&d->ready);
    d->pendingCount = 0;
    d->resultAvailable = resultAvailable;
}

void deinit_Future(iFuture *d) {
    wait_Future(d);
    cnd_destroy(&d->ready);
    deinit_Mutex(&d->mutex);
    deinit_ObjectList(&d->threads);
}

static void threadFinished_Future_(iAny *any, iThread *thread) {
    iFuture *d = any;
    if (d->resultAvailable) {
        d->resultAvailable(d, thread);
    }
    iGuardMutex(&d->mutex, {
        d->pendingCount--;
        iAssert(d->pendingCount >= 0);
        cnd_signal(&d->ready);
    });
}

void add_Future(iFuture *d, iThread *thread) {
    iAssert(!isRunning_Thread(thread));
    iGuardMutex(&d->mutex, {
        d->pendingCount++;
        insert_Audience(finished_Thread(thread), d,
                        (iObserverFunc) threadFinished_Future_);
        pushBack_ObjectList(&d->threads, thread);
    });
}

void runTaken_Future(iFuture *d, iThread *thread_taken, iThreadPool *pool) {
    add_Future(d, thread_taken);
    run_ThreadPool(pool, thread_taken);
    iRelease(thread_taken);
}

iBool isReady_Future(const iFuture *d) {
    iBool ready = iFalse;
    iGuardMutex(&d->mutex, ready = (d->pendingCount == 0));
    return ready;
}

void wait_Future(iFuture *d) {
    iGuardMutex(&d->mutex, {
        while (d->pendingCount > 0) {
            cnd_wait(&d->ready, &d->mutex.mtx);
        }
    });
}

iBool isEmpty_Future(const iFuture *d) {
    iBool empty;
    iGuardMutex(&d->mutex, empty = isEmpty_ObjectList(&d->threads));
    return empty;
}

iThread *nextResult_Future(iFuture *d) {
    iThread *result = NULL;
    iGuardMutex(&d->mutex, {
        while (!isEmpty_ObjectList(&d->threads)) {
            // Check for a finished thread.
            iForEach(ObjectList, i, &d->threads) {
                iThread *thread = (iThread *) i.object;
                if (isFinished_Thread(thread)) {
                    result = ref_Object(thread);
                    remove_ObjectListIterator(&i);
                    break;
                }
            }
            if (result) break;
            cnd_wait(&d->ready, &d->mutex.mtx);
        }
    });
    return result;
}
