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

/* Generated headers */
#define OFONO_OBJECT_PROXY OrgOfonoSimAuthentication
#include "org.ofono.SimAuthentication.h"
#include "gofono_modemintf_p.h"
#undef OFONO_OBJECT_PROXY

#include "gofono_simauth.h"
#include "gofono_simapp_p.h"
#include "gofono_simapp_usim.h"
#include "gofono_modem_p.h"
#include "gofono_names.h"
#include "gofono_log.h"

#include <gutil_macros.h>

typedef struct ofono_simauth_app_entry {
    OfonoSimApp* app;
    gulong valid_handler_id;
} OfonoSimAuthAppEntry;

typedef struct ofono_simauth_call {
    OrgOfonoSimAuthentication* proxy;
    GCancellable* cancel;
    OfonoSimAuth* self;
} OfonoSimAuthCall;

struct ofono_simauth_priv {
    char* identity;
    OfonoSimAuthCall* get_apps_pending;
    GHashTable* apps;
    gboolean apps_ok;
};

typedef OfonoModemInterfaceClass OfonoSimAuthClass;
G_DEFINE_TYPE(OfonoSimAuth, ofono_simauth, OFONO_TYPE_MODEM_INTERFACE)
#define PARENT_CLASS ofono_simauth_parent_class
#define THIS_TYPE OFONO_TYPE_SIMAUTH
#define THIS(obj) OFONO_SIMAUTH(obj)
    
#define SIMAUTH_SIGNAL_IDENTITY_CHANGED_NAME "simauth-identity-changed"
    
/*==========================================================================*
 * Implementation
 *==========================================================================*/

static
void
ofono_simauth_app_entry_destroy(
    gpointer value)
{
    OfonoSimAuthAppEntry* entry = value;

    ofono_simapp_remove_handler(entry->app, entry->valid_handler_id);
    ofono_simapp_unref(entry->app);
    gutil_slice_free(entry);
}

static
gboolean
ofono_simauth_apps_valid(
    OfonoSimAuth* self)
{
    OfonoSimAuthPriv* priv = self->priv;
    GHashTableIter it;
    gpointer value;

    g_hash_table_iter_init(&it, priv->apps);
    while (g_hash_table_iter_next(&it, NULL, &value)) {
        const OfonoSimAuthAppEntry* entry = value;

        if (!ofono_simapp_valid(entry->app)) {
            return FALSE;
        }
    }
    return TRUE;
}

static
void
ofono_simauth_app_valid_changed(
    OfonoSimApp* app,
    void* self)
{
    ofono_object_update_valid(OFONO_OBJECT(self));
}

static
void
ofono_simauth_add_app(
    OfonoSimAuth* self,
    const char* path,
    GVariant* props)
{
    const char* type = NULL;

    if (g_variant_lookup(props, OFONO_SIMAPP_PROPERTY_TYPE, "&s", &type)) {
         OfonoSimApp* app;

        if (!g_strcmp0(type, OFONO_SIMAPP_TYPE_USIM_S)) {
            OfonoSimAppUSim* usim = ofono_simapp_usim_new(path);

            GDEBUG("%s app %s", type, path);
            ofono_simapp_usim_unref(self->usim);
            self->usim = usim;
            app = ofono_simapp_ref(OFONO_SIMAPP(usim));
        } else {
            /* At some point we will need to support ISimApplication too */
#pragma message("TODO: add support for Ims app")
            GDEBUG("Ignoring %s app %s", type, path);
            app = NULL;
        }
        if (app) {
            OfonoSimAuthPriv* priv = self->priv;
            OfonoSimAuthAppEntry* entry = g_slice_new0(OfonoSimAuthAppEntry);

            entry->app = app;
            entry->valid_handler_id =
                ofono_simapp_add_valid_changed_handler(app,
                    ofono_simauth_app_valid_changed, self);
            g_hash_table_replace(priv->apps, (gpointer)
                ofono_simapp_path(app), entry);
        }
    }
}
static
void
ofono_simauth_clear_apps(
    OfonoSimAuth* self)
{
    OfonoSimAuthPriv* priv = self->priv;

    g_hash_table_remove_all(priv->apps);
    ofono_simapp_usim_unref(self->usim);
    ofono_simapp_unref((OfonoSimApp*)self->isim);
    self->usim = NULL;
    self->isim = NULL;
}

