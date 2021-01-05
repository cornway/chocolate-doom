
AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/rt/sr/src/renderer

SUBDIRS = sr/src/renderer .

librt_LDADD = sr/src/renderer/srlib.a

noinst_LIBRARIES=librt.a

librt_a_SOURCES =               \
rt.cpp           rt.h           \
vec3.cpp         vec3.h         \
poly.cpp         poly.h         \
