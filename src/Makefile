# libthumb -- Linux / OS X Makefile

include ../Makedefs

#------------------------------------------------------------------------------

CFLAGS += -DVERSION=0.9

#------------------------------------------------------------------------------

OBJS=	app-data.o \
	app-data-file.o \
	app-data-pack.o \
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
	etc-log.o \
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
	ogl-convex.o \
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
	wrl-world.o \
	zip-data.o

DEPS= $(OBJS:.o=.d)

CFLAGS += -I../include

#------------------------------------------------------------------------------
# The platform and linkage determine the target name.

TARGDIR = ../$(CONFIG)

ifdef DYNAMIC
	ifdef OSX
		TARG = $(TARGDIR)/libthumb.dylib
	endif
	ifdef LINUX
		TARG = $(TARGDIR)/libthumb.so
	endif
else
	TARG = $(TARGDIR)/libthumb.a
endif

#------------------------------------------------------------------------------

all : $(TARGDIR) $(TARG)

$(TARGDIR) :
	mkdir -p $(TARGDIR)

$(TARGDIR)/libthumb.a : $(OBJS)
	ar -r $@ $(OBJS)

$(TARGDIR)/libthumb.so : $(OBJS)
	$(CXX) $(CFLAGS) -shared -o $@ $(OBJS) $(LIBS)

$(TARGDIR)/libthumb.dylib : $(OBJS)
	$(CXX) $(CFLAGS) -dynamiclib -o $@ $(OBJS) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) $(TARG)

#------------------------------------------------------------------------------
# The bin2c tool embeds binary data in C sources.

B2C = ../etc/bin2c

$(B2C) : ../etc/bin2c.c
	$(CC) -o $(B2C) ../etc/bin2c.c

#------------------------------------------------------------------------------

zip-data.cpp : ../data/data.zip $(B2C)
	$(B2C) thumb_data < $< > $@

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
