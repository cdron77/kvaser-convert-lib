# Makefile to build the Kvaser conversion library for LynXLog.

# CFLAGS="-I$(PWD)/linuxcan/include -fpermissive"
DESTDIR=
PREFIX=usr

# As canlib does, import library name and version
include linuxcan/canlib/version.mk

LIBRARY_CAN := $(LIBNAME).$(MAJOR).$(MINOR).$(BUILD)
LIBNAME_CAN := $(LIBNAME)
SONAME_CAN  := $(LIBNAME).$(MAJOR)

include kvlibsdk/kvadblib/version.mk

LIBRARY_KVADB := $(LIBNAME).$(MAJOR).$(MINOR).$(BUILD)
LIBNAME_KVADB := $(LIBNAME)
SONAME_KVADB  := $(LIBNAME).$(MAJOR)

include kvlibsdk/kvlclib/version.mk

LIBRARY_KVLC := $(LIBNAME).$(MAJOR).$(MINOR).$(BUILD)
LIBNAME_KVLC := $(LIBNAME)
SONAME_KVLC  := $(LIBNAME).$(MAJOR)

.PHONY:
	canlib clean all install_canlib install_kvlclib install

canlib:
	@cd ./linuxcan; $(MAKE) canlib

kvlclib:
	CFLAGS="-I$(CURDIR)/linuxcan/include -DOPENSSL_API_COMPAT=0x10100000L" $(MAKE) -C kvlibsdk kvlclib

clean:
	$(MAKE) -C linuxcan clean
	$(MAKE) -C kvlibsdk clean

all:	canlib kvlclib
	@echo "All done!"
	
install_canlib: canlib
	mkdir -p $(DESTDIR)/$(PREFIX)/lib
	mkdir -p $(DESTDIR)/$(PREFIX)/include
	install -m 644 linuxcan/canlib/$(LIBRARY_CAN) $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_CAN)
	ln -sfr $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_CAN) $(DESTDIR)/$(PREFIX)/lib/$(LIBNAME_CAN)
	ln -sfr $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_CAN) $(DESTDIR)/$(PREFIX)/lib/$(SONAME_CAN)
	install -m 644 linuxcan/include/canlib.h $(DESTDIR)/$(PREFIX)/include
	install -m 644 linuxcan/include/canstat.h $(DESTDIR)/$(PREFIX)/include
	install -m 644 linuxcan/include/obsolete.h $(DESTDIR)/$(PREFIX)/include
	mkdir -p $(DESTDIR)/$(PREFIX)/doc/canlib
	cp -r linuxcan/doc/HTMLhelp $(DESTDIR)/$(PREFIX)/doc/canlib

install_kvadblib: kvlclib
	# Create subdirs
	mkdir -p $(DESTDIR)/$(PREFIX)/lib
	mkdir -p $(DESTDIR)/$(PREFIX)/include
	# Install kvadblib
	install  -m 644 kvlibsdk/kvadblib/so.ndb/$(LIBRARY_KVADB) $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_KVADB)
	ln -sfr $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_KVADB) $(DESTDIR)/$(PREFIX)/lib/$(LIBNAME_KVADB)
	ln -sfr $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_KVADB) $(DESTDIR)/$(PREFIX)/lib/$(SONAME_KVADB)
	install -m 644 kvlibsdk/include/kvaDbLib.h $(DESTDIR)/$(PREFIX)/include

install_kvlclib: kvlclib install_kvadblib
	# Create subdirs
	mkdir -p $(DESTDIR)/$(PREFIX)/lib
	mkdir -p $(DESTDIR)/$(PREFIX)/include
	# Install kvlclib
	install  -m 644 kvlibsdk/kvlclib/so.ndb/$(LIBRARY_KVLC) $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_KVLC)
	ln -sfr $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_KVLC) $(DESTDIR)/$(PREFIX)/lib/$(LIBNAME_KVLC)
	ln -sfr $(DESTDIR)/$(PREFIX)/lib/$(LIBRARY_KVLC) $(DESTDIR)/$(PREFIX)/lib/$(SONAME_KVLC)
	install -m 644 kvlibsdk/include/kvlclib.h $(DESTDIR)/$(PREFIX)/include


install: install_canlib install_kvlclib
