
#CFLAGS= -g -Wall
CFLAGS= -g -Wall -ansi -pedantic -fno-rtti
#CFLAGS= -pg -Wall -ansi -pedantic -DNDEBUG
#CFLAGS= -pg -O3 -Wall -ansi -pedantic -DNDEBUG -fno-rtti
#CFLAGS= -O3 -ffast-math -fno-rtti -Wall -ansi -pedantic -DNDEBUG

#------------------------------------------------------------------------------

CC= g++

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
	dev-input.o \
	dev-mouse.o \
	dev-gamepad.o \
	dev-tracker.o \
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

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

ifeq "$(MAKE)" "mingw32-make"

	RM = del

	INCDIR  = -Ic:/rlk/include -Ic:/rlk/include/SDL
	LIBDIR  = -Lc:/rlk/lib

	LIBS=   -lfreetype -lmxml -lode -ljpeg -lpng -lz -lm \
		-lmingw32 -lSDLmain -lSDL -mwindows -lopengl32 -lws2_32

else #-------------------------------------------------------------------------

	RM = rm -f

	SDLCONF = sdl-config
	FT2CONF = freetype-config

	INCDIR  = -I$(HOME)/include
	LIBDIR  = -L$(HOME)/lib

	CFLAGS += $(shell $(SDLCONF) --cflags) \
		  $(shell $(FT2CONF) --cflags)
	LIBS    = $(shell $(SDLCONF) --libs) \
		  $(shell $(FT2CONF) --libs) \
		-lmxml -lode -ljpeg -lpng -lz -lm -lGL
endif

#------------------------------------------------------------------------------

ifdef NVPM
	LIBS   += -lNVPerfSDK
	CFLAGS += -DNVPM
endif

#------------------------------------------------------------------------------

%.d : %.cpp
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $<
#	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(INCDIR) -c $<

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) "$(TARG)"

#------------------------------------------------------------------------------

-include $(DEPS)
