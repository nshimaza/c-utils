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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

size_t
hexdump(char* const out_str, const size_t max_len, const uint8_t* const src, const size_t src_len) {
    if (max_len == 0) {
        return 0;
    }

    char* outp = out_str;
    const char* const out_str_fence = out_str + max_len;
    const char* const outp_fence = out_str + max_len - 1;
    const uint8_t* inp = src;
    const uint8_t* const inp_fence = src + src_len;

    while (outp < outp_fence && inp < inp_fence) {
        const uint8_t* const line_head = inp;

        size_t room = outp_fence - outp;
        int needed = snprintf(outp, room, "%04lx  ", line_head - src);
        outp += room - 1 < needed ? room - 1 : needed;

        const uint8_t* const inp_block1_fence = line_head + 8;
        while (outp < outp_fence && inp < inp_fence && inp < inp_block1_fence) {
            size_t room = outp_fence - outp;
            int needed = snprintf(outp, room, "%02x ", *inp++);
            outp += room - 1 < needed ? room - 1 : needed;
        }
        while (outp < outp_fence && inp < inp_block1_fence) {
            size_t room = outp_fence - outp;
            int needed = snprintf(outp, room, "   ");
            outp += room - 1 < needed ? room - 1 : needed;
            inp++;
        }

        if (outp < outp_fence) {
            *outp++ = ' ';
        }

        const uint8_t* const inp_block2_fence = line_head + 16;
        while (outp < outp_fence && inp < inp_fence && inp < inp_block2_fence) {
            size_t room = outp_fence - outp;
            int needed = snprintf(outp, room, "%02x ", *inp++);
            outp += room - 1 < needed ? room - 1 : needed;
        }
        while (outp < outp_fence && inp < inp_block2_fence) {
            size_t room = outp_fence - outp;
            int needed = snprintf(outp, room, "   ");
            outp += room - 1 < needed ? room - 1 : needed;
            inp++;
        }

        if (outp < outp_fence) {
            *outp++ = ' ';
        }

        inp = line_head;
        while (outp < outp_fence && inp < inp_fence && inp < inp_block1_fence) {
            char c = *inp++;
            *outp++ = 0x20 <= c && c <= 0x7e ? c : '.';
        }
        while (outp < outp_fence && inp < inp_block1_fence) {
            *outp++ = ' ';
            inp++;
        }

        if (outp < outp_fence) {
            *outp++ = ' ';
        }

        while (outp < outp_fence && inp < inp_fence && inp < inp_block2_fence) {
            char c = *inp++;
            *outp++ = 0x20 <= c && c <= 0x7e ? c : '.';
        }
        while (outp < outp_fence && inp < inp_block2_fence) {
            *outp++ = ' ';
            inp++;
        }
        *outp++ = '\n';
    }
    *outp = '\0';
    return outp - out_str;
}
