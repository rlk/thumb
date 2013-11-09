include ../Makedefs

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
	dev-hybrid.o \
	dev-mouse.o \
	dev-sixense.o \
	dev-skeleton.o \
	dev-trackd.o \
	dpy-anaglyph.o \
	dpy-channel.o \
	dpy-direct.o \
	dpy-display.o \
	dpy-fulldome.o \
	dpy-interlace.o \
	dpy-lenticular.o \
	dpy-normal.o \
	etc-dir.o \
	etc-math.o \
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
	wrl-operation.o \
	wrl-param.o \
	wrl-solid.o \
	wrl-world.o

DEPS= $(OBJS:.o=.d)

TARG= libthumb.a

#------------------------------------------------------------------------------

INCDIR += -I../include

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)

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
	ar -r $(TARG) $(OBJS)

clean :
	$(RM) $(OBJS) $(DEPS) $(TARG)

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
