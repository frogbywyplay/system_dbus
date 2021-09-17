/* Copyright (C) 2021 Wyplay, All Rights Reserved.
 * This source code and any compilation or derivative thereof is the
 * proprietary information of Wyplay and is confidential in nature.
 * Under no circumstances is this software to be exposed to or placed under
 * an Open Source License of any type without the expressed written permission
 * of Wyplay.
 */

package com.wyplay.tvmiddleware.dbus;

import android.content.Context;

public final class DBusNative {
    private static class Native {
        static {
            System.loadLibrary("dbus-jni");
        }
        private static native boolean start(Context ctx, String configFile);
        private static native boolean stop(Context ctx);
        private static native String getVersion();
    }

    static boolean start(Context ctx, String configFile) {
        if (ctx == null)
            throw new IllegalArgumentException("context");
        return Native.start(ctx, configFile);
    }

    static boolean stop(Context ctx) {
        if (ctx == null)
            throw new IllegalArgumentException("context");
        return Native.stop(ctx);
    }

    static String getVersion() {
        return Native.getVersion();
    }
}