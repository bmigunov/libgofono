/*
 * Copyright (C) 2014-2020 Jolla Ltd.
 * Copyright (C) 2014-2020 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GOFONO_UTIL_H
#define GOFONO_UTIL_H

#include "gofono_types.h"

#include <gutil_macros.h>

#define OFONO_NAME_INT_MAP_ENTRIES(a) a, G_N_ELEMENTS(a)

G_BEGIN_DECLS

typedef struct ofono_name_int_pair {
    const char* name;
    int value;
} OfonoNameIntPair;

typedef struct ofono_name_int_map {
    const char* name;
    const OfonoNameIntPair* entries;
    unsigned int count;
    OfonoNameIntPair defaults;
} OfonoNameIntMap;

int
ofono_name_to_int(
    const OfonoNameIntMap* map,
    const char* name);

const char*
ofono_int_to_name(
    const OfonoNameIntMap* map,
    int value);

const char*
ofono_country_code(
    const char* mcc,
    const char* mnc);

void
ofono_idle_pool_drain(void);

G_END_DECLS

#endif /* GOFONO_UTIL_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
