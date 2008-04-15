
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
OBJS=   util.o \
	dir.o \
	ode.o \
	obj.o \
	matrix.o \
	tracker.o \
	ogl-lut.o \
	ogl-aabb.o \
	ogl-binding.o \
	ogl-progset.o \
	ogl-program.o \
	ogl-texture.o \
	ogl-surface.o \
	ogl-buffer.o \
	ogl-image.o \
	ogl-frame.o \
	ogl-mesh.o \
	ogl-pool.o \
	ogl-opengl.o \
	gui-control.o \
	gui-gui.o \
	mode-edit.o \
	mode-play.o \
	mode-info.o \
	mode-mode.o \
	wrl-constraint.o \
	wrl-operation.o \
	wrl-param.o \
	wrl-atom.o \
	wrl-solid.o \
	wrl-joint.o \
	wrl-world.o \
        uni-geomap.o \
        uni-geocsh.o \
        uni-geogen.o \
        uni-georen.o \
	uni-spatch.o \
	uni-sphere.o \
	uni-universe.o \
	app-frustum.o \
	app-dome.o \
	app-normal.o \
	app-varrier.o \
	app-message.o \
	app-serial.o \
	app-font.o \
	app-view.o \
	app-tile.o \
	app-host.o \
	app-user.o \
	app-perf.o \
	app-lang.o \
	app-glob.o \
	app-conf.o \
	app-data.o \
	app-prog.o \
	demo.o \
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
