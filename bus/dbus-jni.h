/*
 * Copyright (C) 2021 Wyplay, All Rights Reserved.
 * This source code and any compilation or derivative thereof is the
 * proprietary information of Wyplay and is confidential in nature.
 * Under no circumstances is this software to be exposed to or placed under
 * an Open Source License of any type without the expressed written permission
 * of Wyplay.
 */

#include <jni.h>

#ifndef __DBUS_JNI_H__
#define __DBUS_JNI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_wyplay_tvmiddleware_dbus_DBusNative_Native
 * Method:    start
 * Signature: (Landroid/content/Context;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_wyplay_tvmiddleware_dbus_DBusNative_00024Native_start
  (JNIEnv *, jclass, jobject, jstring);

/*
 * Class:     com_wyplay_tvmiddleware_dbus_DBusNative_Native
 * Method:    stop
 * Signature: (Landroid/content/Context;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_wyplay_tvmiddleware_dbus_DBusNative_00024Native_stop
  (JNIEnv *, jclass, jobject);

/*
 * Class:     com_wyplay_tvmiddleware_dbus_DBusNative_Native
 * Method:    getVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_wyplay_tvmiddleware_dbus_DBusNative_00024Native_getVersion
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif

#endif /* __DBUS_JNI_H__ */
