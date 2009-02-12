-include ../../Makedefs

CFLAGS= -Wall -O3

TARG= prepobj
OBJS= prepobj.o obj.o
LIBS= -ljpeg -lpng -lz -lm $(OGLLIB)

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

	$(CH) /opt/local/lib/libjpeg.62.dylib      \
          @executable_path/../../lib/libjpeg.62.dylib    $(TARG)
	$(CH) /opt/local/lib/libpng12.0.dylib      \
          @executable_path/../../lib/libpng12.0.dylib    $(TARG)
	$(CH) /opt/local/lib/libz.1.dylib          \
          @executable_path/../../lib/libz.1.dylib        $(TARG)

#------------------------------------------------------------------------------
