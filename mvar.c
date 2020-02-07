/*
 * MIT License
 *
 * Copyright (c) 2020 Naoto Shimazaki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * MVar is one element only thread safe queue.
 * This is MVar implementation in C and pthread.
 */

#include <errno.h>
#include <time.h>

#include "mvar.h"

void
initMVar(void* const out_mvar, write_callback write, read_callback read) {
    MVar_abs* const v = out_mvar;
    pthread_mutex_init(&v->lock, NULL);
    pthread_cond_init(&v->putCond, NULL);
    pthread_cond_init(&v->takeCond, NULL);
    v->empty = true;
    v->write = write;
    v->read = read;
}

bool
isEmptyMVar(const void* const mvar) {
    return ((const MVar_abs*) mvar)->empty;
}

int
putMVar(void* const mvar, const void* const user_data) {
    MVar_abs* const v = mvar;
    int err = pthread_mutex_lock(&v->lock);
    if (err != 0) {
        return err;
    }
    if (!v->empty) {
        pthread_cond_wait(&v->putCond, &v->lock);
    }
    v->write(v, user_data);
    v->empty = false;
    pthread_cond_signal(&v->takeCond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
readMVar(void* const mvar, void* const out_user_data)
{
    MVar_abs* const v = mvar;
    int err = pthread_mutex_lock(&v->lock);
    if (err != 0) {
        return err;
    }
    if (v->empty) {
        pthread_cond_wait(&v->takeCond, &v->lock);
    }
    v->read(v, out_user_data);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
takeMVar(void* const mvar, void* const out_user_data)
{
    MVar_abs* const v = mvar;
    int err = pthread_mutex_lock(&v->lock);
    if (err != 0) {
        return err;
    }
    if (v->empty) {
        pthread_cond_wait(&v->takeCond, &v->lock);
    }
    v->read(v, out_user_data);
    v->empty = true;
    pthread_cond_signal(&v->putCond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

struct timespec
timespec_add(struct timespec already_normalized_timesec, long int nano_second) {
    already_normalized_timesec.tv_nsec += nano_second;
    already_normalized_timesec.tv_sec += already_normalized_timesec.tv_nsec / 1000000000;
    already_normalized_timesec.tv_nsec %= 1000000000;
    return already_normalized_timesec;
}

int
timedPutMVar(void* const mvar, const long int timeout_in_msec, const void* const user_data) {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    struct timespec tout = timespec_add(now, timeout_in_msec * 1000);
    MVar_abs* const v = mvar;
    int err = pthread_mutex_timedlock(&v->lock, &tout);
    if (err != 0) {
        // When timer expired, ETIMEDOUT is returned.
        return err;
    }
    if (!v->empty) {
        err = pthread_cond_timedwait(&v->putCond, &v->lock, &tout);
        if (!v->empty) {
            pthread_mutex_unlock(&v->lock);
            return err;
        }
    }
    v->write(v, user_data);
    v->empty = false;
    pthread_cond_signal(&v->takeCond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
timedReadMVar(void* const mvar, const long int timeout_in_msec, void* const out_user_data) {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    struct timespec tout = timespec_add(now, timeout_in_msec * 1000);
    MVar_abs* const v = mvar;
    int err = pthread_mutex_timedlock(&v->lock, &tout);
    if (err != 0) {
        // When timer expired, ETIMEDOUT is returned.
        return err;
    }
    if (v->empty) {
        err = pthread_cond_timedwait(&v->takeCond, &v->lock, &tout);
        if (v->empty) {
            pthread_mutex_unlock(&v->lock);
            return err;
        }
    }
    v->read(v, out_user_data);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
timedTakeMVar(void* const mvar, const long int timeout_in_msec, void* const out_user_data) {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    struct timespec tout = timespec_add(now, timeout_in_msec * 1000);
    MVar_abs* const v = mvar;
    int err = pthread_mutex_timedlock(&v->lock, &tout);
    if (err != 0) {
        // When timer expired, ETIMEDOUT is returned.
        return err;
    }
    if (v->empty) {
        err = pthread_cond_timedwait(&v->takeCond, &v->lock, &tout);
        if (v->empty) {
            pthread_mutex_unlock(&v->lock);
            return err;
        }
    }
    v->read(v, out_user_data);
    v->empty = true;
    pthread_cond_signal(&v->putCond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
tryPutMVar(void* mvar, const void* const user_data)
{
    MVar_abs* const v = mvar;
    int err = pthread_mutex_trylock(&v->lock);
    if (err != 0) {
        // return EBUSY if v->lock is already locked.
        return err;
    }
    if (!v->empty) {
        // MVar is not empty.  Return without waiting.
        pthread_mutex_unlock(&v->lock);
        return EBUSY;
    }
    v->write(v, user_data);
    v->empty = false;
    pthread_cond_signal(&v->takeCond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
tryReadMVar(void* const mvar, void* const out_user_data)
{
    MVar_abs* const v = mvar;
    int err = pthread_mutex_trylock(&v->lock);
    if (err != 0) {
        // return EBUSY if v->lock is already locked.
        return err;
    }
    if (v->empty) {
        // MVar is empty.  Return without waiting.
        pthread_mutex_unlock(&v->lock);
        return EBUSY;
    }
    v->read(v, out_user_data);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
tryTakeMVar(void* const mvar, void* const out_user_data)
{
    MVar_abs* const v = mvar;
    int err = pthread_mutex_trylock(&v->lock);
    if (err != 0) {
        // return EBUSY if v->lock is already locked.
        return err;
    }
    if (v->empty) {
        // MVar is empty.  Return without waiting.
        pthread_mutex_unlock(&v->lock);
        return EBUSY;
    }
    v->read(v, out_user_data);
    v->empty = true;
    pthread_cond_signal(&v->putCond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}
