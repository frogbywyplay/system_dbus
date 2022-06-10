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

#ifndef __DBUS_DAEMON_H__
#define __DBUS_DAEMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Used to asynchronously start the D-Bus service
 * (the mainloop is running from a separated thread).
 *
 * @param[in] config_file Path of the D-Bus config file.
 * @return 0 in case of success. Another value otherwise.
 */
int dbus_daemon_start(const char* config_file);

/**
 * @brief Used to interrupt and stop the D-Bus service
 * (previously started using @dbus_daemon_start).
 *
 * @return 0 in case of success. Another value otherwise.
 */
int dbus_daemon_stop(void);

/**
 * Used to retrieve the version of the D-Bus library.
 *
 * @return String
 */
const char* dbus_daemon_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* __DBUS_DAEMON_H__ */
