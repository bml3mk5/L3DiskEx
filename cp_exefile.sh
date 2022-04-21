#!/bin/sh
APP=l3diskex
OS=`uname -s`
if [ "$OS" = "Darwin" ]; then
	DIR=./bin/macosx/Release
	mkdir -p $DIR
	cp -p ./Release/l3diskex.app/Contents/MacOS/$APP $DIR
fi
if [ "$OS" = "Linux" ]; then
	MA=`uname -m`
	if [ "$MA" = "x86_64" ]; then
		DIR=./bin/linux64
	else
		DIR=./bin/linux32
	fi
	mkdir -p $DIR
	cp -p ./Release/$APP $DIR
fi
