AC_PREREQ(2.52)

AC_INIT([libmm-log], [0.1])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h:config.hin])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_C_CONST
dnl AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_TIME
AC_PROG_GCC_TRADITIONAL
AC_PROG_LIBTOOL

#PLATFORM_INIT

LOGMANAGER_LIBVERSION="0:0:0"
AC_SUBST(LOGMANAGER_LIBVERSION)




##### Disable debug-message-service :+:091130 ######
#PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1 >= 0.7)
#AC_SUBST(DBUS_GLIB_CFLAGS)
#AC_SUBST(DBUS_GLIB_LIBS)

#PKG_CHECK_MODULES(GLIB, glib-2.0)
#AC_SUBST(GLIB_CFLAGS)
#AC_SUBST(GLIB_LIBS)

#PKG_CHECK_MODULES(DEBUG_MESSAGE_SERVICE,debug-message-service)
#AC_SUBST(DEBUG_MESSAGE_SERVICE_CFLAGS)
#AC_SUBST(DEBUG_MESSAGE_SERVICE_LIBS)
#####################################################


dnl use dlog --------------------------------------------------------------------------
AC_ARG_ENABLE(dlog, AC_HELP_STRING([--enable-dlog], [using dlog]),
[
 case "${enableval}" in
	 yes) USE_DLOG=yes ;;
	 no)  USE_DLOG=no ;;
	 *)   AC_MSG_ERROR(bad value ${enableval} for --enable-dlog) ;;
 esac
],[USE_DLOG=yes])

if test "x$USE_DLOG" = "xyes"; then
	PKG_CHECK_MODULES(DLOG, dlog)
	AC_SUBST(DLOG_CFLAGS)
	AC_SUBST(DLOG_LIBS)
fi
AM_CONDITIONAL(USE_DLOG, test "x$USE_DLOG" = "xyes")
dnl end --------------------------------------------------------------------

# logmanager -- setting SECTION .. hyunil46.park@samsung.com
#[setting]
log_direction="$direction"
AC_SUBST([log_direction])
log_ownermask="$ownermask"
AC_SUBST([log_ownermask])
log_classmask="$classmask"
AC_SUBST([log_classmask])
log_filename="$filename"
AC_SUBST([log_filename])
#[color] 
log_classinfomation="$classinfomation"
AC_SUBST([log_classinfomation])
log_classwarning="$classwarning"
AC_SUBST([log_classwarning])
log_classerror="$classerror"
AC_SUBST([log_classerror])
log_clsascritical="$clsascritical"
AC_SUBST([log_clsascritical])
log_classassert="$classassert"
AC_SUBST([log_classassert])
#


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h memory.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([memset select])

LOG_FILE_DIR=/opt/etc
LOG_FILE_NAME=logmessage.conf

LOG_INI_FILE="$LOG_FILE_DIR/$LOG_FILE_NAME"

AC_SUBST([LOG_INI_FILE])
AC_SUBST([LOG_FILE_DIR])
AC_SUBST([LOG_FILE_NAME])
AC_DEFINE_UNQUOTED([LOG_INI_FILE], "$LOG_INI_FILE" , [log ini file position])

#AM_CONDITIONAL([ISPROTECTOR_VODA_SDK], [test "x$DISTRO" = "xvodafone-sdk" && test "x$MACHINE" = "xprotector"])

AC_CONFIG_FILES([
		Makefile
		src/Makefile
		mm-log.pc
		logmessage.conf
])
AC_OUTPUT
