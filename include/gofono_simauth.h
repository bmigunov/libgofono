/*
 * Copyright (C) 2021 Jolla Ltd.
 * Copyright (C) 2021 Slava Monich <slava.monich@jolla.com>
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

#ifndef GOFONO_SIMAUTH_H
#define GOFONO_SIMAUTH_H

#include "gofono_modemintf.h"

/* This file exists since 2.1.0 */

G_BEGIN_DECLS

typedef struct ofono_simauth_priv OfonoSimAuthPriv;

struct ofono_simauth {
    OfonoModemInterface intf;
    OfonoSimAuthPriv* priv;
    OfonoSimAppUSim* usim;
    OfonoSimAppISim* isim;
    const char* identity;               /* NetworkAccessIdentity */
};

GType ofono_simauth_get_type(void);
#define OFONO_TYPE_SIMAUTH (ofono_simauth_get_type())
#define OFONO_SIMAUTH(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
        OFONO_TYPE_SIMAUTH, OfonoSimAuth))

typedef
void
(*OfonoSimAuthHandler)(
    OfonoSimAuth* sender,
    void* arg);

OfonoSimAuth*
ofono_simauth_new(
    const char* path);

OfonoSimAuth*
ofono_simauth_ref(
    OfonoSimAuth* auth);

void
ofono_simauth_unref(
    OfonoSimAuth* auth);

GPtrArray*
ofono_simauth_get_apps(
    OfonoSimAuth* auth);

gulong
ofono_simauth_add_valid_changed_handler(
    OfonoSimAuth* auth,
    OfonoSimAuthHandler handler,
    void* arg); /* Since 2.1.1 */

void
ofono_simauth_remove_handler(
    OfonoSimAuth* auth,
    gulong id);

/* Inline wrappers */

OFONO_INLINE OfonoObject*
ofono_simauth_object(OfonoSimAuth* auth)
    { return G_LIKELY(auth) ? &auth->intf.object : NULL; }

OFONO_INLINE const char*
ofono_simauth_path(OfonoSimAuth* auth)
    { return G_LIKELY(auth) ? auth->intf.object.path : NULL; }

OFONO_INLINE gboolean
ofono_simauth_valid(OfonoSimAuth* auth)
    { return G_LIKELY(auth) && auth->intf.object.valid; }

/* Macros */

#define ofono_simauth_wait_valid(auth, msec, error) \
    ofono_object_wait_valid(ofono_simauth_object(auth), msec, error)

#define ofono_simauth_remove_handlers(auth, ids, n) \
    ofono_object_remove_handlers(ofono_simauth_object(auth), ids, n)

#define ofono_simauth_remove_all_handlers(auth, ids) \
    ofono_simauth_remove_handlers(auth, ids, G_N_ELEMENTS(ids))

G_END_DECLS

#endif /* GOFONO_SIMAUTH_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
