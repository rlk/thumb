include ../../Makedefs

#------------------------------------------------------------------------------

VIEWOBJS= view-load.o view-app.o

ORBOBJS= $(VIEWOBJS) orbiter.o
PANOBJS= $(VIEWOBJS) panoview.o

PANDEPS= $(PANOBJS:.o=.d)
ORBDEPS= $(ORBOBJS:.o=.d)

#------------------------------------------------------------------------------

THUMB = ../../src/libthumb.a
INCDIR += -I../../include -Iscm

SCM = scm/libscm.a

LIBS = $(THUMB) $(SCM) $(LIBFT2) $(LIBMXML) $(LIBODE) $(LIBTIF) $(LIBJPG) $(LIBPNG) $(LIBBZ2) $(LIBZ) $(LIBSDL) $(LIBGLEW) $(LIBEXT) -lm

#------------------------------------------------------------------------------

all : panoview orbiter

panoview: $(SCM) $(PANOBJS) $(THUMB)
	$(CXX) $(CFLAGS) $(CONF) -o $@ $(PANOBJS) $(LIBS)

orbiter: $(SCM) $(ORBOBJS) $(THUMB)
	$(CXX) $(CFLAGS) $(CONF) -o $@ $(ORBOBJS) $(LIBS)

clean:
	$(RM) $(PANOBJS) $(PANDEPS) panoview
	$(RM) $(ORBOBJS) $(ORBDEPS) orbiter

#------------------------------------------------------------------------------

$(SCM) : .FORCE
	$(MAKE) -C scm

.FORCE :

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(PANDEPS) $(ORBDEPS)
endif

