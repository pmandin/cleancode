#!/bin/sh

aclocal -I /usr/local/share/aclocal
autoheader
automake --add-missing
autoconf

