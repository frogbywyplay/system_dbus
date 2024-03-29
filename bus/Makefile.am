configdir=$(sysconfdir)/dbus-1

DBUS_BUS_LIBS = \
	$(XML_LIBS) \
	$(SELINUX_LIBS) \
	$(THREAD_LIBS) \
	$(ADT_LIBS) \
	$(NETWORK_libs) \
	$(NULL)

DBUS_LAUNCHER_LIBS = \
	$(XML_LIBS) \
	$(THREAD_LIBS) \
	$(NETWORK_libs) \
	$(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(XML_CFLAGS) \
	-DDBUS_SYSTEM_CONFIG_FILE=\""$(configdir)/system.conf"\" \
	-DDBUS_COMPILATION \
	-DDBUS_STATIC_BUILD \
	$(NULL)

# if assertions are enabled, improve backtraces
AM_LDFLAGS = @R_DYNAMIC_LDFLAG@

CONFIG_IN_FILES=				\
	session.conf.in				\
	system.conf.in				\
	org.freedesktop.dbus-session.plist.in

config_DATA=					\
	session.conf				\
	system.conf

if DBUS_ENABLE_LAUNCHD
agentdir=$(LAUNCHD_AGENT_DIR)
agent_DATA=org.freedesktop.dbus-session.plist
endif

if DBUS_USE_LIBXML
XML_SOURCES=config-loader-libxml.c
endif
if DBUS_USE_EXPAT
XML_SOURCES=config-loader-expat.c
endif

if DBUS_BUS_ENABLE_KQUEUE
DIR_WATCH_SOURCE=dir-watch-kqueue.c
else
if DBUS_BUS_ENABLE_INOTIFY
DIR_WATCH_SOURCE=dir-watch-inotify.c
else
if DBUS_BUS_ENABLE_DNOTIFY_ON_LINUX
DIR_WATCH_SOURCE=dir-watch-dnotify.c
else
DIR_WATCH_SOURCE=dir-watch-default.c
endif
endif
endif

BUS_SOURCES=					\
	activation.c				\
	activation.h				\
	activation-exit-codes.h			\
	bus.c					\
	bus.h					\
	config-parser.c				\
	config-parser.h				\
	config-parser-common.c			\
	config-parser-common.h			\
	connection.c				\
	connection.h				\
	desktop-file.c				\
	desktop-file.h				\
	$(DIR_WATCH_SOURCE)			\
	dir-watch.h				\
	dispatch.c				\
	dispatch.h				\
	driver.c				\
	driver.h				\
	expirelist.c				\
	expirelist.h				\
	policy.c				\
	policy.h				\
	selinux.h				\
	selinux.c				\
	services.c				\
	services.h				\
	signals.c				\
	signals.h				\
	stats.c					\
	stats.h					\
	test.c					\
	test.h					\
	utils.c					\
	utils.h					\
	$(XML_SOURCES)

if DBUS_UNIX
BUS_SOURCES += dbus-unix.c
endif

LAUNCH_HELPER_SOURCES=				\
	$(XML_SOURCES)				\
	config-parser-common.c			\
	config-parser-common.h			\
	config-parser-trivial.c			\
	config-parser-trivial.h			\
	desktop-file.c				\
	desktop-file.h				\
	utils.c					\
	utils.h					\
	activation-exit-codes.h			\
	activation-helper.h	  		\
	activation-helper.c

## This is the installed launch helper with the setuid checks
dbus_daemon_launch_helper_SOURCES=		\
	activation-helper-bin.c			\
	$(LAUNCH_HELPER_SOURCES)

dbus_daemon_launch_helper_LDADD=		\
	$(top_builddir)/dbus/libdbus-internal.la \
	$(DBUS_LAUNCHER_LIBS)

if ENABLE_LXC_GENESIS
dbus_daemon_launch_helper_LDADD+=$(lxc_genesis_core_LIBS)
endif ENABLE_LXC_GENESIS

## we build another binary so we can do the launch testing without root privs.
## DO NOT INSTALL THIS FILE
dbus_daemon_launch_helper_test_SOURCES=		\
	activation-helper-bin.c			\
	$(LAUNCH_HELPER_SOURCES)

dbus_daemon_launch_helper_test_LDADD=		\
	$(top_builddir)/dbus/libdbus-internal.la \
	$(DBUS_LAUNCHER_LIBS)

if ENABLE_LXC_GENESIS
dbus_daemon_launch_helper_test_LDADD+=$(lxc_genesis_core_LIBS)
endif ENABLE_LXC_GENESIS

dbus_daemon_launch_helper_test_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DACTIVATION_LAUNCHER_TEST

## we build yet another binary so we can do the OOM tests
## DO NOT INSTALL THIS FILE
bus_test_launch_helper_SOURCES=		\
	test-launch-helper.c   		\
	$(LAUNCH_HELPER_SOURCES)

bus_test_launch_helper_LDADD=		\
	$(top_builddir)/dbus/libdbus-internal.la \
	$(DBUS_LAUNCHER_LIBS)

if ENABLE_LXC_GENESIS
bus_test_launch_helper_LDADD+=$(lxc_genesis_core_LIBS)
endif ENABLE_LXC_GENESIS

bus_test_launch_helper_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DACTIVATION_LAUNCHER_TEST	\
	-DACTIVATION_LAUNCHER_DO_OOM

if ENABLE_DAEMON_LIBRARY
lib_LTLIBRARIES = libdbus-1-daemon.la

libdbus_1_daemon_la_SOURCES = $(BUS_SOURCES) dbus-daemon.c
libdbus_1_daemon_la_CPPFLAGS = -fPIC -DPIC $(AM_CPPFLAGS)
libdbus_1_daemon_la_LDFLAGS = -module -avoid-version
libdbus_1_daemon_la_LIBADD = $(top_builddir)/dbus/libdbus-internal.la $(DBUS_BUS_LIBS)

dbusincludedir=$(includedir)/dbus-1.0/dbus
dbusinclude_HEADERS = dbus-daemon.h
else
dbus_daemon_execdir = $(DBUS_DAEMONDIR)
dbus_daemon_exec_PROGRAMS = dbus-daemon

dbus_daemon_SOURCES = $(BUS_SOURCES) main.c
dbus_daemon_LDADD = $(top_builddir)/dbus/libdbus-internal.la $(DBUS_BUS_LIBS)

if ENABLE_LXC_GENESIS
dbus_daemon_LDADD+=$(lxc_genesis_core_LIBS)
dbus_daemon_CFLAGS=-fPIE
dbus_daemon_LDFLAGS=-pie
endif ENABLE_LXC_GENESIS

if DBUS_UNIX
libexec_PROGRAMS = dbus-daemon-launch-helper

install-exec-hook:
	if test `id -u` -eq 0; then \
		chown root:$(DBUS_USER) $(DESTDIR)$(libexecdir)/dbus-daemon-launch-helper$(EXEEXT); \
		chmod 4750 $(DESTDIR)$(libexecdir)/dbus-daemon-launch-helper$(EXEEXT); \
	else \
		echo "Not installing $(DESTDIR)$(libexecdir)/dbus-daemon-launch-helper binary setuid!"; \
		echo "You'll need to manually set permissions to root:$(DBUS_USER) and permissions 4750"; \
	fi
endif DBUS_UNIX
endif

## Note that TESTS has special meaning (stuff to use in make check).
## We don't actually want to run any of these tests until test/ has been
## compiled, so we don't put them in TESTS here; we run them in test/
## instead.

noinst_PROGRAMS =
if DBUS_BUILD_TESTS
## we use noinst_PROGRAMS not check_PROGRAMS so that we build
## even when not doing "make check"

# run as a test by test/Makefile.am
noinst_PROGRAMS += bus-test bus-test-system

if DBUS_UNIX
# run as a test by test/Makefile.am
noinst_PROGRAMS += bus-test-launch-helper
# this is used by the tests but is not,itself, a test
noinst_PROGRAMS += dbus-daemon-launch-helper-test
endif DBUS_UNIX

endif DBUS_BUILD_TESTS

bus_test_system_SOURCES=			\
	$(XML_SOURCES)				\
	config-parser-common.c			\
	config-parser-common.h			\
	config-parser-trivial.c			\
	config-parser-trivial.h			\
	utils.c					\
	utils.h					\
	test-system.c

bus_test_system_LDADD=$(top_builddir)/dbus/libdbus-internal.la $(DBUS_BUS_LIBS)

if ENABLE_LXC_GENESIS
bus_test_system_LDADD+=$(lxc_genesis_core_LIBS)
endif ENABLE_LXC_GENESIS

bus_test_SOURCES=				\
	$(BUS_SOURCES)				\
	test-main.c

bus_test_LDADD=$(top_builddir)/dbus/libdbus-internal.la $(DBUS_BUS_LIBS)

if ENABLE_LXC_GENESIS
bus_test_LDADD+=$(lxc_genesis_core_LIBS)
endif ENABLE_LXC_GENESIS

## mop up the gcov files
clean-local:
	/bin/rm *.bb *.bbg *.da *.gcov || true

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/run/dbus
	$(mkinstalldirs) $(DESTDIR)$(configdir)/system.d
	$(mkinstalldirs) $(DESTDIR)$(configdir)/session.d
	$(mkinstalldirs) $(DESTDIR)$(datadir)/dbus-1/services
	$(mkinstalldirs) $(DESTDIR)$(datadir)/dbus-1/system-services
if HAVE_SYSTEMD
# Install dbus.socket as default implementation of a D-Bus stack.
# Deliberately not using $(LN_S) here: ln -fs is not universally portable,
# but neither is systemd, so it's OK to assume here that ln complies with SUS.
	$(mkinstalldirs) $(DESTDIR)$(systemdsystemunitdir)/dbus.target.wants
	ln -fs ../dbus.socket $(DESTDIR)$(systemdsystemunitdir)/dbus.target.wants/dbus.socket
# Unconditionally enable D-Bus on systemd installations
	$(mkinstalldirs) $(DESTDIR)$(systemdsystemunitdir)/sockets.target.wants
	ln -fs ../dbus.socket $(DESTDIR)$(systemdsystemunitdir)/sockets.target.wants/dbus.socket
	$(mkinstalldirs) $(DESTDIR)$(systemdsystemunitdir)/multi-user.target.wants
	ln -fs ../dbus.service $(DESTDIR)$(systemdsystemunitdir)/multi-user.target.wants/dbus.service
endif

#### Init scripts fun
SCRIPT_IN_FILES=messagebus.in \
		messagebus-config.in \
		rc.messagebus.in

## Red Hat start
if DBUS_INIT_SCRIPTS_RED_HAT

initddir=$(sysconfdir)/rc.d/init.d

initd_SCRIPTS= 	\
	messagebus

endif
 ## Red Hat end

## Slackware start
if DBUS_INIT_SCRIPTS_SLACKWARE

initddir=$(sysconfdir)/rc.d/

initd_SCRIPTS= 	\
	rc.messagebus

endif
## Slackware end

## Cygwin start
if DBUS_INIT_SCRIPTS_CYGWIN

bin_SCRIPTS= 	\
	messagebus-config

endif
## Cygwin end

if HAVE_SYSTEMD
SCRIPT_IN_FILES += \
	dbus.service.in \
	dbus.socket.in

systemdsystemunit_DATA = \
	dbus.service \
	dbus.socket
endif

#### Extra dist

EXTRA_DIST=$(CONFIG_IN_FILES) $(SCRIPT_IN_FILES)
