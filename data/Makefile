
# This Makefile creates a ZIP archive containing all of the necessary assets
# in the data directory. This archive is included in the linking of the Thumb
# library, thus guaranteeing the availability of these assets at run time.


DATA= $(subst ./,, $(shell find . -name \*.md   \
	                           -o -name \*.xml  \
	                           -o -name \*.ttf  \
	                           -o -name \*.obj  \
	                           -o -name \*.png  \
	                           -o -name \*.vert \
	                           -o -name \*.frag))

data.zip : $(DATA)
	rm -f data.zip
	zip -9 -r data.zip $(DATA)
