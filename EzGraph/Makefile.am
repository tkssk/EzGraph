lib_LTLIBRARIES = libEzGraph.la

libEzGraph_la_SOURCES = EzGraph.c EzGraph.h

if ALSA
libEzGraph_la_SOURCES += audio.c code.c mmlp.c play.c token.c audio.h mmlp.h taiko1.h
endif

if NETEXT
libEzGraph_la_SOURCES += connect.c
endif

AM_CPPFLAGS = -I$(top_srcdir)/include $(X_CFLAGS) \
	$(defXdbe) $(defALSA) $(defNETEXT) $(defEzCharCode) \
	$(Freetype2_CFLAGS)
libEzGraph_la_LIBADD =  $(X_LIBS) -lX11 -lXpm -lXft \
	$(libXdbe) $(libALSA) $(Freetype2_LIBS)

include_HEADERS = EzGraph.h
