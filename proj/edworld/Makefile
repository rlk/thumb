-include ../../Makedefs

#------------------------------------------------------------------------------

TARG= edworld
OBJS=   demo.o \
	main.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

INCDIR += -I../../include
LIBDIR += -L../../src

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)
LIBS    = $(shell $(SDLCONF) --libs) \
	  $(shell $(FT2CONF) --libs) \
	-lthumb -lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) $(TARG)

#------------------------------------------------------------------------------

-include $(DEPS)
