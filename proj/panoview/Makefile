include ../../Makedefs

#------------------------------------------------------------------------------

VIEWOBJS= \
	util3d/glsl.o \
	util3d/math3d.o \
	util3d/type.o \
	scm-index.o \
	scm-image.o \
	scm-frame.o \
	scm-set.o \
	scm-task.o \
	scm-file.o \
	scm-cache.o \
	scm-label.o \
	view-step.o \
	view-path.o \
	view-load.o \
	view-app.o \
	scm-model.o

ORBOBJS= $(VIEWOBJS) orbiter.o
PANOBJS= $(VIEWOBJS) panoview.o

PANDEPS= $(PANOBJS:.o=.d)
ORBDEPS= $(ORBOBJS:.o=.d)

#------------------------------------------------------------------------------

THUMB = ../../src/libthumb.a

INCDIR += -I../../include

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags) \
	  -DGL_TEXTURE_TARGET=GL_TEXTURE_3D

LIBS = $(THUMB) $(LIBFT2) $(LIBMXML) $(LIBODE) $(LIBTIF) $(LIBJPG) $(LIBPNG) $(LIBBZ2) $(LIBZ) $(LIBSDL) $(LIBGLEW) $(LIBEXT) -lm

#------------------------------------------------------------------------------

all : panoview orbiter

panoview: $(PANOBJS) $(THUMB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
#	$(STRIP) $@

orbiter: $(ORBOBJS) $(THUMB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
#	$(STRIP) $@

clean:
	$(RM) $(PANOBJS) $(PANDEPS) panoview
	$(RM) $(ORBOBJS) $(ORBDEPS) orbiter
	$(RM) scm-label-circle-vert.h
	$(RM) scm-label-circle-frag.h
	$(RM) scm-label-sprite-vert.h
	$(RM) scm-label-sprite-frag.h

#------------------------------------------------------------------------------

%-vert.h : %.vert
	xxd -i $^ > $@
%-frag.h : %.frag
	xxd -i $^ > $@

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(PANDEPS) $(ORBDEPS)
endif

