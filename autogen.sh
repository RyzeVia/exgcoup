#!/bin/sh
aclocal
libtoolize
aclocal
autoheader
automake --add-missing
autoconf
