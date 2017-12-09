/** @file c_plus/threadpool.h  Thread pool.

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

#include "c_plus/threadpool.h"

iDeclareClass(PooledThread)
iDeclareType(PooledThread)

struct Impl_PooledThread {
    iThread thread;
    iThreadPool *pool;
};

static int run_PooledThread_(iThread *thread) {
    iPooledThread *d = (iAny *) thread;
    for (;;) {
        iThread *job = (iAny *) take_Queue(&d->pool->queue);
        if (job == (void *) d->pool) break; // Terminated.
        // Run the job in this thread.
        job->result = job->run(job);
        iRelease(job);
    }
    return 0;
}

static void init_PooledThread(iPooledThread *d, iThreadPool *pool) {
    init_Thread(&d->thread, run_PooledThread_);
    d->pool = pool;
}

static void deinit_PooledThread(iPooledThread *d) {
    deinit_Thread(&d->thread);
}

iDefineClass(PooledThread)
iDefineObjectConstructionArgs(PooledThread, (iThreadPool *pool), pool)

static inline void start_PooledThread(iPooledThread *d) { start_Thread(&d->thread); }
static inline void join_PooledThread (iPooledThread *d) { join_Thread(&d->thread); }

//---------------------------------------------------------------------------------------

iDefineClass(ThreadPool)
iDefineObjectConstruction(ThreadPool)

static void startThreads_ThreadPool_(iThreadPool *d) {
    const int count = idealConcurrentCount_Thread();
    for (int i = 0; i < count; ++i) {
        iPooledThread *pt = new_PooledThread(d);
        pushBack_ObjectList(d->threads, pt);
        start_PooledThread(pt);
        iRelease(pt);
    }
}

static void stopThreads_ThreadPool_(iThreadPool *d) {
    for (size_t count = size_ObjectList(d->threads); count; count--) {
        put_Queue(&d->queue, d);
    }
    iForEach(ObjectList, i, d->threads) {
        join_PooledThread((iPooledThread *) i.value->object);
        remove_ObjectListIterator(&i);
    }
}

void init_ThreadPool(iThreadPool *d) {
    init_Queue(&d->queue);
    d->threads = new_ObjectList();
    startThreads_ThreadPool_(d);
}

void deinit_ThreadPool(iThreadPool *d) {
    stopThreads_ThreadPool_(d);
    iRelease(d->threads);
    deinit_Queue(&d->queue);
}

void run_ThreadPool(iThreadPool *d, iThread *thread) {
    if (thread) {
        put_Queue(&d->queue, thread);
    }
}