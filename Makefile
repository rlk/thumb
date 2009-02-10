TARG= thumb

CH= install_name_tool -change

#------------------------------------------------------------------------------

all :
	$(MAKE) -C src

clean :
	$(MAKE) -C src clean

osxdist :
	$(MAKE) -C src

	mkdir -p lib

	cp    /opt/local/lib/libSDL-1.2.0.dylib    lib
	cp    /opt/local/lib/libfreetype.6.dylib   lib
	cp    /opt/local/lib/libjpeg.62.dylib      lib
	cp    /opt/local/lib/libpng12.0.dylib      lib
	cp    /opt/local/lib/libz.1.dylib          lib

	$(CH) /opt/local/lib/libSDL-1.2.0.dylib    \
          @executable_path/lib/libSDL-1.2.0.dylib  $(TARG)
	$(CH) /opt/local/lib/libfreetype.6.dylib   \
          @executable_path/lib/libfreetype.6.dylib $(TARG)
	$(CH) /opt/local/lib/libjpeg.62.dylib      \
          @executable_path/lib/libjpeg.62.dylib    $(TARG)
	$(CH) /opt/local/lib/libpng12.0.dylib      \
          @executable_path/lib/libpng12.0.dylib    $(TARG)
	$(CH) /opt/local/lib/libz.1.dylib          \
          @executable_path/lib/libz.1.dylib        $(TARG)

#------------------------------------------------------------------------------
