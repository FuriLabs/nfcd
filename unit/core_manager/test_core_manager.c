/*
 * Copyright (C) 2018-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2021 Jolla Ltd.
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

#include "test_common.h"
#include "test_service.h"
#include "test_host_app.h"
#include "test_host_service.h"

#include "nfc_manager_p.h"
#include "internal/nfc_manager_i.h"
#include "nfc_adapter_impl.h"

#include <gutil_log.h>

static TestOpt test_opt;

static
void
test_manager_inc(
    NfcManager* manager,
    void* user_data)
{
    (*(int*)user_data)++;
}

static
void
test_manager_adapter_inc(
    NfcManager* manager,
    NfcAdapter* adapter,
    void* user_data)
{
    (*(int*)user_data)++;
}

/*==========================================================================*
 * Test adapter
 *==========================================================================*/

typedef NfcAdapterClass TestAdapterClass;
typedef struct test_adapter {
    NfcAdapter adapter;
    gboolean power_request_pending;
    gboolean power_requested;
    gboolean mode_request_pending;
    NFC_MODE mode_requested;
    NFC_TECHNOLOGY supported_techs;
    NFC_TECHNOLOGY allowed_techs;
} TestAdapter;

G_DEFINE_TYPE(TestAdapter, test_adapter, NFC_TYPE_ADAPTER)
#define TEST_TYPE_ADAPTER (test_adapter_get_type())
#define TEST_ADAPTER(obj) (G_TYPE_CHECK_INSTANCE_CAST(obj, \
        TEST_TYPE_ADAPTER, TestAdapter))

TestAdapter*
test_adapter_new(
    void)
{
    return g_object_new(TEST_TYPE_ADAPTER, NULL);
}

static
gboolean
test_adapter_submit_power_request(
    NfcAdapter* adapter,
    gboolean on)
{
    TestAdapter* self = TEST_ADAPTER(adapter);

    g_assert(!self->power_request_pending);
    self->power_requested = on;
    self->power_request_pending = TRUE;
    return TRUE;
}

static
void
test_adapter_cancel_power_request(
    NfcAdapter* adapter)
{
    TestAdapter* self = TEST_ADAPTER(adapter);

    g_assert(self->power_request_pending);
    self->power_request_pending = FALSE;
    NFC_ADAPTER_CLASS(test_adapter_parent_class)->cancel_power_request(adapter);
}

static
void
test_adapter_complete_power_request(
    TestAdapter* self)
{
    g_assert(self->power_request_pending);
    self->power_request_pending = FALSE;
    nfc_adapter_power_notify(&self->adapter, self->power_requested, TRUE);
}

static
gboolean
test_adapter_submit_mode_request(
    NfcAdapter* adapter,
    NFC_MODE mode)
{
    TestAdapter* self = TEST_ADAPTER(adapter);

    g_assert(!self->mode_request_pending);
    self->mode_requested = mode;
    self->mode_request_pending = TRUE;
    return TRUE;
}

static
void
test_adapter_cancel_mode_request(
    NfcAdapter* adapter)
{
    TestAdapter* self = TEST_ADAPTER(adapter);

    g_assert(self->mode_request_pending);
    self->mode_request_pending = FALSE;
    NFC_ADAPTER_CLASS(test_adapter_parent_class)->cancel_mode_request(adapter);
}

static
NFC_TECHNOLOGY
test_adapter_get_supported_techs(
    NfcAdapter* adapter)
{
    return TEST_ADAPTER(adapter)->supported_techs;
}

static
void
test_adapter_set_allowed_techs(
    NfcAdapter* adapter,
    NFC_TECHNOLOGY techs)
{
    TEST_ADAPTER(adapter)->allowed_techs = techs;
}

static
void
test_adapter_init(
    TestAdapter* self)
{
    self->adapter.supported_modes = NFC_MODE_READER_WRITER;
    self->allowed_techs = self->supported_techs =
        NFC_TECHNOLOGY_A | NFC_TECHNOLOGY_B;
}

