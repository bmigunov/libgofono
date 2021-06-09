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

#ifndef GOFONO_SIMAPP_H
#define GOFONO_SIMAPP_H

/* This file exists since 2.1.0 */

#include "gofono_object.h"

G_BEGIN_DECLS

typedef struct ofono_simapp_priv OfonoSimAppPriv;

typedef enum ofono_simapp_type {
    OFONO_SIMAPP_TYPE_UNKNOWN = -1,
    OFONO_SIMAPP_TYPE_NONE,
    OFONO_SIMAPP_TYPE_USIM,                 /* Umts */
    OFONO_SIMAPP_TYPE_ISIM                  /* Ims */
} OFONO_SIMAPP_TYPE;

struct ofono_simapp {
    OfonoObject object;
    OfonoSimAppPriv* priv;
    OFONO_SIMAPP_TYPE type;                 /* Type */
    const char* name;                       /* Name */
    const void* reserved;
    /* Base class, not extendible */
};

/* Authentication results */
#define OFONO_SIMAPP_AUTH_AUTS "AUTS"
#define OFONO_SIMAPP_AUTH_SRES "SRES"
#define OFONO_SIMAPP_AUTH_RES  "RES"
#define OFONO_SIMAPP_AUTH_CK   "CK"
#define OFONO_SIMAPP_AUTH_IK   "IK"
#define OFONO_SIMAPP_AUTH_Kc   "Kc"

GType ofono_simapp_get_type(void);
#define OFONO_TYPE_SIMAPP (ofono_simapp_get_type())
#define OFONO_SIMAPP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
        OFONO_TYPE_SIMAPP, OfonoSimApp))

typedef
void
(*OfonoSimAppHandler)(
    OfonoSimApp* app,
    void* arg);

OfonoSimApp*
ofono_simapp_ref(
    OfonoSimApp* app);

void
ofono_simapp_unref(
    OfonoSimApp* app);

gulong
ofono_simapp_add_valid_changed_handler(
    OfonoSimApp* app,
    OfonoSimAppHandler handler,
    void* arg);

void
ofono_simapp_remove_handler(
    OfonoSimApp* app,
    gulong id);

/* Inline wrappers */

OFONO_INLINE OfonoObject*
ofono_simapp_object(OfonoSimApp* app)
    { return G_LIKELY(app) ? &app->object : NULL; }

OFONO_INLINE const char*
ofono_simapp_path(const OfonoSimApp* app)
    { return G_LIKELY(app) ? app->object.path : NULL; }

OFONO_INLINE gboolean
ofono_simapp_valid(const OfonoSimApp* app)
    { return G_LIKELY(app) && app->object.valid; }

/* Macros */

#define ofono_simapp_wait_valid(app, msec, error) \
    ofono_object_wait_valid(ofono_simapp_object(app), msec, error)

#define ofono_simapp_remove_handlers(app, ids, n) \
    ofono_object_remove_handlers(ofono_simapp_object(app), ids, n)

#define ofono_simapp_remove_all_handlers(app, ids) \
    ofono_simapp_remove_handlers(app, ids, G_N_ELEMENTS(ids))

G_END_DECLS

#endif /* GOFONO_SIMAPP_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