static
void
ofono_simauth_get_apps_finished(
    GObject* proxy,
    GAsyncResult* result,
    gpointer data)
{
    OfonoSimAuthCall* call = data;
    OfonoSimAuth* self = call->self;
    GVariant* apps = NULL;
    GError* error = NULL;
    gboolean ok = org_ofono_sim_authentication_call_get_applications_finish
        (ORG_OFONO_SIM_AUTHENTICATION(proxy), &apps, result, &error);

    /* self is NULL if the call was cancelled */
    GASSERT(!self || self->priv->get_apps_pending == call);
    if (ok) {
        if (self) {
            GVariantIter it;
            GVariant* child;

            GVERBOSE("  %d app(s)", (int) g_variant_n_children(apps));
            ofono_simauth_clear_apps(self);
            for (g_variant_iter_init(&it, apps);
                 (child = g_variant_iter_next_value(&it)) != NULL;
                 g_variant_unref(child)) {
                const char* path = NULL;
                GVariant* props = NULL;

                g_variant_get(child, "{&o@a{sv}}", &path, &props);
                if (props) {
                    if (g_variant_is_object_path(path)) {
                        ofono_simauth_add_app(self, path, props);
                    }
                    g_variant_unref(props);
                }
            }
        }
        g_variant_unref(apps);
    } else if (error->code != G_IO_ERROR_CANCELLED) {
        GERR("%s.GetApplications %s", OFONO_SIMAUTH_INTERFACE_NAME,
            GERRMSG(error));
    }

    if (error) g_error_free(error);
    if (self) {
        OfonoSimAuthPriv* priv = self->priv;

        priv->apps_ok = ok;
        priv->get_apps_pending = NULL;
        ofono_object_update_valid(OFONO_OBJECT(self));
    }

    g_object_unref(call->proxy);
    g_object_unref(call->cancel);
    gutil_slice_free(call);
}

static
void
ofono_simauth_cancel_get_apps(
    OfonoSimAuth* self)
{
    OfonoSimAuthPriv* priv = self->priv;
    OfonoSimAuthCall* pending = priv->get_apps_pending;

    if (pending) {
        pending->self = NULL;
        priv->get_apps_pending = NULL;
        g_cancellable_cancel(pending->cancel);
    }
}

static
void
ofono_simauth_start_get_apps(
    OfonoSimAuth* self,
    OrgOfonoSimAuthentication* proxy)
{
    OfonoSimAuthPriv* priv = self->priv;
    OfonoSimAuthCall* call;

    call = g_slice_new0(OfonoSimAuthCall);
    call->cancel = g_cancellable_new();
    call->self = self;
    g_object_ref(call->proxy = proxy);

    ofono_simauth_cancel_get_apps(self);
    priv->apps_ok = FALSE;
    priv->get_apps_pending = call;
    org_ofono_sim_authentication_call_get_applications(proxy, call->cancel,
        ofono_simauth_get_apps_finished, call);
}

static
void
ofono_simauth_proxy_created(
    OfonoObject* object,
    OrgOfonoSimAuthentication* proxy)
{
    /* This interface doesn't need to handle timeouts */
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(proxy), G_MAXINT);
    OFONO_OBJECT_CLASS(PARENT_CLASS)->fn_proxy_created(object, proxy);
    if (OFONO_OBJECT_GET_CLASS(object)->fn_is_ready(object)) {
        GVERBOSE_("Fetching apps...");
        ofono_simauth_start_get_apps(THIS(object), proxy);
    }
}

/*==========================================================================*
 * API
 *==========================================================================*/

OfonoSimAuth*
ofono_simauth_new(
    const char* path)
{
    const char* ifname = OFONO_SIMAUTH_INTERFACE_NAME;
    OfonoModem* modem = ofono_modem_new(path);
    OfonoModemInterface* intf = ofono_modem_get_interface(modem, ifname);
    OfonoSimAuth* self;

    if (G_TYPE_CHECK_INSTANCE_TYPE(intf, THIS_TYPE)) {
        /* Reuse the existing object */
        self = ofono_simauth_ref(THIS(intf));
    } else {
        self = g_object_new(THIS_TYPE, NULL);
        intf = &self->intf;
        GVERBOSE_("%s", path);
        ofono_modem_interface_initialize(intf, ifname, path);
        ofono_modem_set_interface(modem, intf);
        ofono_object_update_ready(OFONO_OBJECT(self));
        GASSERT(!ofono_object_proxy(OFONO_OBJECT(self)));
        GASSERT(intf->modem == modem);
    }

    ofono_modem_unref(modem);
    return self;
}

