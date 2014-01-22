include ../Makedefs

#------------------------------------------------------------------------------

CC      = cc
CXX     = c++
#CFLAGS= -g -Wall
#CFLAGS= -O2 -ffast-math -Wall -DNDEBUG

CFLAGS += -DVERSION=0.9

#------------------------------------------------------------------------------

OBJS=	app-data.o \
	app-event.o \
	app-file.o \
	app-font.o \
	app-frustum.o \
	app-glob.o \
	app-host.o \
	app-perf.o \
	app-lang.o \
	app-prog.o \
	app-view.o \
	dev-gamepad.o \
	dev-mouse.o \
	dev-sixense.o \
	dev-skeleton.o \
	dpy-anaglyph.o \
	dpy-channel.o \
	dpy-direct.o \
	dpy-display.o \
	dpy-fulldome.o \
	dpy-interlace.o \
	dpy-lenticular.o \
	dpy-normal.o \
	dpy-oculus.o \
	etc-dir.o \
	etc-ode.o \
	gui-control.o \
	gui-gui.o \
	mode-edit.o \
	mode-info.o \
	mode-mode.o \
	mode-play.o \
	ogl-aabb.o \
	ogl-binding.o \
	ogl-buffer.o \
	ogl-cookie.o \
	ogl-cubelut.o \
	ogl-d-omega.o \
	ogl-frame.o \
	ogl-image.o \
	ogl-irradiance-env.o \
	ogl-lut.o \
	ogl-mesh.o \
	ogl-mirror.o \
	ogl-obj.o \
	ogl-opengl.o \
	ogl-pool.o \
	ogl-process.o \
	ogl-program.o \
	ogl-range.o \
	ogl-reflection-env.o \
	ogl-sh-basis.o \
	ogl-shadow.o \
	ogl-sprite.o \
	ogl-surface.o \
	ogl-texture.o \
	ogl-uniform.o \
	wrl-atom.o \
	wrl-constraint.o \
	wrl-joint.o \
	wrl-light.o \
	wrl-operation.o \
	wrl-param.o \
	wrl-solid.o \
	wrl-world.o
#	dev-hybrid.o \
#	dev-trackd.o \

DEPS= $(OBJS:.o=.d)

CFLAGS += -I../include

#------------------------------------------------------------------------------

ALL = libthumb.a

ifeq (,$(STATIC))
	ifeq ($(shell uname), Linux)
		ALL += libthumb.so
	endif
	ifeq ($(shell uname), Darwin)
		ALL += libthumb.dylib
	endif
endif

#------------------------------------------------------------------------------

all : $(ALL)

libthumb.a : $(OBJS)
	ar -r libthumb.a $(OBJS)

libthumb.so : $(OBJS)
	$(CXX) $(CFLAGS) -shared -o libthumb.so $(OBJS) $(LIBS)

libthumb.dylib : $(OBJS)
	$(CXX) $(CFLAGS) -dynamiclib -o libthumb.dylib $(OBJS) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) $(ALL)

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
