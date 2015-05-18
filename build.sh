#!/bin/sh
set -x

U=`uname`

if [ ${U} = FreeBSD ]; then
    INCLUDE="-I ../filemon"
else
    INCLUDE="-I ../filemon-linux"
fi

CFLAGS=-g
CFLAGS=
gcc ${CFLAGS} ${INCLUDE} -D${U} filemon_trace.c -o fmtrace

