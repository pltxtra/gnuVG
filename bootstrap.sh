#!/bin/sh

#########
#
#  This script runs
#  all needed commands to create
#  a configure script.
#
#########

autoheader
libtoolize --automake --ltdl
aclocal
autoconf
automake --add-missing

