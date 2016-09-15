SUBDIRS = libtess2

lib_LTLIBRARIES = libgnuVG.la

libgnuVG_la_CPPFLAGS = -std=c++11 -I $(srcdir)/gnuVG -I $(srcdir)/libtess2/include -I ${prefix}/include/freetype2 -Wall -Werror -Wfatal-errors

libgnuVG_la_SOURCES = \
gnuVG_profiler.cc \
gnuVG_context.cc gnuVG_context.hh \
gnuVG_debug.hh \
gnuVG_shader.cc gnuVG_shader.hh \
gnuVG_math.cc gnuVG_math.hh \
gnuVG_object.cc gnuVG_object.hh \
gnuVG_image.cc gnuVG_image.hh \
gnuVG_path.cc gnuVG_path.hh \
gnuVG_mask.cc gnuVG_mask.hh \
gnuVG_font.cc gnuVG_font.hh \
gnuVG_vgu.cc gnuVG_vgu.hh \
gnuVG_paint.cc gnuVG_paint.hh \
gnuVG_memclasses.cc gnuVG_memclasses.hh \
gnuVG_fontloader.cc gnuVG_fontloader.hh \
gnuVG_simplified_path.cc gnuVG_simplifified_path.hh

libgnuVG_la_LDFLAGS =  -L${prefix}/lib ./libtess2/src/libtess2.la -lGLESv2 -lfreetype

nobase_include_HEADERS = gnuVG/VG/openvg.h gnuVG/VG/gvgextensions.h gnuVG/VG/vgplatform.h gnuVG/VG/vgu.h gnuVG/VG/gnuVG_profiler.hh
