include ../../Makedefs

#------------------------------------------------------------------------------

VIEWOBJS= \
	scm/util3d/glsl.o \
	scm/util3d/math3d.o \
	scm/util3d/type.o \
	scm/scm-cache.o \
	scm/scm-file.o \
	scm/scm-image.o \
	scm/scm-index.o \
	scm/scm-label.o \
	scm/scm-log.o \
	scm/scm-path.o \
	scm/scm-render.o \
	scm/scm-sample.o \
	scm/scm-scene.o \
	scm/scm-set.o \
	scm/scm-sphere.o \
	scm/scm-step.o \
	scm/scm-system.o \
	scm/scm-task.o \
	view-step.o \
	view-path.o \
	view-load.o \
	view-app.o

ORBOBJS= $(VIEWOBJS) orbiter.o
PANOBJS= $(VIEWOBJS) panoview.o

PANDEPS= $(PANOBJS:.o=.d)
ORBDEPS= $(ORBOBJS:.o=.d)

GLSL= \
	scm/data/scm-label-circle-frag.h \
	scm/data/scm-label-circle-vert.h \
	scm/data/scm-label-sprite-frag.h \
	scm/data/scm-label-sprite-vert.h \
	scm/data/scm-render-blur-frag.h \
	scm/data/scm-render-blur-vert.h \
	scm/data/scm-render-both-frag.h \
	scm/data/scm-render-both-vert.h \
	scm/data/scm-render-fade-frag.h \
	scm/data/scm-render-fade-vert.h

#------------------------------------------------------------------------------

THUMB = ../../src/libthumb.a

INCDIR += -I../../include -Iscm/data

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)

LIBS = $(THUMB) $(LIBFT2) $(LIBMXML) $(LIBODE) $(LIBTIF) $(LIBJPG) $(LIBPNG) $(LIBBZ2) $(LIBZ) $(LIBSDL) $(LIBGLEW) $(LIBEXT) -lm

#------------------------------------------------------------------------------

all : panoview orbiter

panoview: $(GLSL) $(PANOBJS) $(THUMB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	# $(STRIP) $@

orbiter: $(GLSL) $(ORBOBJS) $(THUMB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	# $(STRIP) $@

clean:
	$(RM) $(PANOBJS) $(PANDEPS) panoview
	$(RM) $(ORBOBJS) $(ORBDEPS) orbiter
	$(RM) $(GLSL)

#------------------------------------------------------------------------------

scm/data/%-vert.h : scm/data/%.vert
	$(MAKE) -C scm/data $(notdir $@)

scm/data/%-frag.h : scm/data/%.frag
	$(MAKE) -C scm/data $(notdir $@)

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(PANDEPS) $(ORBDEPS)
endif