static
void
test_adapter_class_init(
    NfcAdapterClass* klass)
{
    klass->submit_power_request = test_adapter_submit_power_request;
    klass->cancel_power_request = test_adapter_cancel_power_request;
    klass->submit_mode_request = test_adapter_submit_mode_request;
    klass->cancel_mode_request = test_adapter_cancel_mode_request;
    klass->get_supported_techs = test_adapter_get_supported_techs;
    klass->set_allowed_techs = test_adapter_set_allowed_techs;
}

/*==========================================================================*
 * null
 *==========================================================================*/

static
void
test_null(
    void)
{
    /* Public interfaces are NULL tolerant */
    g_assert(!nfc_manager_ref(NULL));
    g_assert(!nfc_manager_plugins(NULL));
    g_assert(!nfc_manager_get_adapter(NULL, NULL));
    g_assert(!nfc_manager_add_adapter(NULL, NULL));
    g_assert(!nfc_manager_add_adapter_added_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_add_adapter_removed_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_add_service_registered_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_add_service_unregistered_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_add_enabled_changed_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_add_mode_changed_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_add_techs_changed_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_add_stopped_handler(NULL, NULL, NULL));
    g_assert(!nfc_manager_mode_request_new(NULL, 0, 0));
    g_assert(!nfc_manager_tech_request_new(NULL, 0, 0));

    nfc_manager_mode_request_free(NULL);
    nfc_manager_tech_request_free(NULL);
    nfc_manager_stop(NULL, 0);
    nfc_manager_set_enabled(NULL, FALSE);
    nfc_manager_request_power(NULL, FALSE);
    nfc_manager_request_mode(NULL, NFC_MODE_NONE);
    nfc_manager_register_host_app(NULL, NULL);
    nfc_manager_register_host_service(NULL, NULL);
    nfc_manager_register_service(NULL, NULL);
    nfc_manager_unregister_host_app(NULL, NULL);
    nfc_manager_unregister_host_service(NULL, NULL);
    nfc_manager_unregister_service(NULL, NULL);
    nfc_manager_remove_adapter(NULL, NULL);
    nfc_manager_remove_handler(NULL, 0);
    nfc_manager_remove_handlers(NULL, NULL, 0);
    nfc_manager_unref(NULL);

    /* The internal APIs are NULL tolerant too */
    g_assert(!nfc_manager_start(NULL));
    g_assert(!nfc_manager_peer_services(NULL));
    g_assert(!nfc_manager_host_services(NULL));
    g_assert(!nfc_manager_host_apps(NULL));
}

/*==========================================================================*
 * basic
 *==========================================================================*/

