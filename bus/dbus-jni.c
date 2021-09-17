/*
 * Copyright (C) 2021 Wyplay, All Rights Reserved.
 * This source code and any compilation or derivative thereof is the
 * proprietary information of Wyplay and is confidential in nature.
 * Under no circumstances is this software to be exposed to or placed under
 * an Open Source License of any type without the expressed written permission
 * of Wyplay.
 */

#include "config.h"
#include "dbus-jni.h"
#include "dbus-unix.h"

#include <android/log.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifndef DBUS_UNIX
#error "DBUS_UNIX support is required"
#endif

#define TAG "dbus-jni"

#define __LOG__(lvl, msg, ...) \
    __android_log_print(lvl, TAG, msg, ##__VA_ARGS__)
#define LOG_DBG(msg, ...) \
    __LOG__(ANDROID_LOG_DEBUG, msg, ##__VA_ARGS__)
#define LOG_INF(msg, ...) \
    __LOG__(ANDROID_LOG_INFO, msg, ##__VA_ARGS__)
#define LOG_ERR(msg, ...) \
    __LOG__(ANDROID_LOG_ERROR, msg, ##__VA_ARGS__)


static BusContext *g_context = NULL;
static pthread_t g_thread;

static void* dbus_mainloop_thread(void* arg)
{
    BusContext *context = (BusContext*)arg;

    LOG_INF("Starting D-Bus main loop...");
    _dbus_loop_run(bus_context_get_loop(context));
    LOG_INF("D-Bus main loop stopped.");

    return NULL;
}

JNIEXPORT jboolean JNICALL Java_com_wyplay_tvmiddleware_dbus_DBusNative_00024Native_start
  (JNIEnv *env, jclass cl, jobject ctx, jstring jconfig)
{
    BusContextFlags flags = BUS_CONTEXT_FLAG_NONE;
    DBusError error;
    DBusString dbus_config;
    const char *config = NULL;
    int rc = 0;

    if (g_context)
        return JNI_TRUE;

    if (!jconfig) {
        LOG_ERR("Null parameter.");
        return JNI_FALSE;
    }

    config = (*env)->GetStringUTFChars(env, jconfig, NULL);

    LOG_INF("Initializing D-Bus...");
    LOG_DBG("- config:  %s", config);

    /* Initialize D-Bus data structure */
    if (!_dbus_string_init(&dbus_config)) {
        LOG_ERR("Failed to init DBusConfig parameter.");
        goto err_dbus_string_config_init;
    }
    if (!_dbus_string_append(&dbus_config, config)) {
        LOG_ERR("Failed to init DBusConfig parameters.");
        goto err_dbus_string_config_append;
    }

    /* Create D-Bus context */
    dbus_error_init(&error);
    flags |= BUS_CONTEXT_FLAG_FORK_NEVER;
    g_context = bus_context_new(&dbus_config, flags, NULL, NULL, NULL, &error);
    if (!g_context) {
        LOG_ERR("Failed to create the D-Bus context (%s)", error.message);
        goto err_dbus_context_new;
    }

    /* Setup D-Bus unix pipe used to interrupt the main loop. */
    dbus_unix_setup_reload_pipe(g_context);

    /* Start the D-Bus mainloop thread */
    if ((rc = pthread_create(&g_thread, NULL, dbus_mainloop_thread, g_context)) < 0) {
        LOG_ERR("Failed to create the D-Bus main loop thread (%s)", strerror(rc));
        goto err_dbus_thread_create;
    }

    /* Release resources */
    _dbus_string_free(&dbus_config);
    (*env)->ReleaseStringUTFChars(env, jconfig, config);

    return JNI_TRUE;

err_dbus_thread_create:
    bus_context_unref(g_context);
    g_context = NULL;
err_dbus_context_new:
    dbus_error_free(&error);
err_dbus_string_config_append:
    _dbus_string_free(&dbus_config);
err_dbus_string_config_init:
    (*env)->ReleaseStringUTFChars(env, jconfig, config);
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_wyplay_tvmiddleware_dbus_DBusNative_00024Native_stop
  (JNIEnv *env, jclass cl, jobject ctx)
{
    if (!g_context)
        return JNI_TRUE;

    /* Interrupt the D-Bus main loop */
    dbus_unix_action_quit();
    pthread_join(g_thread, NULL);

    /* Release resources */
    bus_context_unref(g_context);
    g_context = NULL;

    LOG_INF("D-Bus service stopped.");

    return JNI_TRUE;
}

JNIEXPORT jstring JNICALL Java_com_wyplay_tvmiddleware_dbus_DBusNative_00024Native_getVersion
  (JNIEnv *env, jclass cl)
{
    return (*env)->NewStringUTF(env, PACKAGE_VERSION);
}
