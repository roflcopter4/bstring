#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Initialize autoconf
AC_PREREQ([2.69])

# Library versioning
# Making releases:
#	micro_version += 1
#	interface_age += 1
# If functions have been added or backward-compatibility has been broken:
#	interface_age = 0
m4_define([major_version], [1])
m4_define([minor_version], [0])
m4_define([micro_version], [1])
m4_define([interface_age], [0])

# Initialize autoconf & automake
AC_INIT([Bstring], [major_version.minor_version.micro_version],
        [https://github.com/msteinert/bstring/issues], [bstring])
AM_INIT_AUTOMAKE([1.9 subdir-objects foreign -Wno-portability])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR([bstring/bstring.h])

# Initialize Libtool
LT_PREREQ([2.2])
LT_INIT

# Check the build environment
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_MKDIR_P

# Libtool versioning
LT_CURRENT=m4_eval(100 * minor_version + micro_version - interface_age)
LT_REVISION=interface_age
LT_AGE=m4_eval(m4_eval(100 * minor_version + micro_version) - interface_age)
AC_SUBST([LT_CURRENT])
AC_SUBST([LT_REVISION])
AC_SUBST([LT_AGE])

AC_DEFINE([_GNU_SOURCE], [], [For asprintf])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h limits.h stddef.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_CHECK_TYPES([ptrdiff_t])
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memchr memmove memset strcasecmp strstr vasprintf err])

# Find talloc
AC_CHECK_LIB([talloc], [_talloc], [], [echo 'Talloc is required' >&2; exit 1])

# Output files & status
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	bstring.pc
	doc/Doxyfile
	Makefile
])
AC_OUTPUT
AC_MSG_RESULT([
$PACKAGE_NAME build options:
	Host: $host
	Version: $VERSION
])
