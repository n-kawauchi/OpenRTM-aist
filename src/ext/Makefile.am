## -*- Makefile -*-
##---------------------------------------------------------------------------
## Makefile.am for external libraries
##
## $Id$
##---------------------------------------------------------------------------

AUTOMAKE_OPTIONS = 1.4

if ENABLE_OMNISSL
SUBDIRS = local_service ec sdo ssl
DIST_SUBDIRS = local_service ec sdo logger ssl
else
SUBDIRS = local_service ec sdo
DIST_SUBDIRS = local_service ec sdo logger
endif


