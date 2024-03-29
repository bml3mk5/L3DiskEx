#!/bin/sh

make_all() {
	if [ "$3" = "" ]; then
		make -f Makefile.$1 $2_clean
	fi
	make -f Makefile.$1 $2_install
}

svn update
UNAME=`uname -s`
UNAMEU=`echo $UNAME | cut -c 1-5`
if [ "$UNAME" = "Darwin" ]; then
	make_all macosx st $1
fi
if [ "$UNAME" = "Linux" ]; then
	make_all linux sh $1
fi
if [ "$UNAMEU" = "MINGW" ]; then
	make_all win st $1
fi

