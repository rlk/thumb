-include ../Makedefs

#------------------------------------------------------------------------------

OBJS=   dir.o \
	ode.o \
	util.o \
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
	ogl-mirror.o \
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
	dev-hybrid.o \
	dev-gamepad.o \
	dev-tracker.o \
	dpy-channel.o \
	dpy-display.o \
	dpy-normal.o \
	dpy-anaglyph.o \
	dpy-interlace.o \
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

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

INCDIR += -I../include

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)
LIBS    = $(shell $(SDLCONF) --libs) \
	  $(shell $(FT2CONF) --libs) \
	-lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)

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

$(TARG) : $(OBJS)
	$(LD) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) $(TARG)

#------------------------------------------------------------------------------

-include $(DEPS)
