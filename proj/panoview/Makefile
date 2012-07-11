-include ../../Makedefs

#------------------------------------------------------------------------------

TARG= panoview
PANOBJS= \
	math3d.o \
	glsl.o \
	type.o \
	scm-set.o \
	scm-task.o \
	scm-file.o \
	scm-step.o \
	scm-path.o \
	scm-index.o \
	scm-model.o \
	scm-cache.o \
	scm-label.o \
	scm-viewer.o \
	scm-loader.o \
	panoview.o

ORBOBJS= \
	math3d.o \
	glsl.o \
	type.o \
	scm-set.o \
	scm-task.o \
	scm-file.o \
	scm-step.o \
	scm-path.o \
	scm-index.o \
	scm-model.o \
	scm-cache.o \
	scm-label.o \
	scm-viewer.o \
	scm-loader.o \
	orbiter.o

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
#	$(STRIP) $@

orbiter: $(ORBOBJS) $(THUMB)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
#	$(STRIP) $@

clean:
	$(RM) $(PANOBJS) $(PANDEPS) panoview
	$(RM) $(ORBOBJS) $(ORBDEPS) orbiter

#------------------------------------------------------------------------------

%-vert.h : %.vert
	xxd -i $^ > $@
%-frag.h : %.frag
	xxd -i $^ > $@

scm-label.o : \
	scm-label-circle-vert.h \
	scm-label-circle-frag.h \
	scm-label-sprite-vert.h \
	scm-label-sprite-frag.h

#------------------------------------------------------------------------------

-include $(PANDEPS) $(ORBDEPS)
