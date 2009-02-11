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

#------------------------------------------------------------------------------
