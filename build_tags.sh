#!/bin/bash

rm TAGS
dr=`pwd`
for i in src test
  do
    cd $i;
    for k in `find -name "*.[ch]"`
      do
        etags -a -o $dr/TAGS $k
      done
    cd $dr
  done;
