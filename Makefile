# Makefile to build the Kvaser conversion library for LynXLog.

CFLAGS=-I$(PWD)/linuxcan/include

.PHONY:
	canlib clean all

canlib:
	@cd ./linuxcan; $(MAKE) canlib

kvlclib: canlib
	CFLAGS=-I$(PWD)/linuxcan/include $(MAKE) -C kvlibsdk kvlclib

clean:
	$(MAKE) -C linuxcan clean
	$(MAKE) -C kvlibsdk clean

all:	linuxcan kvlclib
	@echo "All done!"
