##
## Extl Makefile
##

# System-specific configuration is in system.mk
TOPDIR=.
include $(TOPDIR)/build/system-inc.mk

######################################

INCLUDES += $(LIBTU_INCLUDES) $(LUA_INCLUDES)

CFLAGS += $(XOPEN_SOURCE) $(C99_SOURCE)

SOURCES=readconfig.c luaextl.c misc.c

HEADERS=readconfig.h extl.h luaextl.h private.h types.h

TARGETS=libextl.a libextl-mkexports

######################################

include $(TOPDIR)/build/rules.mk

######################################

libextl.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $+
	$(RANLIB) $@

libextl-mkexports: libextl-mkexports.in
	sed "1s:LUA50:$(LUA):" $< > $@

install:
	$(INSTALLDIR) $(BINDIR)
	$(INSTALL) -m $(BIN_MODE) libextl-mkexports $(BINDIR)
	$(INSTALLDIR) $(LIBDIR)
	$(INSTALL) -m $(DATA_MODE) libextl.a $(LIBDIR)
	$(INSTALLDIR) $(INCDIR)
	for h in $(HEADERS); do \
		$(INSTALL) -m $(DATA_MODE) $$h $(INCDIR); \
	done
