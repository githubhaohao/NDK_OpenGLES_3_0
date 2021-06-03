//
// Created by ByteFlow on 2019/7/15.
//

#include "Looper.h"

#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "LogUtil.h"
struct LooperMessage;
typedef struct LooperMessage LooperMessage;

void* Looper::trampoline(void* p) {
    ((Looper*)p)->loop();
    return NULL;
}

Looper::Looper() {
    head = NULL;

    sem_init(&headDataAvailable, 0, 0);
    sem_init(&headWriteProtect, 0, 1);
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&worker, &attr, trampoline, this);
    running = true;
}

Looper::~Looper() {
    if (running) {
        LOGCATE("Looper deleted while still running. Some messages will not be processed");
        quit();
    }
}

void Looper::postMessage(int what, bool flush) {
    postMessage(what, 0, 0, NULL, flush);
}

void Looper::postMessage(int what, void *obj, bool flush) {
    postMessage(what, 0, 0, obj, flush);
}

void Looper::postMessage(int what, int arg1, int arg2, bool flush) {
    postMessage(what, arg1, arg2, NULL, flush);
}

void Looper::postMessage(int what, int arg1, int arg2, void *obj, bool flush) {
    LooperMessage *msg = new LooperMessage();
    msg->what = what;
    msg->obj = obj;
    msg->arg1 = arg1;
    msg->arg2 = arg2;
    msg->next = NULL;
    msg->quit = false;
    addMessage(msg, flush);
}

void Looper::addMessage(LooperMessage *msg, bool flush) {
    sem_wait(&headWriteProtect);
    LooperMessage *h = head;

    if (flush) {
        while(h) {
            LooperMessage *next = h->next;
            delete h;
            h = next;
        }
        h = NULL;
    }
    if (h) {
        while (h->next) {
            h = h->next;
        }
        h->next = msg;
    } else {
        head = msg;
    }
    LOGCATE("Looper::addMessage msg->what=%d", msg->what);
    sem_post(&headWriteProtect);
    sem_post(&headDataAvailable);
}

void Looper::loop() {
    while(true) {
        // wait for available message
        sem_wait(&headDataAvailable);

        // get next available message
        sem_wait(&headWriteProtect);
        LooperMessage *msg = head;
        if (msg == NULL) {
            LOGCATE("Looper::loop() no msg");
            sem_post(&headWriteProtect);
            continue;
        }
        head = msg->next;
        sem_post(&headWriteProtect);

        if (msg->quit) {
            LOGCATE("Looper::loop() quitting");
            delete msg;
            return;
        }
        LOGCATE("Looper::loop() processing msg.what=%d", msg->what);
        handleMessage(msg);
        delete msg;
    }
}

void Looper::quit() {
    LOGCATE("Looper::quit()");
    LooperMessage *msg = new LooperMessage();
    msg->what = 0;
    msg->obj = NULL;
    msg->next = NULL;
    msg->quit = true;
    addMessage(msg, false);
    void *retval;
    pthread_join(worker, &retval);
    sem_destroy(&headDataAvailable);
    sem_destroy(&headWriteProtect);
    running = false;
}

void Looper::handleMessage(LooperMessage *msg) {
    LOGCATE("Looper::handleMessage [what, obj]=[%d, %p]", msg->what, msg->obj);
}