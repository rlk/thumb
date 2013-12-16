-include Makedefs

TARG= thumb

#------------------------------------------------------------------------------

# all : dist

$(TARG) : FORCE
	$(MAKE) -C src
	$(MAKE) -C proj/edworld
#	$(MAKE) -C proj/panoview

FORCE :

clean :
	$(MAKE) -C src clean
	$(MAKE) -C proj/edworld clean
#	$(MAKE) -C proj/panoview clean

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
	mkdir -p $(DATDIR)/program
	mkdir -p $(DATDIR)/glsl
	mkdir -p $(DATDIR)/glsl/dpy
	mkdir -p $(DATDIR)/scm
	mkdir -p $(DATDIR)/csv

	svnversion > $(DSTDIR)/VERSION

	# Copy and strip the executable.

	$(CP) proj/panoview/orbiter $(DSTDIR)
	$(STRIP)  $(DSTDIR)/orbiter

	# Copy global data.

	$(CP) data/conf.xml                             $(DATDIR)
	$(CP) data/data.xml                             $(DATDIR)
	$(CP) data/lang.xml                             $(DATDIR)
	$(CP) data/demo.xml                             $(DATDIR)
	$(CP) data/FreeMono.ttf                         $(DATDIR)
	$(CP) data/FreeSans.ttf                         $(DATDIR)
	$(CP) data/host/common.xml                      $(DATDIR)/host
	$(CP) data/program/fulldome.xml                 $(DATDIR)/program
	$(CP) data/glsl/dpy/fulldome.vert               $(DATDIR)/glsl/dpy
	$(CP) data/glsl/dpy/fulldome.frag               $(DATDIR)/glsl/dpy

	# Copy panoview / orbiter data

	$(CP) proj/panoview/data/csv/IAUMOON.csv  $(DATDIR)/csv
	$(CP) proj/panoview/data/csv/Tycho.csv    $(DATDIR)/csv
	$(CP) proj/panoview/data/csv/Copernic.csv $(DATDIR)/csv

	$(CP) proj/panoview/data/glsl/scm-basic.frag                  $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-basic.vert                  $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-displace.vert               $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-overlay.frag                $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-zoom.vert                   $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-lomsee.frag                 $(DATDIR)/glsl
	$(CP) proj/panoview/data/glsl/scm-relief-colormap-scalar.frag $(DATDIR)/glsl


	$(CP) proj/panoview/data/scm/GLD-WAC.xml              $(DATDIR)/scm
	$(CP) proj/panoview/data/scm/NAC_ROI.xml              $(DATDIR)/scm
	$(CP) proj/panoview/data/scm/NAC_ROI_Apollo16HiA.xml  $(DATDIR)/scm
	$(CP) proj/panoview/data/scm/NAC_ROI_TychoCtrLoA.xml  $(DATDIR)/scm
	$(CP) proj/panoview/data/scm/NAC_ROI_PlatoEjcLoA.xml  $(DATDIR)/scm
	$(CP) proj/panoview/data/scm/NAC_ROI_LUNOKOD1HIA.xml  $(DATDIR)/scm

	# Copy the documentation

	$(CP) proj/panoview/doc/README.html    $(DOCDIR)
	$(CP) proj/panoview/doc/img/select.png $(DOCDIR)/img
	$(CP) proj/panoview/doc/img/view.png   $(DOCDIR)/img
	$(CP) proj/panoview/doc/img/path.png   $(DOCDIR)/img
	$(CP) proj/panoview/doc/img/label.png  $(DOCDIR)/img

#------------------------------------------------------------------------------
