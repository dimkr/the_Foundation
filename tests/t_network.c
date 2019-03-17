/**
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

#include <the_Foundation/address.h>
#include <the_Foundation/audience.h>
#include <the_Foundation/commandline.h>
#include <the_Foundation/string.h>
#include <the_Foundation/objectlist.h>
#include <the_Foundation/service.h>
#include <the_Foundation/socket.h>
#include <the_Foundation/thread.h>
#if defined (iHaveCurl)
#  include <the_Foundation/webrequest.h>
#endif

static void logConnected_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: connected\n", sock);
}

static void logDisonnected_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: disconnected\n", sock);
}

static void logWriteFinished_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: write finished\n", sock);
}

static void logReadyRead_(iAny *d, iSocket *sock) {
    iUnused(d);
    printf("Socket %p: data ready for reading (%zu bytes)\n", sock, receivedBytes_Socket(sock));
}

static void logError_(iAny *d, iSocket *sock, int code, const char *msg) {
    iUnused(d);
    printf("Socket %p: error %i: %s\n", sock, code, msg);
}

static void logWebRequestProgress_(iAny *d, iWebRequest *web, size_t currentSize, size_t totalSize) {
    iUnused(d);
    printf("WebRequest %p: downloaded %zu/%zu bytes\n", web, currentSize, totalSize);
}

static void observeSocket_(iSocket *sock) {
    iConnect(Socket, sock, connected, sock, logConnected_);
    iConnect(Socket, sock, disconnected, sock, logDisonnected_);
    iConnect(Socket, sock, error, sock, logError_);
    iConnect(Socket, sock, readyRead, sock, logReadyRead_);
    iConnect(Socket, sock, writeFinished, sock, logWriteFinished_);
}

static void hostLookedUp(iAny *d, const iAddress *address) {
    iUnused(d);
    if (isValid_Address(address)) {
        iString *ip = toString_Address(address);
        printf("%i IP addresses for %s; chosen: %s \n",
               count_Address(address),
               cstr_String(hostName_Address(address)),
               cstr_String(ip));
        delete_String(ip);
    }
}

static void printMessage_(iAny *any, iSocket *sock) {
    iUnused(any);
    iBlock *data = readAll_Socket(sock);
    printf("%s", constData_Block(data));
    delete_Block(data);
}

static iThreadResult messageReceiver_(iThread *thread) {
    iSocket *sock = userData_Thread(thread);
    iConnect(Socket, sock, readyRead, sock, printMessage_);
    printMessage_(NULL, sock);
    while (isOpen_Socket(sock)) {
        sleep_Thread(0.1);
    }
    iRelease(sock);
    iRelease(thread);
    return 0;
}

static void communicate_(iAny *d, iService *sv, iSocket *sock) {
    iUnused(d, sv);
    iString *addr = toString_Address(address_Socket(sock));
    printf("incoming connecting from %s\n", cstr_String(addr));
    delete_String(addr);
    // Start a new thread to communicate through the socket.
    iThread *receiver = new_Thread(messageReceiver_);
    setUserData_Thread(receiver, ref_Object(sock));
    observeSocket_(sock);
    start_Thread(receiver);
}

int main(int argc, char *argv[]) {
    init_Foundation();
    /* List network interface addresses. */ {
        iObjectList *ifs = networkInterfaces_Address();
        printf("%zu network interfaces:\n", size_ObjectList(ifs));
        iConstForEach(ObjectList, i, ifs) {
            iString *str = toString_Address((const iAddress *) i.object);
            printf("- %s\n", cstr_String(str));
            delete_String(str);
        }
        iRelease(ifs);
    }
    // Check the arguments.
    iCommandLine *cmdline = iClob(new_CommandLine(argc, argv)); {
#if defined (iHaveCurl)
        iCommandLineArg *getUrl = iClob(checkArgumentValues_CommandLine(cmdline, "g", 1));
        if (getUrl) {
            iWebRequest *web = iClob(new_WebRequest());
            setUrl_WebRequest(web, value_CommandLineArg(getUrl, 0));
            iConnect(WebRequest, web, progress, web, logWebRequestProgress_);
            printf("Getting URL \"%s\"...\n", cstr_String(value_CommandLineArg(getUrl, 0)));
            iBool ok = get_WebRequest(web);
            if (ok) {
                printf("Success! Received %zu bytes\n", size_Block(result_WebRequest(web)));
                iString *ctype = collect_String(new_String());
                iString *clen = collect_String(new_String());
                headerValue_WebRequest(web, "Content-Type:", ctype);
                headerValue_WebRequest(web, "Content-Length:", clen);
                printf("Content type: %s\n", cstr_String(ctype));
                if (!isEmpty_String(clen)) {
                    printf("Content length according to header: %s bytes\n", cstr_String(clen));
                }
                else {
                    puts("Content length omitted");
                }
            }
            else {
                printf("Failure! CURL says: %s\n", cstr_String(errorMessage_WebRequest(web)));
            }
            return 0;
        }
#endif
    }
    if (contains_CommandLine(cmdline, "s;server")) {
        iService *sv = iClob(new_Service(14666));
        iConnect(Service, sv, incomingAccepted, sv, communicate_);
        if (!open_Service(sv)) {
            puts("Failed to start service");
            return 1;
        }
        puts("Press Enter to quit..."); {
            char line[2];
            if (!fgets(line, sizeof(line), stdin)) {
                iWarning("fgets failed\n");
            }
        }
    }
    else if (contains_CommandLine(cmdline, "c;client")) {
        iSocket *sock = iClob(new_Socket("localhost", 14666));
        observeSocket_(sock);
        if (!open_Socket(sock)) {
            puts("Failed to connect");
            return 1;
        }
        puts("Type to send a message (empty to quit):");
        for (;;) {
            char buf[200];
            if (!fgets(buf, sizeof(buf), stdin)) {
                break;
            }
            if (strlen(buf) <= 1) break;
            writeData_Socket(sock, buf, strlen(buf));
        }
        puts("Good day!");
    }
    else {
        iConstForEach(CommandLine, i, cmdline) {
            if (i.argType == value_CommandLineArgType) {
                printf("\nLooking up \"%s\"...\n", cstr_String(value_CommandLineConstIterator(&i)));
                iAddress *addr = new_Address();
                iConnect(Address, addr, lookupFinished, addr, hostLookedUp);
                lookupTcp_Address(addr, value_CommandLineConstIterator(&i), 0);
                iRelease(addr);
            }
        }
    }
    return 0;
}