-include ../Makedefs

#------------------------------------------------------------------------------

CC= g++
TARG= thumb
OBJS=   util.o \
	dir.o \
	ode.o \
	matrix.o \
	ogl-obj.o \
	ogl-lut.o \
	ogl-aabb.o \
	ogl-range.o \
	ogl-shadow.o \
	ogl-reflection-env.o \
	ogl-irradiance-env.o \
	ogl-sh-basis.o \
	ogl-d-omega.o \
	ogl-cubelut.o \
	ogl-process.o \
	ogl-uniform.o \
	ogl-program.o \
	ogl-texture.o \
	ogl-binding.o \
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
	uni-galaxy.o \
	uni-slides.o \
	uni-overlay.o \
	uni-universe.o \
	dev-tracker.o \
	dev-mouse.o \
	dpy-channel.o \
	dpy-display.o \
	dpy-normal.o \
	dpy-anaglyph.o \
	dpy-lenticular.o \
	app-event.o \
	app-frustum.o \
	app-serial.o \
	app-font.o \
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

#	dev-wiimote.o \
#	dpy-dome.o \
#	dpy-varrier.o \

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------
# Windows MinGW build

ifeq "$(MAKE)" "mingw32-make"

	INCDIR += -Ie:/rlk/include -Ie:/rlk/include/SDL
	LIBDIR += -Le:/rlk/lib

	LIBS=   -lfreetype -lmxml -lode -ljpeg -lpng -lz -lm \
		-lmingw32 -lSDLmain -lSDL -mwindows -lopengl32 -lws2_32

else #-------------------------------------------------------------------------
# Linux/OSX build

	INCDIR += -I$(HOME)/include
	LIBDIR += -L$(HOME)/lib

	CFLAGS += $(shell $(SDLCONF) --cflags) \
		  $(shell $(FT2CONF) --cflags)
	LIBS    = $(shell $(SDLCONF) --libs) \
		  $(shell $(FT2CONF) --libs) \
		-lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)
endif

#------------------------------------------------------------------------------

ifdef NVPM
	LIBS   += -lNVPerfSDK
	CFLAGS += -DNVPM
endif

ifdef WII
	LIBS   += -lcwiimote -lbluetooth
	CFLAGS += -DENABLE_WIIMOTE
endif

#------------------------------------------------------------------------------

%.d : %.cpp
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $<
#	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(INCDIR) -c $<

#------------------------------------------------------------------------------

../$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o ../$(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) "../$(TARG)"

test : ../$(TARG)
	(cd ..; ./$(TARG))
#	(cd ..; ./$(TARG) 960x540)

#------------------------------------------------------------------------------

-include $(DEPS)
