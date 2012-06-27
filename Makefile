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

osxdist : $(TARG)

	mkdir -p lib

	$(CP) /opt/local/lib/libSDL-1.2.0.dylib  lib
	$(CP) /opt/local/lib/libGLEW.1.7.0.dylib lib
	$(CP) /opt/local/lib/libfreetype.6.dylib lib
	$(CP) /opt/local/lib/libbz2.1.0.dylib    lib
	$(CP) /opt/local/lib/libjpeg.8.dylib     lib
	$(CP) /opt/local/lib/libpng14.14.dylib   lib
	$(CP) /opt/local/lib/libz.1.dylib        lib

	$(CP) src/libthumb.dylib                 lib

	$(CH) /opt/local/lib/libSDL-1.2.0.dylib  @executable_path/lib/libSDL-1.2.0.dylib  lib/libthumb.dylib
	$(CH) /opt/local/lib/libGLEW.1.7.0.dylib @executable_path/lib/libGLEW.1.7.0.dylib lib/libthumb.dylib
	$(CH) /opt/local/lib/libfreetype.6.dylib @executable_path/lib/libfreetype.6.dylib lib/libthumb.dylib
	$(CH) /opt/local/lib/libbz2.1.0.dylib    @executable_path/lib/libbz2.1.0.dylib    lib/libthumb.dylib
	$(CH) /opt/local/lib/libjpeg.8.dylib     @executable_path/lib/libjpeg.8.dylib     lib/libthumb.dylib
	$(CH) /opt/local/lib/libpng14.14.dylib   @executable_path/lib/libpng14.14.dylib   lib/libthumb.dylib
	$(CH) /opt/local/lib/libz.1.dylib        @executable_path/lib/libz.1.dylib        lib/libthumb.dylib

	$(CP) proj/panoview/orbiter .

	$(CH) /opt/local/lib/libSDL-1.2.0.dylib  @executable_path/lib/libSDL-1.2.0.dylib  orbiter
	$(CH) /opt/local/lib/libGLEW.1.7.0.dylib @executable_path/lib/libGLEW.1.7.0.dylib orbiter
	$(CH) /opt/local/lib/libfreetype.6.dylib @executable_path/lib/libfreetype.6.dylib orbiter
	$(CH) /opt/local/lib/libbz2.1.0.dylib    @executable_path/lib/libbz2.1.0.dylib    orbiter
	$(CH) /opt/local/lib/libjpeg.8.dylib     @executable_path/lib/libjpeg.8.dylib     orbiter
	$(CH) /opt/local/lib/libpng14.14.dylib   @executable_path/lib/libpng14.14.dylib   orbiter
	$(CH) /opt/local/lib/libz.1.dylib        @executable_path/lib/libz.1.dylib        orbiter
	$(CH) libthumb.dylib                     @executable_path/lib/libthumb.dylib      orbiter

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
