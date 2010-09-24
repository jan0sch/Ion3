##
## libtu Makefile
##

# System-specific configuration is in system.mk
TOPDIR=.
include $(TOPDIR)/build/system-inc.mk

######################################

CFLAGS += $(C89_SOURCE) $(POSIX_SOURCE) 

SOURCES=misc.c output.c util.c optparser.c parser.c tokenizer.c \
        map.c obj.c objlist.c errorlog.c ptrlist.c rb.c \
        stringstore.c iterable.c setparam.c prefix.c

ifdef LIBTU_NO_ERRMSG
DEFINES += -DLIBTU_NO_ERRMSG
else
ifndef HAS_SYSTEM_ASPRINTF
SOURCES += snprintf_2.2/snprintf.c
DEFINES += -DHAVE_SNPRINTF
else
DEFINES += -DHAS_SYSTEM_ASPRINTF
endif
endif

TARGETS=libtu.a
TESTERS=tester tester2 tester3

######################################

include $(TOPDIR)/build/rules.mk

######################################

testers: $(TESTERS)

libtu.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $+
	$(RANLIB) $@
	
tester: tester.c libtu.a
	$(CC) $(CFLAGS) $< -L. -ltu -lm -o $@

tester2: tester2.c libtu.a
	$(CC) $(CFLAGS) $< -L. -ltu -lm -o $@

tester3: tester3.c libtu.a
	$(CC) $(CFLAGS) $< -L. -ltu -lm -o $@

_install:
	$(INSTALLDIR) $(LIBDIR)
	$(INSTALLDIR) $(INCDIR)/libtu
	$(INSTALL) -m $(DATA_MODE) libtu.a $(LIBDIR)
	for i in *.h; do \
		$(INSTALL) -m $(DATA_MODE) $$i $(INCDIR)/libtu; \
	done
