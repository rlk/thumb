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
	ogl-terrain.o \
	ogl-sprite.o \
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
	dev-mouse.o \
	dev-tracker.o \
	dev-gamepad.o \
	dpy-channel.o \
	dpy-display.o \
	dpy-normal.o \
	dpy-anaglyph.o \
	dpy-lenticular.o \
	app-event.o \
	app-frustum.o \
	app-file.o \
	app-font.o \
	app-host.o \
	app-user.o \
	app-perf.o \
	app-lang.o \
	app-glob.o \
	app-data.o \
	app-prog.o \
	demo.o \
	main.o \
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
	dan/danpart.o


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

	INCDIR += -I$(HOME)/include -I/usr/local/cuda/include
	LIBDIR += -L$(HOME)/lib     -L/usr/local/cuda/lib

	CFLAGS += $(shell $(SDLCONF) --cflags) \
		  $(shell $(FT2CONF) --cflags)
	LIBS    = $(shell $(SDLCONF) --libs) \
		  $(shell $(FT2CONF) --libs) \
		-lcuda -lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)
#	-lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)
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

#%.o : %.cpp Makefile
%.o : %.cpp
	$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@

#------------------------------------------------------------------------------

../$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o ../$(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) "../$(TARG)"

test : ../$(TARG)
	(cd ..; ./$(TARG) 960x540)

#------------------------------------------------------------------------------
# "make images" generates contour images of all source files.

SRCS= $(shell find . -name "*.cpp" -or -name "*.hpp")
IMGS= $(SRCS:%=%.png)

ESFLAGS= -Ecpp --no-header --color -MScroll
CVFLAGS= -flatten -resize 25% -trim -normalize -quality 100 -colors 16

%.png : %
	enscript $(ESFLAGS) -p - $< | convert ps:- $(CVFLAGS) $@

images : $(IMGS)

#------------------------------------------------------------------------------

-include $(DEPS)
