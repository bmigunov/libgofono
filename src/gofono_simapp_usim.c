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

#define OFONO_OBJECT_PROXY OrgOfonoUSimApplication
#include "org.ofono.USimApplication.h"
#include "gofono_object_p.h"

#include "gofono_simapp_usim.h"
#include "gofono_simapp_p.h"
#include "gofono_names.h"
#include "gofono_log.h"

#include <gutil_macros.h>

typedef OfonoSimAppClass OfonoSimAppUSimClass;

G_DEFINE_TYPE(OfonoSimAppUSim, ofono_simapp_usim, OFONO_TYPE_SIMAPP)
#define SUPER_CLASS ofono_simapp_usim_parent_class
#define THIS_TYPE OFONO_TYPE_SIMAPP_USIM
#define THIS(obj) OFONO_SIMAPP_USIM(obj)

typedef
gboolean
(*OfonoSimAppUSimAuthenticateFinish)(
    OrgOfonoUSimApplication* proxy,
    GVariant** result,
    GAsyncResult* res,
    GError **error);

typedef struct ofono_simapp_usim_call {
    OfonoSimAppUSim* usim;
    GCancellable* cancel;
    OfonoSimAppUSimAuthenticateFinish finish;
    OfonoSimAppUSimAuthenticateComplete complete;
    GDestroyNotify destroy;
    void* data;
} OfonoSimAppUSimCall;

/*==========================================================================*
 * Implementation
 *==========================================================================*/

static
OfonoSimAppUSimCall*
ofono_simapp_usim_call_new(
    OfonoSimAppUSim* usim,
    OfonoSimAppUSimAuthenticateFinish finish,
    OfonoSimAppUSimAuthenticateComplete complete,
    GDestroyNotify destroy,
    void* data)
{
    OfonoSimAppUSimCall* call = g_slice_new0(OfonoSimAppUSimCall);

    call->cancel = g_cancellable_new();
    call->usim = ofono_simapp_usim_ref(usim);
    call->finish = finish;
    call->complete = complete;
    call->destroy = destroy;
    call->data = data;
    return call;
}

static
void
ofono_simapp_usim_call_free(
    OfonoSimAppUSimCall* call)
{
    if (call->destroy) {
        call->destroy(call->data);
    }
    ofono_simapp_usim_unref(call->usim);
    g_object_unref(call->cancel);
    gutil_slice_free(call);
}

static
void
ofono_simapp_usim_authenticate_complete(
    GObject* proxy,
    GAsyncResult* res,
    gpointer data)
{
    OfonoSimAppUSimCall* call = data;
    OfonoSimAppUSim* usim = call->usim;

    if (!g_cancellable_is_cancelled(call->cancel) && call->complete) {
        GError* err = NULL;
        GVariant* out;

        if (call->finish(ORG_OFONO_USIM_APPLICATION(proxy), &out, res, &err)) {
            GVariantIter it;
            GVariant* child;
            GHashTable* results = g_hash_table_new_full(g_str_hash,
                g_str_equal, g_free, (GDestroyNotify) g_bytes_unref);

            for (g_variant_iter_init(&it, out);
                 (child = g_variant_iter_next_value(&it)) != NULL;
                 g_variant_unref(child)) {
                const char* name = NULL;
                GVariant* bytes = NULL;

                g_variant_get(child, "{&s@ay}", &name, &bytes);
                if (bytes) {
                    g_hash_table_replace(results, g_strdup(name),
                        g_bytes_new_with_free_func(g_variant_get_data(bytes),
                        g_variant_get_size(bytes), (GDestroyNotify)
                        g_variant_unref, g_variant_ref(bytes)));
                    g_variant_unref(bytes);
                }
            }
            call->complete(usim, results, NULL, call->data);
            g_hash_table_unref(results);
            g_variant_unref(out);
        } else {
            GDEBUG_("%s %s", ofono_simapp_usim_path(usim), GERRMSG(err));
            call->complete(usim, NULL, err, call->data);
            g_error_free(err);
        }
    }
    ofono_simapp_usim_call_free(call);
}

/*==========================================================================*
 * API
 *==========================================================================*/

OfonoSimAppUSim*
ofono_simapp_usim_ref(
    OfonoSimAppUSim* self)
{
    if (G_LIKELY(self)) {
        g_object_ref(self);
        return self;
    } else {
        return NULL;
    }
}

void
ofono_simapp_usim_unref(
    OfonoSimAppUSim* self)
{
    if (G_LIKELY(self)) {
        g_object_unref(self);
    }
}

GCancellable*
ofono_simapp_usim_umts_authenticate(
    OfonoSimAppUSim* self,
    GBytes* rand,
    GBytes* autn,
    OfonoSimAppUSimAuthenticateComplete complete,
    GDestroyNotify destroy,
    void* data)
{
    if (G_LIKELY(self) && G_LIKELY(rand) && G_LIKELY(autn)) {
        OrgOfonoUSimApplication* proxy = ofono_object_proxy(OFONO_OBJECT(self));
        OfonoSimAppUSimCall* call = ofono_simapp_usim_call_new(self,
            org_ofono_usim_application_call_umts_authenticate_finish,
            complete, destroy,data);

        org_ofono_usim_application_call_umts_authenticate(proxy,
            g_variant_new_from_bytes(G_VARIANT_TYPE_BYTESTRING, rand, TRUE),
            g_variant_new_from_bytes(G_VARIANT_TYPE_BYTESTRING, autn, TRUE),
            call->cancel, ofono_simapp_usim_authenticate_complete, call);
        return call->cancel;
    }
    return NULL;
}

/*==========================================================================*
 * Internal API
 *==========================================================================*/

OfonoSimAppUSim*
ofono_simapp_usim_new(
    const char* path)
{
    OfonoSimAppUSim* self = g_object_new(THIS_TYPE, NULL);
    OfonoSimApp* app = &self->app;

    ofono_simapp_initialize(app, OFONO_USIM_APPLICATION_INTERFACE_NAME, path);
    return self;
}

/*==========================================================================*
 * Internals
 *==========================================================================*/

static
void
ofono_simapp_usim_init(
    OfonoSimAppUSim* self)
{
}

static
void
ofono_simapp_usim_class_init(
    OfonoSimAppUSimClass* klass)
{
    OfonoObjectClass* ofono = OFONO_OBJECT_CLASS(klass);

    OFONO_OBJECT_CLASS_SET_PROXY_CALLBACKS_RO(ofono,
        org_ofono_usim_application);
    ofono_class_initialize(ofono);
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
