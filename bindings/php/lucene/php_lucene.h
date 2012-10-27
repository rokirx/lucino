/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2003 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Robert Kirchgessner                                          |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_LUCENE_H
#define PHP_LUCENE_H

extern zend_module_entry lucene_module_entry;
#define phpext_lucene_ptr &lucene_module_entry

#ifdef PHP_WIN32
#define PHP_LUCENE_API __declspec(dllexport)
#else
#define PHP_LUCENE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(lucene);
PHP_MSHUTDOWN_FUNCTION(lucene);
PHP_RINIT_FUNCTION(lucene);
PHP_RSHUTDOWN_FUNCTION(lucene);
PHP_MINFO_FUNCTION(lucene);

PHP_FUNCTION(confirm_lucene_compiled); /* For testing, remove later. */
PHP_FUNCTION(lucene_open_directory);
PHP_FUNCTION(lucene_create_index_searcher);
PHP_FUNCTION(lucene_create_term);
PHP_FUNCTION(lucene_create_term_query);
PHP_FUNCTION(lucene_create_boolean_query);
PHP_FUNCTION(lucene_create_phrase_query);
PHP_FUNCTION(lucene_create_prefix_query);
PHP_FUNCTION(lucene_boolean_query_add);
PHP_FUNCTION(lucene_boolean_query_to_string);
PHP_FUNCTION(lucene_phrase_query_add_term);
PHP_FUNCTION(lucene_index_searcher_search);
PHP_FUNCTION(lucene_index_searcher_sort_by);
PHP_FUNCTION(lucene_hits_length);
PHP_FUNCTION(lucene_hits_doc);
PHP_FUNCTION(lucene_hits_score);
PHP_FUNCTION(lucene_document_get);
PHP_FUNCTION(lucene_create_german_stemmer);
PHP_FUNCTION(lucene_german_stemmer_stem);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(lucene)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(lucene)
*/

/* In every utility function you add that needs to use variables 
   in php_lucene_globals, call TSRM_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as LUCENE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define LUCENE_G(v) TSRMG(lucene_globals_id, zend_lucene_globals *, v)
#else
#define LUCENE_G(v) (lucene_globals.v)
#endif

#endif	/* PHP_LUCENE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
