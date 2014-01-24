-include Makedefs

TARG= thumb

#------------------------------------------------------------------------------

$(TARG) : FORCE
	$(MAKE) -C data
	$(MAKE) -C src

FORCE :

clean :
	$(MAKE) -C src clean

doc :
	doxygen Doxyfile

.PHONY : doc

#------------------------------------------------------------------------------
