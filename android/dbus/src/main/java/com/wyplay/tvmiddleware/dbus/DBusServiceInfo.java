/* Copyright (C) 2021 Wyplay, All Rights Reserved.
 * This source code and any compilation or derivative thereof is the
 * proprietary information of Wyplay and is confidential in nature.
 * Under no circumstances is this software to be exposed to or placed under
 * an Open Source License of any type without the expressed written permission
 * of Wyplay.
 */

package com.wyplay.tvmiddleware.dbus;

import com.google.auto.service.AutoService;
import com.wyplay.tools.services.BaseServiceInfo;
import com.wyplay.tools.services.ServiceInfo;

@AutoService(ServiceInfo.class)
public class DBusServiceInfo extends BaseServiceInfo {
    @Override
    public String getClassName() {
        return "com.wyplay.tvmiddleware.dbus.DBusService";
    }

    @Override
    public String getConfigFilename() {
        return "dbus.xml";
    }
}
