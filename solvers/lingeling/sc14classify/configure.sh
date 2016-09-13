#!/bin/sh
debug=no
while [ $# -gt 0 ]
do
  case $1 in
    -g) debug=yes;;
    *) echo "usage: configure.sh [-g]" 1>&2; exit 1;;
  esac
  shift
done
if [ $debug = yes ]
then
  COMPILE="gcc -Wall -g3"
else
  COMPILE="gcc -Wall -O3 -DNDEBUG"
fi
echo "$COMPILE"
sed -e "s,@COMPILE@,$COMPILE," makefile.in > makefile
