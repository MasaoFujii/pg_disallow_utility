# contrib/pg_disallow_utility/Makefile

MODULE_big = pg_disallow_utility
OBJS = pg_disallow_utility.o $(WIN32RES)
PGFILEDESC = "pg_disallow_utility - disallows utility commands to be executed"

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_disallow_utility
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
