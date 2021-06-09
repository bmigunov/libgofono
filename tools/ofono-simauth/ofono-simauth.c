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

#include "gofono_modem.h"
#include "gofono_names.h"
#include "gofono_simmgr.h"
#include "gofono_simauth.h"
#include "gofono_simapp_usim.h"

#include <gutil_misc.h>
#include <gutil_log.h>

#include <glib-unix.h>

#define RET_OK          (0)
#define RET_CMDLINE     (1)
#define RET_NOTFOUND    (2)
#define RET_ERR         (3)
#define RET_TIMEOUT     (4)

typedef struct app {
    const char* path;
    GBytes* rand;
    GBytes* autn;
    int ret;
    GMainLoop* loop;
} App;

static
void
app_dump_data(
    const GLogModule* module,
    const int level,
    const char* prefix,
    const void* data,
    gsize size)
{
    if (GLOG_ENABLED(level)) {
        GString* buf = g_string_new(NULL);

        if (size > 0) {
            guint i;
            const guint8* ptr = data;

            g_string_append_printf(buf, "%02x", ptr[0]);
            for (i = 1; i < size; i++) {
                g_string_append_printf(buf, "%02x", ptr[i]);
            }
        }
        gutil_log(module, level, "%s%s", prefix, buf->str);
        g_string_free(buf, TRUE);
    }
}

static
void
app_dump_bytes(
    const GLogModule* module,
    const int level,
    const char* prefix,
    GBytes* bytes)
{
    gsize size = 0;
    const guint8* data;

    if (bytes) {
        data = g_bytes_get_data(bytes, &size);
    } else {
        size = 0;
        data = NULL;
    }

    app_dump_data(module, level, prefix, data, size);
}

static
gboolean
app_signal(
    gpointer arg)
{
    GMainLoop* loop = arg;
    GINFO("Caught signal, shutting down...");
    g_idle_add((GSourceFunc)g_main_loop_quit, loop);
    return G_SOURCE_CONTINUE;
}

static
void
app_auth_complete(
    OfonoSimAppUSim* usim,
    GHashTable* result,
    const GError* error,
    void* data)
{
    App* app = data;

    if (error) {
        GERR("%s", error->message);
    } else {
        GHashTableIter it;
        gpointer key, value;

        g_hash_table_iter_init(&it, result);
        while (g_hash_table_iter_next(&it, &key, &value)) {
            char* prefix = g_strdup_printf("%s=", (char*) key);

            app_dump_bytes(GLOG_MODULE_CURRENT, GLOG_LEVEL_INFO, prefix, value);
            g_free(prefix);
        }
    }
    g_main_loop_quit(app->loop);
}

static
void
app_usim_auth(
    App* app,
    OfonoSimAppUSim* usim)
{
    GCancellable* cancel;
    guint sigterm_id, sigint_id;

    sigterm_id = g_unix_signal_add(SIGTERM, app_signal, app->loop);
    sigint_id = g_unix_signal_add(SIGINT, app_signal, app->loop);
    g_object_ref(cancel = ofono_simapp_usim_umts_authenticate(usim,
        app->rand, app->autn, app_auth_complete, NULL, app));
    g_main_loop_run(app->loop);
    g_source_remove(sigterm_id);
    g_source_remove(sigint_id);
    g_cancellable_cancel(cancel);
    g_object_unref(cancel);
}

static
int
app_run(
    App* app)
{
    GError* error = NULL;
    OfonoSimMgr* sim = ofono_simmgr_new(app->path);

    if (ofono_simmgr_wait_valid(sim, -1, &error)) {
        if (sim->present) {
            if (ofono_modem_has_interface(sim->intf.modem,
                OFONO_SIMAUTH_INTERFACE_NAME)) {
                OfonoSimAuth* auth = ofono_simauth_new(app->path);

                if (ofono_simauth_wait_valid(auth, -1, &error)) {
                    app->loop = g_main_loop_new(NULL, FALSE);
                    if (auth->usim) {
                        app_usim_auth(app, auth->usim);
                    } else {
                        GERR("No USIM application at %s", app->path);
                    }
                    g_main_loop_unref(app->loop);
                    app->loop = NULL;
                } else {
                    app->ret = RET_NOTFOUND;
                    GERR("%s", error->message);
                    g_error_free(error);
                }
                ofono_simauth_unref(auth);
            } else {
                GERR("SIM at %s doesn't support authentication", app->path);
                app->ret = RET_NOTFOUND;
            }
        } else {
            GERR("No SIM at %s", app->path);
            app->ret = RET_NOTFOUND;
        }
    } else {
        GERR("%s", error->message);
        g_error_free(error);
    }
    ofono_simmgr_unref(sim);
    return app->ret;
}

static
gboolean
app_init(
    App* app,
    int argc,
    char* argv[])
{
    gboolean ok = FALSE;
    gboolean verbose = FALSE;
    GOptionEntry entries[] = {
        { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
          "Enable verbose output", NULL },
        { NULL }
    };
    GError* error = NULL;
    GOptionContext* options = g_option_context_new("PATH RAND AUTN");

    g_option_context_add_main_entries(options, entries, NULL);
    if (g_option_context_parse(options, &argc, &argv, &error)) {
        if (argc == 4 &&
            g_variant_is_object_path(app->path = argv[1]) &&
            (app->rand = gutil_hex2bytes(argv[2], -1)) != NULL  &&
            (app->autn = gutil_hex2bytes(argv[3], -1)) != NULL) {
            app->ret = RET_ERR;
            if (verbose) gutil_log_default.level = GLOG_LEVEL_VERBOSE;
            ok = TRUE;
        } else {
            char* help = g_option_context_get_help(options, TRUE, NULL);
            fprintf(stderr, "%s", help);
            g_free(help);
        }
    } else {
        GERR("%s", error->message);
        g_error_free(error);
    }
    g_option_context_free(options);
    return ok;
}

int main(int argc, char* argv[])
{
    int ret = RET_CMDLINE;
    App app;

    memset(&app, 0, sizeof(app));
    gutil_log_timestamp = FALSE;
    gutil_log_set_type(GLOG_TYPE_STDOUT, NULL);
    gutil_log_default.level = GLOG_LEVEL_DEFAULT;
    if (app_init(&app, argc, argv)) {
        ret = app_run(&app);
    }
    if (app.rand) {
        g_bytes_unref(app.rand);
    }
    if (app.autn) {
        g_bytes_unref(app.autn);
    }
    return ret;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
