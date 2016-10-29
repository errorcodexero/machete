#
# Makefile for building the SABRE repository.  This is used by jenkins to
# do periodic builds
#

WPILIB=/usr/local/wpilib

all: machete

machete::
	(cd util ; make)
	(cd input ; make)
	(cd control ; make)
	(cd executive ; make)
	(cd roborio ; make WPILIB=$(WPILIB))

clean::
	rm */*.o */*.a

