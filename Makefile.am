ACLOCAL_AMFLAGS= -I m4

SUBDIRS = src

pcfiles = mm-log.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

conffiles = logmessage.conf
logconfigdir = $(LOG_FILE_DIR)
logconfig_DATA = $(conffiles)

EXTRA_DIST = $(pcfiles) $(conffiles)
