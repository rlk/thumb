-include Makedefs

TARG= thumb

#------------------------------------------------------------------------------

$(TARG) : FORCE
	$(MAKE) -C src
	$(MAKE) -C proj/panoview

FORCE :

clean :
	$(MAKE) -C src clean
	$(MAKE) -C proj/panoview clean

doc :
	doxygen Doxyfile

.PHONY : doc

#------------------------------------------------------------------------------

DSTDIR=orbiter
DOCDIR=$(DSTDIR)/doc
DATDIR=$(DSTDIR)/data

dist : $(TARG)

	# Create the distribution directory structure.

	mkdir -p $(DSTDIR)
	mkdir -p $(DATDIR)
	mkdir -p $(DOCDIR)
	mkdir -p $(DOCDIR)/img
	mkdir -p $(DATDIR)/host
	mkdir -p $(DATDIR)/glsl
	mkdir -p $(DATDIR)/scm

	# Copy the executable.

	$(CP) proj/panoview/orbiter $(DSTDIR)

	# Copy global data.

	$(CP) data/conf.xml                             $(DATDIR)
	$(CP) data/data.xml                             $(DATDIR)
	$(CP) data/lang.xml                             $(DATDIR)
	$(CP) data/demo.xml                             $(DATDIR)
	$(CP) data/FreeMono.ttf                         $(DATDIR)
	$(CP) data/FreeSans.ttf                         $(DATDIR)
	$(CP) data/host/common.xml                      $(DATDIR)/host

	# Copy panoview / orbiter data

	$(CP) proj/panoview/data/IAUMOON.csv            $(DATDIR)
	$(CP) proj/panoview/data/glsl/scm-basic.frag    $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-basic.vert    $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-displace.vert $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-zoom.vert     $(DATDIR)/glsl
	$(CP) proj/panoview/data/scm/GLD-WAC.xml        $(DATDIR)/scm

	# Copy the documentation

	$(CP) proj/panoview/doc/README.html    $(DOCDIR)
	$(CP) proj/panoview/doc/img/select.png $(DOCDIR)/img
	$(CP) proj/panoview/doc/img/view.png   $(DOCDIR)/img
	$(CP) proj/panoview/doc/img/path.png   $(DOCDIR)/img

#------------------------------------------------------------------------------
