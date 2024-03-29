/*
 * Copyright (C) 2003, 2021 Red Hat, Inc.
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

#ifndef __DBUS_UNIX_H__
#define __DBUS_UNIX_H__

#include "bus.h"

void dbus_unix_setup_reload_pipe(BusContext *context);
void dbus_unix_action_reload(void);
void dbus_unix_action_quit(void);
void dbus_unix_signal_handler(int sig);

#endif /* __DBUS_UNIX_H__ */

