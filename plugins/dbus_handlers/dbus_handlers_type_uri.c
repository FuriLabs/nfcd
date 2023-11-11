/*
 * Copyright (C) 2018-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2019 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "dbus_handlers.h"

static const char dbus_handlers_type_uri_key[] = "URI";

static
gboolean
dbus_handlers_type_uri_supported_record(
    NdefRec* ndef)
{
    return NDEF_IS_REC_U(ndef);
}

static
gboolean
dbus_handlers_type_uri_match(
    GKeyFile* file,
    const char* group,
    NdefRecU* rec)
{
    char* pattern = dbus_handlers_config_get_string(file, group,
        dbus_handlers_type_uri_key);
    gboolean match = (!pattern || g_pattern_match_simple(pattern, rec->uri));

    g_free(pattern);
    return match;
}

static
DBusHandlerConfig*
dbus_handlers_type_uri_new_handler_config(
    GKeyFile* file,
    NdefRec* ndef)
{
    static const char group[] = "URI-Handler";

    return dbus_handlers_type_uri_match(file, group, NDEF_REC_U(ndef)) ?
        dbus_handlers_new_handler_config(file, group) : NULL;
}

static
DBusListenerConfig*
dbus_handlers_type_uri_new_listener_config(
    GKeyFile* file,
    NdefRec* ndef)
{
    static const char group[] = "URI-Listener";

    return dbus_handlers_type_uri_match(file, group, NDEF_REC_U(ndef)) ?
        dbus_handlers_new_listener_config(file, group) : NULL;
}

static
GVariant*
dbus_handlers_type_uri_handler_args(
    NdefRec* ndef)
{
    return g_variant_new ("(s)", NDEF_REC_U(ndef)->uri);
}

static
GVariant*
dbus_handlers_type_uri_listener_args(
    gboolean handled,
    NdefRec* ndef)
{
    return g_variant_new ("(bs)", handled, NDEF_REC_U(ndef)->uri);
}

const DBusHandlerType dbus_handlers_type_uri = {
    .name = "URI",
    .priority = DBUS_HANDLER_PRIORITY_DEFAULT,
    .supported_record = dbus_handlers_type_uri_supported_record,
    .new_handler_config = dbus_handlers_type_uri_new_handler_config,
    .new_listener_config = dbus_handlers_type_uri_new_listener_config,
    .free_handler_config = dbus_handlers_free_handler_config,
    .free_listener_config = dbus_handlers_free_listener_config,
    .handler_args = dbus_handlers_type_uri_handler_args,
    .listener_args = dbus_handlers_type_uri_listener_args
};

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