static
void
test_basic(
    void)
{
    NfcPluginsInfo pi;
    NfcManager* manager;
    NfcPlugin* const* plugins;
    int count = 0;
    gulong id;

    memset(&pi, 0, sizeof(pi));
    manager = nfc_manager_new(&pi);

    /* No plugins */
    plugins = nfc_manager_plugins(manager);
    g_assert(plugins);
    g_assert(!plugins[0]);

    /* NULL services are ignored */
    g_assert(!nfc_manager_register_service(manager, NULL));
    nfc_manager_unregister_service(manager, NULL);

    /* No adapters */
    g_assert(!nfc_manager_get_adapter(manager, "foo"));
    id = nfc_manager_add_adapter_removed_handler(manager,
        test_manager_adapter_inc, &count);
    nfc_manager_remove_adapter(manager, "foo");
    nfc_manager_remove_handler(manager, id);
    g_assert(!count);

    nfc_manager_request_power(manager, TRUE);
    nfc_manager_request_mode(manager, NFC_MODE_NONE);

    /* Events */
    id = nfc_manager_add_enabled_changed_handler(manager,
        test_manager_inc, &count);
    g_assert(manager->enabled);
    nfc_manager_set_enabled(manager, FALSE);
    g_assert(!manager->enabled);
    g_assert(count == 1);
    nfc_manager_set_enabled(manager, FALSE);
    g_assert(count == 1);
    count = 0;
    nfc_manager_remove_handler(manager, id);

    id = nfc_manager_add_stopped_handler(manager,
        test_manager_inc, &count);
    g_assert(nfc_manager_start(manager));
    nfc_manager_stop(manager, 0);
    g_assert(count == 1);
    nfc_manager_stop(manager, 1);
    g_assert(count == 1);
    g_assert(manager->error == 1);
    nfc_manager_stop(manager, 2);
    g_assert(manager->error == 1); /* Error remains 1 */
    g_assert(count == 1);
    count = 0;
    nfc_manager_remove_handler(manager, id);

    /* These have no effect */
    g_assert(!nfc_manager_get_adapter(manager, NULL));
    g_assert(!nfc_manager_add_adapter(manager, NULL));
    g_assert(!nfc_manager_add_adapter_added_handler(manager, NULL, NULL));
    g_assert(!nfc_manager_add_adapter_removed_handler(manager, NULL, NULL));
    g_assert(!nfc_manager_add_service_registered_handler(manager, NULL, NULL));
    g_assert(!nfc_manager_add_service_unregistered_handler(manager, NULL,NULL));
    g_assert(!nfc_manager_add_enabled_changed_handler(manager, NULL, NULL));
    g_assert(!nfc_manager_add_stopped_handler(manager, NULL, NULL));
    nfc_manager_remove_handler(manager, 0);

    nfc_manager_unref(nfc_manager_ref(manager));
    nfc_manager_unref(manager);
}

/*==========================================================================*
 * adapter
 *==========================================================================*/

static
void
test_adapter(
    void)
{
    NfcPluginsInfo pi;
    NfcManager* manager;
    TestAdapter* test_adapter1 = test_adapter_new();
    TestAdapter* test_adapter2 = test_adapter_new();
    NfcAdapter* adapter1 = &test_adapter1->adapter;
    NfcAdapter* adapter2 = &test_adapter2->adapter;
    const char* name1;
    const char* name2;
    int count = 0;
    gulong id;

    memset(&pi, 0, sizeof(pi));
    manager = nfc_manager_new(&pi);

    /* Add adapters */
    id = nfc_manager_add_adapter_added_handler(manager,
        test_manager_adapter_inc, &count);
    name1 = nfc_manager_add_adapter(manager, adapter1);
    name2 = nfc_manager_add_adapter(manager, adapter2);
    g_assert(nfc_manager_add_adapter(manager, adapter1) == name1);
    g_assert(nfc_manager_add_adapter(manager, adapter2) == name2);
    g_assert(nfc_manager_get_adapter(manager, name1));
    g_assert(nfc_manager_get_adapter(manager, name2));
    g_assert(count == 2);
    nfc_manager_remove_handler(manager, id);
    count = 0;

    id = nfc_manager_add_enabled_changed_handler(manager,
        test_manager_inc, &count);
    g_assert(manager->enabled);
    nfc_manager_set_enabled(manager, FALSE);
    g_assert(!manager->enabled);
    g_assert(!adapter1->enabled);
    g_assert(!adapter2->enabled);
    g_assert(count == 1);
    nfc_manager_set_enabled(manager, TRUE);
    g_assert(manager->enabled);
    g_assert(adapter1->enabled);
    g_assert(adapter2->enabled);
    g_assert(count == 2);
    count = 0;
    nfc_manager_remove_handler(manager, id);

    nfc_manager_request_power(manager, TRUE);
    g_assert(test_adapter1->power_requested);
    g_assert(test_adapter2->power_requested);
    test_adapter_complete_power_request(test_adapter1);
    test_adapter_complete_power_request(test_adapter2);

    nfc_manager_request_mode(manager, NFC_MODE_READER_WRITER);
    g_assert(test_adapter1->mode_requested == NFC_MODE_READER_WRITER);
    g_assert(test_adapter2->mode_requested == NFC_MODE_READER_WRITER);

    /* Remove them */
    id = nfc_manager_add_adapter_removed_handler(manager,
        test_manager_adapter_inc, &count);
    nfc_manager_remove_adapter(manager, name1);
    nfc_manager_remove_adapter(manager, name1);
    nfc_manager_remove_adapter(manager, name2);
    nfc_manager_remove_adapter(manager, name2);
    g_assert(!nfc_manager_get_adapter(manager, name1));
    g_assert(!nfc_manager_get_adapter(manager, name2));
    g_assert(count == 2);
    nfc_manager_remove_handler(manager, id);
    count = 0;

    nfc_adapter_unref(adapter1);
    nfc_adapter_unref(adapter2);
    nfc_manager_unref(manager);
}

