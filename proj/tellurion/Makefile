-include ../../Makedefs

#------------------------------------------------------------------------------

TARG= tellurion
OBJS=	uni-galaxy.o \
	uni-geocsh.o \
	uni-geogen.o \
	uni-geomap.o \
	uni-georen.o \
	uni-overlay.o \
	uni-slides.o \
	uni-spatch.o \
	uni-sphere.o \
	uni-universe.o \
	tellurion.o \
	main.o
#	uni-progset.o
#	uni-atmo.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

INCDIR += -I../../include
LIBDIR += -L../../src

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)
LIBS    = $(shell $(SDLCONF) --libs) \
	  $(shell $(FT2CONF) --libs) \
	-lthumb -lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) $(TARG)

#------------------------------------------------------------------------------

-include $(DEPS)
