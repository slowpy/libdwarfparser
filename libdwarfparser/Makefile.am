# @configure_input@
AM_CXXFLAGS = -Wall -Wextra -Werror -fpic -std=c++11 -I $(top_builddir)/src


# Package-specific substitution variables
package = @PACKAGE_NAME@
version = @PACKAGE_VERSION@
tarname = @PACKAGE_TARNAME@
distdir = $(tarname)-$(version)

# Prefix-specific substitution variables
prefix      = @prefix@
exec_prefix = @exec_prefix@
bindir      = @bindir@
srcdir      = @srcdir@

lib_LTLIBRARIES = libdwarfparser.la

libdwarfparser_la_SOURCES =	basetype.cpp \
							consttype.cpp \
							dwarfexception.cpp \
							dwarfparser.cpp \
							enum.cpp \
							funcpointer.cpp \
							function.cpp \
							pointer.cpp \
							refbasetype.cpp \
							referencingtype.cpp \
							struct.cpp \
							structured.cpp \
							structuredmember.cpp \
							symbol.cpp \
							typedef.cpp \
							union.cpp \
							variable.cpp 

#libdwarfparser_la_HEADERS =	basetype.h \
#							consttype.h \
#							dwarfexception.h \
#							dwarfparser.h \
#							enum.h \
#							funcpointer.h \
#							function.h \
#							pointer.h \
#							refbasetype.h \
#							referencingtype.h \
#							struct.h \
#							structured.h \
#							structuredmember.h \
#							symbol.h \
#							typedef.h \
#							union.h \
#							variable.h \
#							libdwarfparser.h \
#							helpers.h

libdwarfparser_la_CPPFLAGS = -I /home/kittel/projekte/libvmi/libdwarf/libdwarf/ -ggdb
libdwarfparser_la_LDFLAGS  = -ldwarf -lelf  -L/home/kittel/projekte/libvmi/libdwarf/libdwarf/