/*==========================================================================*
 * mode
 *==========================================================================*/

static
void
test_mode(
    void)
{
    NfcPluginsInfo pi;
    NfcManager* manager;
    int count = 0;
    gulong id;
    NfcModeRequest* enable_p2p;
    NfcModeRequest* enable_all;
    NfcModeRequest* enable_all2;
    NfcModeRequest* disable_p2p;

    memset(&pi, 0, sizeof(pi));
    manager = nfc_manager_new(&pi);
    nfc_manager_request_mode(manager, NFC_MODE_READER_WRITER);

    /* Add the listener */
    g_assert(!nfc_manager_add_mode_changed_handler(manager, NULL, NULL));
    id = nfc_manager_add_mode_changed_handler(manager,
        test_manager_inc, &count);

    /* Core is refusing to create mode requests with no mode */
    g_assert(!nfc_manager_mode_request_new(manager, 0, 0));

    /* Enable P2P modes (NFC_MODE_P2P_INITIATOR disable bit gets ignored) */
    enable_p2p = nfc_manager_mode_request_new(manager, NFC_MODES_P2P,
        NFC_MODE_P2P_INITIATOR);
    g_assert_cmpint(manager->mode, == ,NFC_MODES_P2P | NFC_MODE_READER_WRITER);
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* Disable P2P (last submitted request takes precedence) */
    disable_p2p = nfc_manager_mode_request_new(manager, 0, NFC_MODES_P2P);
    g_assert_cmpint(manager->mode, == ,NFC_MODE_READER_WRITER);
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* Add another enable request on top of that */
    enable_all = nfc_manager_mode_request_new(manager, NFC_MODES_ALL, 0);
    g_assert_cmpint(manager->mode, == , NFC_MODES_ALL);
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* And the same request (no changes are signaled this time) */
    enable_all2 = nfc_manager_mode_request_new(manager, NFC_MODES_ALL, 0);
    g_assert_cmpint(manager->mode, == , NFC_MODES_ALL);
    g_assert_cmpint(count, == ,0);

    /* Removing disable_p2p request makes no difference */
    nfc_manager_mode_request_free(disable_p2p);
    g_assert_cmpint(manager->mode, == , NFC_MODES_ALL);
    g_assert_cmpint(count, == ,0);

    /* But submitting a new one does */
    disable_p2p = nfc_manager_mode_request_new(manager, 0, NFC_MODES_P2P);
    g_assert_cmpint(manager->mode, == ,NFC_MODE_READER_WRITER |
        NFC_MODE_CARD_EMULATION);
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* And re-enabled when we release disable_p2p */
    nfc_manager_mode_request_free(disable_p2p);
    g_assert_cmpint(manager->mode, == , NFC_MODES_ALL);
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* enable_all2 remains active after we release enable_all */
    nfc_manager_mode_request_free(enable_all);
    g_assert_cmpint(manager->mode, == , NFC_MODES_ALL);
    g_assert_cmpint(count, == ,0);

    /* We are back to the default when all requests are released */
    nfc_manager_mode_request_free(enable_p2p);
    g_assert_cmpint(count, == ,0);
    nfc_manager_mode_request_free(enable_all2);
    g_assert_cmpint(manager->mode, == , NFC_MODE_READER_WRITER);
    g_assert_cmpint(count, == ,1);
    count = 0;

    nfc_manager_remove_handler(manager, id);
    nfc_manager_unref(manager);
}

