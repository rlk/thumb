
export CFLAGS= -g -Wall -ansi -pedantic

#------------------------------------------------------------------------------

CC= g++
RM= rm
CD= cd
CP= cp

TARG= ../thumb
OBJS=   opengl.o \
	matrix.o \
	light.o \
	camera.o \
	earth.o \
	sky.o \
	entity.o \
	joint.o \
	solid.o \
	scene.o \
	constraint.o \
	program.o \
	texture.o \
	geodata.o \
	data.o \
	lang.o \
	conf.o \
	edit.o \
	play.o \
	info.o \
	mode.o \
	demo.o \
	view.o \
	glob.o \
	prog.o \
	util.o \
	font.o \
	control.o \
	gui.o \
	param.o \
	directory.o \
	dir.o \
	main.o \
	obj/obj.o

LIBS= -lmxml -lode -ljpeg -lpng -lz -lm
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

ifeq ($(shell uname), Darwin)
	INCDIR += -I/opt/local/include

	SDL_CONF = /opt/local/bin/sdl-config
	FT2_CONF = /opt/local/bin/freetype-config
else
	LIBS += -lGL -lGLU

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

%.d : %.cpp
	$(CC) $(CFLAGS) $(OPTS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(OPTS) $(INCDIR) -c $<

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) $(OPTS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(CD) obj && $(MAKE) clean
	$(RM) -f $(TARG) $(OBJS) $(DEPS)

#------------------------------------------------------------------------------

obj/obj.o :
	cd obj && $(MAKE)

#------------------------------------------------------------------------------

-include $(DEPS)
