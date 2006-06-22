
CC= g++
CFLAGS= -g -Wall -ansi -pedantic

#CC= /opt/intel/cc/9.0/bin/icc
#CFLAGS= -g -Wall -wd981 -wd383 -wd869

TARG= ../thumb
OBJS= opengl.o matrix.o obj.o light.o camera.o earth.o sky.o entity.o \
	joint.o solid.o scene.o constraint.o image.o data.o lang.o \
	conf.o edit.o play.o info.o mode.o demo.o prog.o util.o font.o \
	control.o gui.o param.o directory.o main.o

LIBS= -lmxml -lode -ljpeg -lpng -lz -lm
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

ifeq ($(shell uname), Darwin)
	INCDIR += -I/sw/include

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

CFLAGS += $(shell $(SDL_CONF) --cflags) \
	  $(shell $(FT2_CONF) --cflags)
LIBS   += $(shell $(SDL_CONF) --libs) \
	  $(shell $(FT2_CONF) --libs)

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.d : %.cpp
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(INCDIR) -c $<

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(INCDIR) -c $<

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm -f $(TARG) $(OBJS) $(DEPS)

#------------------------------------------------------------------------------

-include $(DEPS)
