-include ../../Makedefs

CC= gcc -std=c99
CFLAGS= -Wall -pedantic -g

TARG= prepgmm
OBJS= prepgmm.o
LIBS= -lpng -lz -lm

#------------------------------------------------------------------------------

.c.o :
	$(CC) $(CFLAGS) $(INCDIR) -c $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS)

install : $(TARG)
	$(CP) $(TARG) $(INSTALLDIR)

osxdist : $(TARG)

	$(CH) /opt/local/lib/libpng12.0.dylib      \
          @executable_path/../../lib/libpng12.0.dylib    $(TARG)
	$(CH) /opt/local/lib/libz.1.dylib          \
          @executable_path/../../lib/libz.1.dylib        $(TARG)

#------------------------------------------------------------------------------
