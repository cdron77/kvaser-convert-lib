# Makefile to build the Kvaser conversion library for LynXLog.

linuxcan:
	$(MAKE) -C linuxcan

kvlclib:
	$(MAKE) -C kvlibsdk
	
.PHONY:
	clean all

clean:
	$(MAKE) -C linuxcan clean

all:	linuxcan kvlclib
	@echo "All done!"
