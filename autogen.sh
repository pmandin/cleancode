#!/bin/sh

aclocal -I /usr/local/share/aclocal
autoheader
automake --copy --add-missing
autoconf

