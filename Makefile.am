AM_CPPFLAGS=@THREADFLAGS@ $(BOOST_CPPFLAGS)
#if !ALLSTATIC
#install-exec-local:
#	install .lib/libsocketbackend.so.0.0.0 @libdir@
#endif

EXTRA_DIST=OBJECTFILES OBJECTLIBS
lib_LTLIBRARIES = libsocketbackend.la

libsocketbackend_la_SOURCES=connector.hh connector.cc socketbackend.cc socketbackend.hh json.c json.h ../pipebackend/coprocess.cc ../pipebackend/coprocess.hh
libsocketbackend_la_LDFLAGS=-module -avoid-version
libsocketbackend_la_LIBS=-ljson -lboost_system
