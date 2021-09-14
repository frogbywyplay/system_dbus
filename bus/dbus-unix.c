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

#include "config.h"
#include "dbus-unix.h"

#include <stdlib.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>     /* for write() and STDERR_FILENO */
#endif

#include <dbus/dbus-internals.h>
#include <dbus/dbus-watch.h>

static int reload_pipe[2];

#define RELOAD_READ_END 0
#define RELOAD_WRITE_END 1

typedef enum
 {
   ACTION_RELOAD = 'r',
   ACTION_QUIT = 'q'
 } SignalAction;

static void dbus_unix_close_reload_pipe(BusContext *context, DBusWatch **watch)
{
    _dbus_loop_remove_watch (bus_context_get_loop (context), *watch);
    _dbus_watch_invalidate (*watch);
    _dbus_watch_unref (*watch);
    *watch = NULL;

    _dbus_close_socket (reload_pipe[RELOAD_READ_END], NULL);
    reload_pipe[RELOAD_READ_END] = -1;

    _dbus_close_socket (reload_pipe[RELOAD_WRITE_END], NULL);
    reload_pipe[RELOAD_WRITE_END] = -1;
}

static dbus_bool_t dbus_unix_handle_reload_watch(
    DBusWatch *watch, unsigned int flags, void *data)
{
  DBusError error;
  DBusString str;
  BusContext *context = (BusContext*)data;
  char *action_str;
  char action = '\0';

  while (!_dbus_string_init (&str))
    _dbus_wait_for_memory ();

  if ((reload_pipe[RELOAD_READ_END] > 0) &&
      _dbus_read_socket (reload_pipe[RELOAD_READ_END], &str, 1) != 1)
    {
      _dbus_warn ("Couldn't read from reload pipe.\n");
      dbus_unix_close_reload_pipe (context, &watch);
      return TRUE;
    }

  action_str = _dbus_string_get_data (&str);
  if (action_str != NULL)
    {
      action = action_str[0];
    }
  _dbus_string_free (&str);

  /* this can only fail if we don't understand the config file
   * or OOM.  Either way we should just stick with the currently
   * loaded config.
   */
  dbus_error_init (&error);

  switch (action)
    {
    case ACTION_RELOAD:
      if (! bus_context_reload_config (context, &error))
        {
          _DBUS_ASSERT_ERROR_IS_SET (&error);
          _dbus_assert (dbus_error_has_name (&error, DBUS_ERROR_FAILED) ||
                        dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY));
          _dbus_warn ("Unable to reload configuration: %s\n",
                      error.message);
          dbus_error_free (&error);
        }
      break;

    case ACTION_QUIT:
      {
        DBusLoop *loop;
        /*
         * On OSs without abstract sockets, we want to quit
         * gracefully rather than being killed by SIGTERM,
         * so that DBusServer gets a chance to clean up the
         * sockets from the filesystem. fd.o #38656
         */
        loop = bus_context_get_loop (context);
        if (loop != NULL)
          {
            _dbus_loop_quit (loop);
          }
      }
      break;

    default:
      break;
    }

  return TRUE;
}

void dbus_unix_setup_reload_pipe(BusContext *context)
{
  DBusError error;
  DBusWatch *watch;
  DBusLoop *loop = bus_context_get_loop(context);

  dbus_error_init (&error);

  if (!_dbus_full_duplex_pipe (&reload_pipe[0], &reload_pipe[1],
			       TRUE, &error))
    {
      _dbus_warn ("Unable to create reload pipe: %s\n",
		  error.message);
      dbus_error_free (&error);
      exit (1);
    }

  watch = _dbus_watch_new (reload_pipe[RELOAD_READ_END],
			   DBUS_WATCH_READABLE, TRUE,
			   dbus_unix_handle_reload_watch, context, NULL);

  if (watch == NULL)
    {
      _dbus_warn ("Unable to create reload watch: %s\n",
		  error.message);
      dbus_error_free (&error);
      exit (1);
    }

  if (!_dbus_loop_add_watch (loop, watch))
    {
      _dbus_warn ("Unable to add reload watch to main loop: %s\n",
		  error.message);
      dbus_error_free (&error);
      exit (1);
    }

}

void dbus_unix_signal_handler(int sig)
{
  switch (sig)
    {
#ifdef DBUS_BUS_ENABLE_DNOTIFY_ON_LINUX
    case SIGIO:
      /* explicit fall-through */
#endif /* DBUS_BUS_ENABLE_DNOTIFY_ON_LINUX  */
#ifdef SIGHUP
    case SIGHUP:
      {
        DBusString str;
        char action[2] = { ACTION_RELOAD, '\0' };

        _dbus_string_init_const (&str, action);
        if ((reload_pipe[RELOAD_WRITE_END] > 0) &&
            !_dbus_write_socket (reload_pipe[RELOAD_WRITE_END], &str, 0, 1))
          {
            /* If we receive SIGHUP often enough to fill the pipe buffer (4096
             * times on old Linux, 65536 on modern Linux) before it can be
             * drained, let's just warn and ignore. The configuration will be
             * reloaded while draining the pipe buffer, which is what we
             * wanted. It's harmless that it will be reloaded fewer times than
             * we asked for, since the reload is delayed anyway, so new changes
             * will be picked up.
             *
             * We use write() because _dbus_warn uses vfprintf, which isn't
             * async-signal-safe.
             *
             * This is necessarily Unix-specific, but so are POSIX signals,
             * so... */
            static const char message[] =
              "Unable to write to reload pipe - buffer full?\n";

            if (write (STDERR_FILENO, message, strlen (message)) != strlen (message))
              {
                /* ignore failure to write out a warning */
              }
          }
      }
      break;
#endif

    case SIGTERM:
      {
        DBusString str;
        char action[2] = { ACTION_QUIT, '\0' };
        _dbus_string_init_const (&str, action);
        if ((reload_pipe[RELOAD_WRITE_END] < 0) ||
            !_dbus_write_socket (reload_pipe[RELOAD_WRITE_END], &str, 0, 1))
          {
            /* If we can't write to the socket, dying seems a more
             * important response to SIGTERM than cleaning up sockets,
             * so we exit. We'd use exit(), but that's not async-signal-safe,
             * so we'll have to resort to _exit(). */
            static const char message[] =
              "Unable to write termination signal to pipe - buffer full?\n"
              "Will exit instead.\n";

            if (write (STDERR_FILENO, message, strlen (message)) != strlen (message))
              {
                /* ignore failure to write out a warning */
              }
            _exit (1);
          }
      }
      break;
    }
}
