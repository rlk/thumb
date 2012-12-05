include ../../Makedefs

#------------------------------------------------------------------------------

VIEWOBJS= \
	scm/util3d/glsl.o \
	scm/util3d/math3d.o \
	scm/util3d/type.o \
	scm/scm-system.o \
	scm/scm-index.o \
	scm/scm-image.o \
	scm/scm-scene.o \
	scm/scm-set.o \
	scm/scm-task.o \
	scm/scm-file.o \
	scm/scm-cache.o \
	scm/scm-model.o \
	scm/scm-label.o \
	scm/scm-log.o \
	view-step.o \
	view-path.o \
	view-load.o \
	view-app.o

ORBOBJS= $(VIEWOBJS) orbiter.o
PANOBJS= $(VIEWOBJS) panoview.o

PANDEPS= $(PANOBJS:.o=.d)
ORBDEPS= $(ORBOBJS:.o=.d)

#------------------------------------------------------------------------------

THUMB = ../../src/libthumb.a

INCDIR += -I../../include

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)

LIBS = $(THUMB) $(LIBFT2) $(LIBMXML) $(LIBODE) $(LIBTIF) $(LIBJPG) $(LIBPNG) $(LIBBZ2) $(LIBZ) $(LIBSDL) $(LIBGLEW) $(LIBEXT) -lm

#------------------------------------------------------------------------------

all : panoview orbiter

panoview: $(PANOBJS) $(THUMB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	# $(STRIP) $@

orbiter: $(ORBOBJS) $(THUMB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	# $(STRIP) $@

clean:
	$(RM) $(PANOBJS) $(PANDEPS) panoview
	$(RM) $(ORBOBJS) $(ORBDEPS) orbiter

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(PANDEPS) $(ORBDEPS)
endif

