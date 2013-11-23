include ../../Makedefs

#------------------------------------------------------------------------------

VIEWOBJS= view-load.o view-app.o

ORBOBJS= $(VIEWOBJS) orbiter.o
PANOBJS= $(VIEWOBJS) panoview.o
OPTOBJS= $(VIEWOBJS) panoptic.o

PANDEPS= $(PANOBJS:.o=.d)
ORBDEPS= $(ORBOBJS:.o=.d)
OPTDEPS= $(OPTOBJS:.o=.d)

#------------------------------------------------------------------------------

THUMB = ../../src/libthumb.a
INCDIR += -I../../include -Iscm

SCM = scm/libscm.a

LIBS = $(THUMB) $(SCM) $(LIBFT2) $(LIBMXML) $(LIBODE) $(LIBTIF) $(LIBJPG) $(LIBPNG) $(LIBBZ2) $(LIBZ) $(LIBSDL) $(LIBGLEW) $(LIBEXT) -lm

#------------------------------------------------------------------------------

all : panoview orbiter panoptic

panoview: $(SCM) $(PANOBJS) $(THUMB)
	$(CXX) $(CFLAGS) $(CONF) -o $@ $(PANOBJS) $(LIBS)

orbiter: $(SCM) $(ORBOBJS) $(THUMB)
	$(CXX) $(CFLAGS) $(CONF) -o $@ $(ORBOBJS) $(LIBS)

panoptic: $(SCM) $(OPTOBJS) $(THUMB)
	$(CXX) $(CFLAGS) $(CONF) -o $@ $(OPTOBJS) $(LIBS)

clean:
	$(RM) $(PANOBJS) $(PANDEPS) panoview
	$(RM) $(ORBOBJS) $(ORBDEPS) orbiter
	$(RM) $(OPTOBJS) $(OPTDEPS) panoptic

#------------------------------------------------------------------------------

$(SCM) : .FORCE
	$(MAKE) -C scm

.FORCE :

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(PANDEPS) $(ORBDEPS)
endif

