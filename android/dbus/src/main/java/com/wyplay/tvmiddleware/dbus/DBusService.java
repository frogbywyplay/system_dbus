/* Copyright (C) 2021 Wyplay, All Rights Reserved.
 * This source code and any compilation or derivative thereof is the
 * proprietary information of Wyplay and is confidential in nature.
 * Under no circumstances is this software to be exposed to or placed under
 * an Open Source License of any type without the expressed written permission
 * of Wyplay.
 */

package com.wyplay.tvmiddleware.dbus;

import android.util.Log;

import com.wyplay.tools.services.BaseService;

public class DBusService extends BaseService {
    public DBusService() {
        super(new DBusServiceInfo());
    }

    @Override
    protected synchronized void start() {
        Log.i(TAG(), String.format("Starting \"%s\" (version %s)...",
                getServiceInfo().getName(), DBusNative.getVersion()));

        final String configFile = getServiceInfo().getCmdlineArgs().get("config-file");
        DBusNative.start(this, configFile);
    }

    @Override
    protected synchronized void stop() {
        Log.i(TAG(), String.format("Stopping \"%s\"...", getServiceInfo().getName()));
        DBusNative.stop(this);
    }
}
