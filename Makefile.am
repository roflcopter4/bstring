#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -I$(top_srcdir)/bstring

lib_LTLIBRARIES = bstring/libbstring.la

bstring_libbstring_la_SOURCES = \
    bstring/additions.c         \
    bstring/b_list.c            \
    bstring/bstrlib.c

bstring_libbstring_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

# install development headers
pkgincludedir = $(includedir)/bstring
pkginclude_HEADERS =    \
    bstring/bstring.h   \
    bstring/additions.h \
    bstring/defines.h

# Install pkg-config scripts
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bstring.pc

release: dist
	$(AM_V_GEN)sha256sum $(DIST_ARCHIVES) > $(PACKAGE)-$(VERSION).sha256sum

EXTRA_DIST = doc README.md
