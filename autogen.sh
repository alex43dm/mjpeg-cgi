#!/bin/sh
libtoolize --install --copy --force --automake
aclocal -I m4
autoconf
autoheader
automake --add-missing --copy --foreign --force-missing
