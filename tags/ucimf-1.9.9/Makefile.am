SUBDIRS = ./display 

AM_CPPFLAGS = -g -DDEBUG 

ucimfinclude_HEADERS = ucimf.h imf.h
ucimfincludedir = $(includedir)

imfinclude_HEADERS = imf.h widget.h type.h graphport.h window.h
imfincludedir = $(includedir)/imf/

lib_LTLIBRARIES = libucimf.la
libucimf_la_SOURCES = options.cpp\
                        graphdev.cpp font.cpp\
			type.cpp graphport.cpp\
			window.cpp widget.cpp shape.cpp cwm.cpp\
			ucimf.cpp

libucimf_la_LDFLAGS = -lltdl -ldisplay
libucimf_la_LIBADD = -L$(libdir) -L./display/

bin_PROGRAMS = ucimf_keyboard
ucimf_keyboard_SOURCES = ucimf_keyboard.cpp

confdir=$(sysconfdir)
conf_DATA = ucimf.conf ucimf.conf.example

EXTRA_DIST=  cwm.h font.h graphdev.h shape.h options.h ucimf.conf ucimf.conf.example