OfonoSimAuth*
ofono_simauth_ref(
    OfonoSimAuth* self)
{
    if (G_LIKELY(self)) {
        g_object_ref(self);
        return self;
    } else {
        return NULL;
    }
}

void
ofono_simauth_unref(
    OfonoSimAuth* self)
{
    if (G_LIKELY(self)) {
        g_object_unref(self);
    }
}

gulong
ofono_simauth_add_valid_changed_handler(
    OfonoSimAuth* self,
    OfonoSimAuthHandler handler,
    void* arg) /* Since 2.1.1 */
{
    return G_LIKELY(self) ? ofono_object_add_valid_changed_handler(
        ofono_simauth_object(self), (OfonoObjectHandler) handler, arg) : 0;
}

void
ofono_simauth_remove_handler(
    OfonoSimAuth* self,
    gulong id)
{
    if (G_LIKELY(self) && G_LIKELY(id)) {
        g_signal_handler_disconnect(self, id);
    }
}

/*==========================================================================*
 * Internals
 *==========================================================================*/

#define SIMAUTH_DEFINE_PROPERTY_STRING(NAME,var)                        \
    OFONO_OBJECT_DEFINE_PROPERTY_STRING(SIMAUTH,simauth,NAME,OfonoSimAuth,var)

static
void*
ofono_simauth_property_priv(
    OfonoObject* object,
    const OfonoObjectProperty* prop)
{
    return THIS(object)->priv;
}

static
gboolean
ofono_simauth_is_valid(
    OfonoObject* object)
{
    OfonoSimAuth* self = THIS(object);
    OfonoSimAuthPriv* priv = self->priv;

    return priv->apps_ok && !priv->get_apps_pending &&
        ofono_simauth_apps_valid(self) &&
        OFONO_OBJECT_CLASS(PARENT_CLASS)->fn_is_valid(object);
}

static
void
ofono_simauth_ready_changed(
    OfonoObject* object,
    gboolean ready)
{
    OfonoSimAuth* self = THIS(object);
    OrgOfonoSimAuthentication* proxy = ofono_object_proxy(object);

    if (proxy && ready) {
        GVERBOSE_("Fetching contexts...");
        ofono_simauth_start_get_apps(self, proxy);
    } else {
        ofono_simauth_cancel_get_apps(self);
        if (!ready) {
            ofono_simauth_clear_apps(self);
        }
    }
    OFONO_OBJECT_CLASS(PARENT_CLASS)->fn_ready_changed(object, ready);
}

static
void
ofono_simauth_finalize(
    GObject* object)
{
    OfonoSimAuth* self = THIS(object);
    OfonoSimAuthPriv* priv = self->priv;

    g_free(priv->identity);
    ofono_simauth_cancel_get_apps(self);
    ofono_simauth_clear_apps(self);
    g_hash_table_destroy(priv->apps);
    G_OBJECT_CLASS(PARENT_CLASS)->finalize(object);
}

static
void
ofono_simauth_init(
    OfonoSimAuth* self)
{
    OfonoSimAuthPriv* priv = G_TYPE_INSTANCE_GET_PRIVATE(self, THIS_TYPE,
        OfonoSimAuthPriv);

    self->priv = priv;
    priv->apps = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
        ofono_simauth_app_entry_destroy);
}

/**
 * Per class initializer
 */
static
void
ofono_simauth_class_init(
    OfonoSimAuthClass* klass)
{
    static OfonoObjectProperty ofono_simauth_properties[] = {
        SIMAUTH_DEFINE_PROPERTY_STRING(IDENTITY,identity)
    };

    GObjectClass* object = G_OBJECT_CLASS(klass);
    OfonoObjectClass* ofono = &klass->object;

    object->finalize = ofono_simauth_finalize;
    g_type_class_add_private(klass, sizeof(OfonoSimAuthPriv));
    ofono->fn_is_valid = ofono_simauth_is_valid;
    ofono->fn_proxy_created = ofono_simauth_proxy_created;
    ofono->fn_ready_changed = ofono_simauth_ready_changed;
    ofono->properties = ofono_simauth_properties;
    ofono->nproperties = G_N_ELEMENTS(ofono_simauth_properties);
    OFONO_OBJECT_CLASS_SET_PROXY_CALLBACKS_RO(ofono,
        org_ofono_sim_authentication);
    ofono_class_initialize(ofono);
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