/*==========================================================================*
 * tech
 *==========================================================================*/

static
void
test_tech(
    void)
{
    NfcPluginsInfo pi;
    NfcManager* manager;
    TestAdapter* adapter = test_adapter_new();
    const char* adapter_name;
    NfcTechRequest* disable_b;
    NfcTechRequest* disable_a;
    NfcTechRequest* enable_b1;
    NfcTechRequest* enable_b2;
    int count = 0;
    gulong id;

    memset(&pi, 0, sizeof(pi));
    manager = nfc_manager_new(&pi);
    g_assert_cmpint(manager->techs, == ,0);

    /* Add the listener */
    g_assert(!nfc_manager_add_techs_changed_handler(manager, NULL, NULL));
    id = nfc_manager_add_techs_changed_handler(manager,
        test_manager_inc, &count);

    adapter_name = nfc_manager_add_adapter(manager, NFC_ADAPTER(adapter));
    g_assert(adapter->allowed_techs & (NFC_TECHNOLOGY_A|NFC_TECHNOLOGY_B));
    g_assert_cmpint(manager->techs, == ,adapter->allowed_techs);
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* Useless requests are refused */
    g_assert(!nfc_manager_tech_request_new(manager, 0, 0));

    /* Disable NFC-B */
    disable_b = nfc_manager_tech_request_new(manager, 0, NFC_TECHNOLOGY_B);
    g_assert(disable_b);
    g_assert(adapter->allowed_techs & NFC_TECHNOLOGY_A);
    g_assert(!(adapter->allowed_techs & NFC_TECHNOLOGY_B));
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* Disable NFC-A too */
    disable_a = nfc_manager_tech_request_new(manager, 0, NFC_TECHNOLOGY_A);
    g_assert(disable_a);
    g_assert(!(adapter->allowed_techs & NFC_TECHNOLOGY_A));
    g_assert(!(adapter->allowed_techs & NFC_TECHNOLOGY_B));
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* Re-enable NFC-B */
    enable_b1 = nfc_manager_tech_request_new(manager, NFC_TECHNOLOGY_B, 0);
    g_assert(enable_b1);
    g_assert(!(adapter->allowed_techs & NFC_TECHNOLOGY_A));
    g_assert(adapter->allowed_techs & NFC_TECHNOLOGY_B);
    g_assert_cmpint(count, == ,1);
    count = 0;

    /* Re-enabling NFC-B again makes no difference */
    enable_b2 = nfc_manager_tech_request_new(manager, NFC_TECHNOLOGY_B, 0);
    g_assert(enable_b2);
    g_assert(!(adapter->allowed_techs & NFC_TECHNOLOGY_A));
    g_assert(adapter->allowed_techs & NFC_TECHNOLOGY_B);
    g_assert_cmpint(count, == ,0);

    /* This re-enables NFC-A */
    nfc_manager_tech_request_free(disable_a);
    g_assert(adapter->allowed_techs & NFC_TECHNOLOGY_A);
    g_assert(adapter->allowed_techs & NFC_TECHNOLOGY_B);
    g_assert_cmpint(count, == ,1);
    count = 0;

    nfc_manager_tech_request_free(disable_b);
    nfc_manager_tech_request_free(enable_b1);
    nfc_manager_tech_request_free(enable_b2);
    g_assert_cmpint(adapter->allowed_techs, == ,adapter->supported_techs);
    g_assert_cmpint(manager->techs, == ,adapter->allowed_techs);
    count = 0;

    /* Removing the last adapter zeros the techs */
    nfc_manager_remove_adapter(manager, adapter_name);
    g_assert_cmpint(manager->techs, == ,0);
    g_assert_cmpint(count, == ,1);
    count = 0;

    nfc_adapter_unref(NFC_ADAPTER(adapter));
    nfc_manager_remove_handler(manager, id);
    nfc_manager_unref(manager);
}

