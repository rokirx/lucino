dnl config.m4 for extension lucene

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(lucene, for lucene support,
[  --with-lucene[=DIR]      Include lucene support])

if test "$PHP_LUCENE" != "no"; then

 # --with-lucene -> check with-path
 echo "teSt $PHP_LUCENE"
 SEARCH_PATH="/usr/local /usr"     # you might want to change this
 echo "GEF $LUCENE_DIR"
 SEARCH_FOR="include/lucene.h"  # you most likely want to change this
 if test -r $PHP_LUCENE/; then # path given as parameter
   LUCENE_DIR=$PHP_LUCENE
 else # search default path list
   AC_MSG_CHECKING([for lucene files in default path])
   for i in $SEARCH_PATH ; do
     if test -r $i/$SEARCH_FOR; then
       LUCENE_DIR=$i
       AC_MSG_RESULT(found in $i)
     fi
   done
 fi

 echo "GEF $LUCENE_DIR"

 if test -z "$LUCENE_DIR"; then
   AC_MSG_RESULT([not found])
   AC_MSG_ERROR([Please reinstall the lucene distribution])
 fi

 # --with-lucene -> add include path
 PHP_ADD_INCLUDE($LUCENE_DIR/include)

 # --with-lucene -> check for lib and symbol presence
 LIBNAME=lucene # you may want to change this
 LIBSYMBOL=lucene # you most likely want to change this 

  PHP_SUBST(LUCENE_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(lucene, $LUCENE_DIR/lib, LUCENE_SHARED_LIBADD)
dnl  AC_CHECK_LIB(lucene, BZ2_bzerror, [AC_DEFINE(HAVE_BZ2,1,[ ])], [AC_MSG_ERROR(bz2 module requires libbz2 >= 1.0.0)],)

dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
dnl  [
dnl    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LUCENE_DIR/lib, LUCENE_SHARED_LIBADD)
dnl    AC_DEFINE(HAVE_LUCENELIB,0,[ ])
dnl  ],[
dnl    AC_MSG_ERROR([wrong lucene lib version or lib not found])
dnl  ],[
dnl    -L$LUCENE_DIR/lib -lm -ldl
dnl  ])

dnl  PHP_SUBST(LUCENE_SHARED_LIBADD)
 PHP_NEW_EXTENSION(lucene, lucene.c, $ext_shared)
fi
