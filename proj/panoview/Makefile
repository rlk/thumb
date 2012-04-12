-include ../../Makedefs

#------------------------------------------------------------------------------

TARG= panoview
PANOBJS= \
	math3d.o \
	glsl.o \
	cube.o \
	type.o \
	scm-file.o \
	scm-model.o \
	scm-cache.o \
	scm-label.o \
	scm-viewer.o \
	scm-loader.o \
	panoview.o

ORBOBJS= \
	math3d.o \
	glsl.o \
	cube.o \
	type.o \
	scm-file.o \
	scm-model.o \
	scm-cache.o \
	scm-label.o \
	scm-viewer.o \
	scm-loader.o \
	orbiter.o

PANDEPS= $(PANOBJS:.o=.d)
ORBDEPS= $(ORBOBJS:.o=.d)

#------------------------------------------------------------------------------

INCDIR += -I../../include
LIBDIR += -L../../src

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)
LIBS    = $(shell $(SDLCONF) --libs) \
	  $(shell $(FT2CONF) --libs) \
	  /usr/local/lib/libtiff.a \
	-lthumb -lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)

#------------------------------------------------------------------------------

all : panoview orbiter

panoview : $(PANOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)

orbiter : $(ORBOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBDIR) $(LIBS)

clean :
	$(RM) $(PANOBJS) $(PANDEPS) panoview
	$(RM) $(ORBOBJS) $(ORBDEPS) orbiter

#------------------------------------------------------------------------------

-include $(PANDEPS) $(ORBDEPS)
