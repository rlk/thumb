-include ../../Makedefs

#------------------------------------------------------------------------------

TARG= panoview
PANOBJS= \
	math3d.o \
	glsl.o \
	cube.o \
	type.o \
	sph-model.o \
	sph-cache.o \
	sph-label.o \
	sph-viewer.o \
	sph-loader.o \
	panoview.o

ORBOBJS= \
	math3d.o \
	glsl.o \
	cube.o \
	type.o \
	sph-model.o \
	sph-cache.o \
	sph-label.o \
	sph-viewer.o \
	sph-loader.o \
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
	-lthumb -ltiff -lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)

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
