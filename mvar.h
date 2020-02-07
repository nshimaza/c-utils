#ifndef MVAR_H
#define MVAR_H
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

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

typedef void (*write_callback)(void* const mvar_context, const void* const user_data);
typedef void (*read_callback)(void* const mvar_context, void* const out_user_data);

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t putCond;
    pthread_cond_t takeCond;
    bool empty;
    write_callback write;
    read_callback read;
} MVar_abs;

void initMVar(void* const out_mvar, write_callback put, read_callback read);
bool isEmptyMVar(const void* const mvar);
int putMVar(void* const mvar, const void* const user_data);
int readMVar(void* const mvar, void* const out_user_data);
int takeMVar(void* const mvar, void* const out_user_data);
int timedPutMVar(void* const mvar, const long int timeout_in_msec, const void* const user_data);
int timedReadMVar(void* const mvar, const long int timeout_in_msec, void* const out_user_data);
int timedTakeMVar(void* const mvar, const long int timeout_in_msec, void* const out_user_data);
int tryPutMVar(void* mvar, const void* const user_data);
int tryReadMVar(void* const mvar, void* const out_user_data);
int tryTakeMVar(void* const mvar, void* const out_user_data);

#endif