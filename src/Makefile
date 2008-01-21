
CFLAGS= -g -Wall -ansi -pedantic -fno-rtti
#CFLAGS= -pg -Wall -ansi -pedantic -DNDEBUG
#CFLAGS= -pg -O3 -Wall -ansi -pedantic -DNDEBUG -fno-rtti
#CFLAGS= -O3 -ffast-math -fno-rtti -Wall -ansi -pedantic -DNDEBUG

#------------------------------------------------------------------------------

CC= g++
RM= rm
CD= cd
CP= cp

TARG= ../thumb
OBJS=	serial.o \
	tracker.o \
	opengl.o \
	aabb.o \
	obj.o \
	matrix.o \
	lut.o \
	patch.o \
	buffer.o \
        geogen.o \
        georen.o \
        geomap.o \
	sphere.o \
	universe.o \
	atom.o \
	world.o \
	solid.o \
	joint.o \
	operation.o \
	constraint.o \
	program.o \
	texture.o \
	surface.o \
	binding.o \
	image.o \
	frame.o \
	mesh.o \
	pool.o \
	data.o \
	lang.o \
	conf.o \
	edit.o \
	play.o \
	info.o \
	mode.o \
	demo.o \
	user.o \
	glob.o \
	prog.o \
	util.o \
	font.o \
	control.o \
	gui.o \
	param.o \
	dir.o \
	perf.o \
	odeutil.o \
	frustum.o \
	normal.o \
	varrier.o \
	message.o \
	view.o \
	tile.o \
	host.o \
	main.o

LIBS= -lmxml -lode -ljpeg -lpng -lz -lm
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

ifeq ($(shell uname), Darwin)
	INCDIR += -I/opt/local/include
	LIBS   += -framework OpenGL

	SDL_CONF = /opt/local/bin/sdl-config
	FT2_CONF = /opt/local/bin/freetype-config
else
	LIBS += -lGL

	SDL_CONF = sdl-config
	FT2_CONF = freetype-config
endif

#------------------------------------------------------------------------------

INCDIR += -I$(HOME)/include
LIBDIR += -L$(HOME)/lib

OPTS +=   $(shell $(SDL_CONF) --cflags) \
	  $(shell $(FT2_CONF) --cflags)
LIBS +=   $(shell $(SDL_CONF) --libs) \
	  $(shell $(FT2_CONF) --libs)

#------------------------------------------------------------------------------

ifdef NVPM
	LIBS += -lNVPerfSDK
	OPTS += -DNVPM
endif

#------------------------------------------------------------------------------

%.d : %.cpp
	$(CC) $(CFLAGS) $(OPTS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(OPTS) $(INCDIR) -c $<

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) $(OPTS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) -f $(TARG) $(OBJS) $(DEPS)

#------------------------------------------------------------------------------

-include $(DEPS)
