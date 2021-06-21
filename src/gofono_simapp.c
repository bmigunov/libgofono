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

#include "gofono_simapp_p.h"
#include "gofono_util.h"
#include "gofono_names.h"

struct ofono_simapp_priv {
    char* name;
};

G_DEFINE_TYPE(OfonoSimApp, ofono_simapp, OFONO_TYPE_OBJECT)
#define PARENT_CLASS ofono_simapp_parent_class
#define THIS_TYPE OFONO_TYPE_SIMAPP
#define THIS(obj) OFONO_SIMAPP(obj)

#define SIMAPP_SIGNAL_TYPE_CHANGED_NAME "simapp-type-changed"
#define SIMAPP_SIGNAL_NAME_CHANGED_NAME "simapp-name-changed"

/* Enum <-> string mappings */
static const OfonoNameIntPair ofono_simapp_type_values[] = {
    { OFONO_SIMAPP_TYPE_USIM_S, OFONO_SIMAPP_TYPE_USIM },
    { OFONO_SIMAPP_TYPE_ISIM_S, OFONO_SIMAPP_TYPE_ISIM }
};

static const OfonoNameIntMap ofono_simapp_type_map = {
    "type",
    OFONO_NAME_INT_MAP_ENTRIES(ofono_simapp_type_values),
    { NULL,  OFONO_SIMAPP_TYPE_UNKNOWN }
};

/*==========================================================================*
 * API
 *==========================================================================*/

OfonoSimApp*
ofono_simapp_ref(
    OfonoSimApp* self)
{
    if (G_LIKELY(self)) {
        g_object_ref(self);
        return self;
    } else {
        return NULL;
    }
}

void
ofono_simapp_unref(
    OfonoSimApp* self)
{
    if (G_LIKELY(self)) {
        g_object_unref(self);
    }
}

gulong
ofono_simapp_add_valid_changed_handler(
    OfonoSimApp* self,
    OfonoSimAppHandler handler,
    void* arg)
{
    return G_LIKELY(self) ? ofono_object_add_valid_changed_handler(
        ofono_simapp_object(self), (OfonoObjectHandler) handler, arg) : 0;
}

void
ofono_simapp_remove_handler(
    OfonoSimApp* self,
    gulong id)
{
    if (G_LIKELY(self) && G_LIKELY(id)) {
        g_signal_handler_disconnect(self, id);
    }
}

/*==========================================================================*
 * Internal API
 *==========================================================================*/

void
ofono_simapp_initialize(
    OfonoSimApp* self,
    const char* intf,
    const char* path)
{
    ofono_object_initialize(&self->object, intf, path);
}

/*==========================================================================*
 * Internals
 *==========================================================================*/

#define SIMAPP_DEFINE_PROPERTY_STRING(NAME,var) \
    OFONO_OBJECT_DEFINE_PROPERTY_STRING(SIMAPP,simapp,NAME,OfonoSimApp,var)

#define SIMAPP_DEFINE_PROPERTY_ENUM(NAME,var) \
    OFONO_OBJECT_DEFINE_PROPERTY_ENUM(SIMAPP,NAME,OfonoSimApp,var, \
    &ofono_simapp_##var##_map)

G_STATIC_ASSERT(sizeof(OFONO_SIMAPP_TYPE) == sizeof(int));

static
void*
ofono_simapp_property_priv(
    OfonoObject* object,
    const OfonoObjectProperty* prop)
{
    return THIS(object)->priv;
}

static
void
ofono_simapp_finalize(
    GObject* object)
{
    OfonoSimApp* self = THIS(object);
    OfonoSimAppPriv* priv = self->priv;

    g_free(priv->name);
    G_OBJECT_CLASS(PARENT_CLASS)->finalize(object);
}

static
void
ofono_simapp_init(
    OfonoSimApp* self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, THIS_TYPE, OfonoSimAppPriv);
}

static
void
ofono_simapp_class_init(
    OfonoSimAppClass* klass)
{
    static OfonoObjectProperty ofono_simapp_properties[] = {
        SIMAPP_DEFINE_PROPERTY_ENUM(TYPE,type),
        SIMAPP_DEFINE_PROPERTY_STRING(NAME,name)
    };

    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    klass->properties = ofono_simapp_properties;
    klass->nproperties = G_N_ELEMENTS(ofono_simapp_properties);
    object_class->finalize = ofono_simapp_finalize;
    g_type_class_add_private(klass, sizeof(OfonoSimAppPriv));
    ofono_class_initialize(klass);
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
