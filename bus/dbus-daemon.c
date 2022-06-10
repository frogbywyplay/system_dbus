/*
 * Copyright (C) 2003, 2022 Red Hat, Inc.
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "dbus-daemon.h"
#include "dbus-unix.h"
#include "dbus/dbus-internals.h"

#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifndef DBUS_UNIX
#error "DBUS_UNIX support is required"
#endif

static BusContext *g_context = NULL;
static pthread_t g_thread;

static void* dbus_mainloop_thread(void* arg)
{
    BusContext *context = (BusContext*)arg;

    _dbus_verbose("Starting D-Bus main loop...\n");
    _dbus_loop_run(bus_context_get_loop(context));
    _dbus_verbose("D-Bus main loop stopped.\n");

    return NULL;
}

int dbus_daemon_start(const char* config_file)
{
    BusContextFlags flags = BUS_CONTEXT_FLAG_NONE;
    DBusError error;
    DBusString dbus_config;
    int rc = 0;

    if (g_context)
        return 0;

    _dbus_return_val_if_fail(config_file != NULL, -1);

    _dbus_verbose("Initializing D-Bus...\n");
    _dbus_verbose("- config:  %s\n", config_file);

    /* Initialize D-Bus data structure */
    if (!_dbus_string_init(&dbus_config)) {
        _dbus_warn("Failed to init DBusConfig parameter.\n");
        goto err_dbus_string_config_init;
    }
    if (!_dbus_string_append(&dbus_config, config_file)) {
        _dbus_warn("Failed to init DBusConfig parameters.\n");
        goto err_dbus_string_config_append;
    }

    /* Create D-Bus context */
    dbus_error_init(&error);
    flags |= BUS_CONTEXT_FLAG_FORK_NEVER;
    g_context = bus_context_new(&dbus_config, flags, NULL, NULL, NULL, &error);
    if (!g_context) {
        _dbus_warn("Failed to create the D-Bus context (%s)\n", error.message);
        goto err_dbus_context_new;
    }

    /* Setup D-Bus unix pipe used to interrupt the main loop. */
    dbus_unix_setup_reload_pipe(g_context);

    /* Start the D-Bus mainloop thread */
    if ((rc = pthread_create(&g_thread, NULL, dbus_mainloop_thread, g_context)) < 0) {
        _dbus_warn("Failed to create the D-Bus main loop thread (%s)\n", strerror(rc));
        goto err_dbus_thread_create;
    }

    /* Release resources */
    _dbus_string_free(&dbus_config);

    return 0;

err_dbus_thread_create:
    bus_context_unref(g_context);
    g_context = NULL;
err_dbus_context_new:
    dbus_error_free(&error);
err_dbus_string_config_append:
    _dbus_string_free(&dbus_config);
err_dbus_string_config_init:
    return -1;
}

int dbus_daemon_stop()
{
    if (!g_context)
        return 0;

    /* Interrupt the D-Bus main loop */
    dbus_unix_action_quit();
    pthread_join(g_thread, NULL);

    /* Release resources */
    bus_context_unref(g_context);
    g_context = NULL;

    _dbus_verbose("D-Bus service stopped.\n");

    return 0;
}

const char* dbus_daemon_get_version()
{
    return PACKAGE_VERSION;
}
