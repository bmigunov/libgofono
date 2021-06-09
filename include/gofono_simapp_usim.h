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

#ifndef GOFONO_SIMAPP_USIM_H
#define GOFONO_SIMAPP_USIM_H

/* This file exists since 2.1.0 */

#include "gofono_simapp.h"

G_BEGIN_DECLS

typedef struct ofono_simapp_usim_priv OfonoSimAppUSimPriv;

struct ofono_simapp_usim {
    OfonoSimApp app;
    OfonoSimAppUSimPriv* priv;
};

GType ofono_simapp_usim_get_type(void);
#define OFONO_TYPE_SIMAPP_USIM (ofono_simapp_usim_get_type())
#define OFONO_SIMAPP_USIM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
        OFONO_TYPE_SIMAPP_USIM, OfonoSimAppUSim))

typedef
void
(*OfonoSimAppUSimAuthenticateComplete)(
    OfonoSimAppUSim* usim,
    GHashTable* result, /* char* => GBytes* map */
    const GError* error,
    void* arg);

OfonoSimAppUSim*
ofono_simapp_usim_ref(
    OfonoSimAppUSim* usim);

void
ofono_simapp_usim_unref(
    OfonoSimAppUSim* usim);

GCancellable*
ofono_simapp_usim_umts_authenticate(
    OfonoSimAppUSim* usim,
    GBytes* rand,
    GBytes* autn,
    OfonoSimAppUSimAuthenticateComplete complete,
    GDestroyNotify destroy,
    void* data);

/* Inline wrappers */

OFONO_INLINE OfonoObject*
ofono_simapp_usim_object(OfonoSimAppUSim* usim)
    { return G_LIKELY(usim) ? ofono_simapp_object(&usim->app) : NULL; }

OFONO_INLINE const char*
ofono_simapp_usim_path(const OfonoSimAppUSim* usim)
    { return G_LIKELY(usim) ? ofono_simapp_path(&usim->app) : NULL; }

OFONO_INLINE gboolean
ofono_simapp_usim_valid(const OfonoSimAppUSim* usim)
    { return G_LIKELY(usim) && ofono_simapp_valid(&usim->app); }

G_END_DECLS

#endif /* GOFONO_SIMAPP_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
