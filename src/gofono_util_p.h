/*
 * Copyright (C) 2014-2015 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
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
 *   3. Neither the name of the Jolla Ltd nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
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

#ifndef GOFONO_UTIL_PRIVATE_H
#define GOFONO_UTIL_PRIVATE_H

#include "gofono_util.h"

typedef
void
(*OfonoConditionHandler)(
    GObject* object,
    void* arg);

typedef
gboolean
(*OfonoConditionCheck)(
    GObject* object);

typedef
gulong
(*OfonoConditionAddHandler)(
    GObject* object,
    OfonoConditionHandler handler,
    void* arg);

typedef
void
(*OfonoWaitConditionRemoveHandler)(
    GObject* object,
    gulong id);

gboolean
ofono_condition_wait(
    GObject* object,
    OfonoConditionCheck check,
    OfonoConditionAddHandler add_handler,
    OfonoWaitConditionRemoveHandler remove_handler,
    int timeout_msec,
    GError** error);

GPtrArray*
ofono_string_array_from_variant(
    GVariant* value);

GPtrArray*
ofono_string_array_sort(
    GPtrArray* strings);

gboolean
ofono_string_array_equal(
    GPtrArray* strings1,
    GPtrArray* strings2);

#endif /* GOFONO_UTIL_PRIVATE_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