/*==========================================================================*
 * service
 *==========================================================================*/

static
void
test_service_cb(
    NfcManager* manager,
    NfcPeerService* service,
    void* user_data)
{
    (*(int*)user_data)++;
}

static
void
test_service(
    void)
{
    NfcPluginsInfo pi;
    NfcManager* manager;
    NfcPeerService* service = NFC_PEER_SERVICE(test_service_new("foo"));
    int registered = 0, unregistered = 0;

    memset(&pi, 0, sizeof(pi));
    manager = nfc_manager_new(&pi);

    /* Empty list by default */
    g_assert(manager->services);
    g_assert(!manager->services[0]);

    /* Some (non-zero) LLCP version must be there */
    g_assert(manager->llcp_version);

    /* Register the handlers */
    g_assert(nfc_manager_add_service_registered_handler(manager,
        test_service_cb, &registered));
    g_assert(nfc_manager_add_service_unregistered_handler(manager,
        test_service_cb, &unregistered));

    /* Register the service */
    g_assert(nfc_manager_register_service(manager, service));
    g_assert_cmpint(registered, == ,1);
    g_assert_cmpint(unregistered, == ,0);
    g_assert(manager->services[0] == service);
    g_assert(!manager->services[1]);

    /* Service can only be registered once */
    g_assert(!nfc_manager_register_service(manager, service));
    g_assert_cmpint(registered, == ,1);
    g_assert_cmpint(unregistered, == ,0);

    /* Then unregister it */
    nfc_manager_unregister_service(manager, service);
    g_assert_cmpint(registered, == ,1);
    g_assert_cmpint(unregistered, == ,1);
    g_assert(!manager->services[0]);

    /* Then again, it won't have any effect */
    nfc_manager_unregister_service(manager, service);
    g_assert_cmpint(registered, == ,1);
    g_assert_cmpint(unregistered, == ,1);
    g_assert(!manager->services[0]);

    nfc_peer_service_unref(service);
    nfc_manager_unref(manager);
}

/*==========================================================================*
 * host_service
 *==========================================================================*/

static
void
test_host_service(
    void)
{
    NfcPluginsInfo pi;
    NfcManager* manager;
    NfcHostService* service1 = NFC_HOST_SERVICE(test_host_service_new("Test1"));
    NfcHostService* service2 = NFC_HOST_SERVICE(test_host_service_new("Test2"));
    TestAdapter* test_adapter = test_adapter_new();
    NfcAdapter* adapter = NFC_ADAPTER(test_adapter);
    const char* adapter_name;

    adapter->supported_modes |= NFC_MODE_CARD_EMULATION;

    memset(&pi, 0, sizeof(pi));
    manager = nfc_manager_new(&pi);
    adapter_name = nfc_manager_add_adapter(manager, adapter);
    nfc_manager_request_power(manager, TRUE);
    test_adapter_complete_power_request(test_adapter);
    g_assert(!(adapter->mode_requested & NFC_MODE_CARD_EMULATION));

    g_assert(!nfc_manager_register_host_service(manager, NULL));

    /* Register the service */
    g_assert(nfc_manager_register_host_service(manager, service1));
    g_assert(test_adapter->mode_requested & NFC_MODE_CARD_EMULATION);
    g_assert(nfc_manager_register_host_service(manager, service2));
    g_assert(test_adapter->mode_requested & NFC_MODE_CARD_EMULATION);

    /* Service can only be registered once */
    g_assert(!nfc_manager_register_host_service(manager, service1));
    g_assert(!nfc_manager_register_host_service(manager, service2));

    /* Then unregister all services */
    nfc_manager_unregister_host_service(manager, service1);
    g_assert(test_adapter->mode_requested & NFC_MODE_CARD_EMULATION);
    nfc_manager_unregister_host_service(manager, service2);
    g_assert(!(adapter->mode_requested & NFC_MODE_CARD_EMULATION));

    /* These won't have any effect */
    nfc_manager_unregister_host_service(manager, service1);
    nfc_manager_unregister_host_service(manager, NULL);
    nfc_manager_remove_adapter(manager, adapter_name);
    nfc_manager_remove_adapter(manager, NULL);

    nfc_host_service_unref(service1);
    nfc_host_service_unref(service2);
    nfc_adapter_unref(adapter);
    nfc_manager_unref(manager);
}

