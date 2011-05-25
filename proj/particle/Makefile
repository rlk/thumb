-include ../../Makedefs

#------------------------------------------------------------------------------

TARG= particle
OBJS=   danpart.o \
	main.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

INCDIR += -I../../include -I/usr/local/cuda/include
LIBDIR += -L../../src     -L/usr/local/cuda/lib

CFLAGS += -DCUDA_FORCE_API_VERSION=3010 \
	  $(shell $(SDLCONF) --cflags) \
	  $(shell $(FT2CONF) --cflags)
LIBS    = $(shell $(SDLCONF) --libs) \
	  $(shell $(FT2CONF) --libs) \
	-lcuda -lthumb -lmxml -lode -ljpeg -lpng -lz -lm $(OGLLIB)

#------------------------------------------------------------------------------

$(TARG) : $(OBJS) danpart.ptx
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

danpart.ptx : danpart.cu
	/usr/local/cuda/bin/nvcc --ptx danpart.cu

clean :
	$(RM) $(OBJS) $(DEPS) $(TARG)

#------------------------------------------------------------------------------

-include $(DEPS)
