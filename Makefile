-include Makedefs

TARG= thumb
APP= Thumb.app

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

osxdist : $(TARG)

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

$(APP) :

	mkdir -p $(APP)
	mkdir -p $(APP)/Contents
	mkdir -p $(APP)/Contents/MacOS
	mkdir -p $(APP)/Contents/Resources
	mkdir -p $(APP)/Contents/Frameworks

	$(CH) /opt/local/lib/libSDL-1.2.0.dylib    \
		  @executable_path/../Frameworks/libSDL-1.2.0.dylib  $(TARG)
	$(CH) /opt/local/lib/libfreetype.6.dylib   \
		  @executable_path/../Frameworks/libfreetype.6.dylib $(TARG)
	$(CH) /opt/local/lib/libjpeg.62.dylib      \
		  @executable_path/../Frameworks/libjpeg.62.dylib    $(TARG)
	$(CH) /opt/local/lib/libpng12.0.dylib      \
		  @executable_path/../Frameworks/libpng12.0.dylib    $(TARG)
	$(CH) /opt/local/lib/libz.1.dylib          \
		  @executable_path/../Frameworks/libz.1.dylib        $(TARG)

	$(CP) $(TARG) $(APP)/Contents/MacOS
	$(CP) app/Info.plist $(APP)/Contents
	$(CP) app/thumb.icns $(APP)/Contents/Resources

	$(RSYNC) --exclude .svn data $(APP)/Contents/Resources

	$(CP) /opt/local/lib/libSDL-1.2.0.dylib  $(APP)/Contents/Frameworks
	$(CP) /opt/local/lib/libfreetype.6.dylib $(APP)/Contents/Frameworks
	$(CP) /opt/local/lib/libjpeg.62.dylib    $(APP)/Contents/Frameworks
	$(CP) /opt/local/lib/libpng12.0.dylib    $(APP)/Contents/Frameworks
	$(CP) /opt/local/lib/libz.1.dylib        $(APP)/Contents/Frameworks

#------------------------------------------------------------------------------