/*==========================================================================*
 * host_app
 *==========================================================================*/

static
void
test_host_app(
    void)
{
    static const guchar aid1_bytes[] = { 0x01, 0x02, 0x03, 0x04 };
    static const guchar aid2_bytes[] = { 0x05, 0x06, 0x07, 0x08 };
    static const GUtilData aid1 = { TEST_ARRAY_AND_SIZE(aid1_bytes) };
    static const GUtilData aid2 = { TEST_ARRAY_AND_SIZE(aid2_bytes) };
    NfcPluginsInfo pi;
    NfcManager* manager;
    NfcHostApp* app1 = NFC_HOST_APP(test_host_app_new(&aid1, NULL, 0));
    NfcHostApp* app2 = NFC_HOST_APP(test_host_app_new(&aid2, NULL, 0));
    TestAdapter* test_adapter = test_adapter_new();
    NfcAdapter* adapter = NFC_ADAPTER(test_adapter);
    const char* adapter_name;

    adapter->supported_modes |= NFC_MODE_CARD_EMULATION;

    memset(&pi, 0, sizeof(pi));
    manager = nfc_manager_new(&pi);
    adapter_name = nfc_manager_add_adapter(manager, adapter);
    nfc_manager_request_power(manager, TRUE);
    test_adapter_complete_power_request(test_adapter);
    g_assert(!(adapter->mode_requested & NFC_MODE_CARD_EMULATION));

    g_assert(!nfc_manager_register_host_app(manager, NULL));

    /* Register the app */
    g_assert(nfc_manager_register_host_app(manager, app1));
    g_assert(test_adapter->mode_requested & NFC_MODE_CARD_EMULATION);
    g_assert(nfc_manager_register_host_app(manager, app2));
    g_assert(test_adapter->mode_requested & NFC_MODE_CARD_EMULATION);

    /* App can only be registered once */
    g_assert(!nfc_manager_register_host_app(manager, app1));
    g_assert(!nfc_manager_register_host_app(manager, app2));

    /* Then unregister all apps */
    nfc_manager_unregister_host_app(manager, app1);
    g_assert(test_adapter->mode_requested & NFC_MODE_CARD_EMULATION);
    nfc_manager_unregister_host_app(manager, app2);
    g_assert(!(adapter->mode_requested & NFC_MODE_CARD_EMULATION));

    /* These won't have any effect */
    nfc_manager_unregister_host_app(manager, app1);
    nfc_manager_unregister_host_app(manager, NULL);
    nfc_manager_remove_adapter(manager, adapter_name);
    nfc_manager_remove_adapter(manager, NULL);

    nfc_host_app_unref(app1);
    nfc_host_app_unref(app2);
    nfc_adapter_unref(adapter);
    nfc_manager_unref(manager);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(name) "/core/manager/" name

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("null"), test_null);
    g_test_add_func(TEST_("basic"), test_basic);
    g_test_add_func(TEST_("adapter"), test_adapter);
    g_test_add_func(TEST_("mode"), test_mode);
    g_test_add_func(TEST_("tech"), test_tech);
    g_test_add_func(TEST_("service"), test_service);
    g_test_add_func(TEST_("host_service"), test_host_service);
    g_test_add_func(TEST_("host_app"), test_host_app);
    test_init(&test_opt, argc, argv);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
