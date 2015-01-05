
# This Makefile creates a ZIP archive containing all of the necessary assets
# in this directory. The archive is converted to C code and included during
# linking, thus guaranteeing the availability of these assets at run time.

DATA= $(subst ./,, $(shell find . -name \*.md   \
                               -o -name \*.xml  \
                               -o -name \*.ttf  \
                               -o -name \*.csv  \
                               -o -name \*.obj  \
                               -o -name \*.png  \
                               -o -name \*.vert \
                               -o -name \*.frag))

data.zip : $(DATA)
	zip -FS9r data.zip $(DATA)
