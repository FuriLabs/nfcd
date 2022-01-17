/*
 * Copyright (C) 2022 Jolla Ltd.
 * Copyright (C) 2022 Slava Monich <slava.monich@jolla.com>
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

#ifndef SETTINGS_PLUGIN_IMPL_H
#define SETTINGS_PLUGIN_IMPL_H

/* Internal header file for settings plugin implementation */

#include <nfc_plugin_impl.h>

#define GLOG_MODULE_NAME settings_log
#include <gutil_log.h>

#include <gio/gio.h>

#define SETTINGS_G_BUS           G_BUS_TYPE_SYSTEM
#define SETTINGS_DA_BUS          DA_BUS_SYSTEM

/* Class structure is exposed for unit testing */
typedef struct settings_plugin SettingsPlugin;
typedef struct settings_plugin_class {
    NfcPluginClass parent;
    const char* storage_dir;
    const char* config_dir;
} SettingsPluginClass;

GType settings_plugin_get_type(void);
#define SETTINGS_PLUGIN_TYPE settings_plugin_get_type()

/* And these are also separated so that unit test can substitute them */
guint
settings_plugin_name_own(
    SettingsPlugin* plugin,
    const char* name,
    GBusAcquiredCallback bus_acquired,
    GBusNameAcquiredCallback name_acquired,
    GBusNameLostCallback name_lost);

void
settings_plugin_name_unown(
    guint id);

#endif /* SETTINGS_PLUGIN_IMPL_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
