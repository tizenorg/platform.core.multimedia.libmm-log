lib_LTLIBRARIES = libmm_log.la

includelibmm_logdir = $(includedir)/mm_log
includelibmm_log_HEADERS = mm_log.h

libmm_log_la_CFLAGS = -DEXPORT_API="__attribute__((visibility(\"default\")))"
libmm_log_la_SOURCES = logmanager.c
libmm_log_la_CFLAGS += 	$(DLOG_CFLAGS)

if USE_DLOG
libmm_log_la_CFLAGS += -DUSE_DLOG
else
libmm_log_la_LDFLAGS = -Wl,-init,__init_logmanager \
		       -Wl,-fini,__fini_logmanager \
		       -version-info $(LOGMANAGER_LIBVERSION)
endif
		       
libmm_log_la_LIBADD = $(libdir)/libiniparser.a \
		      $(DLOG_LIBS) \
		      -lrt 
libmm_log_la_CPPFLAGS = -DUSE_KERNEL_OBJECT

bin_PROGRAMS = mmlogviewer
noinst_HEADERS = mm_log.h

mmlogviewer_DEPENDENCIES = libmm_log.la 

mmlogviewer_SOURCES = logmanager_viewer.c
mmlogviewer_CFLAGS = 
mmlogviewer_LDADD = libmm_log.la \
		    -lrt



