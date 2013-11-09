-include ../../Makedefs

#------------------------------------------------------------------------------

TARG= edworld
OBJS=   demo.o \
	main.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

THUMB = ../../src/libthumb.a

INCDIR += -I../../include
LIBDIR += -L../../src

CFLAGS += $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)

LIBS = $(THUMB) $(LIBFT2) $(LIBMXML) $(LIBODE) $(LIBTIF) $(LIBJPG) $(LIBPNG) $(LIBBZ2) $(LIBZ) $(LIBSDL) $(LIBGLEW) $(LIBEXT) -lm

#------------------------------------------------------------------------------

$(TARG) : $(OBJS) $(THUMB)
	$(CXX) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(OBJS) $(DEPS) $(TARG)

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
