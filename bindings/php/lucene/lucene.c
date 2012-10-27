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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_lucene.h"
#include <lucene.h>

/* If you declare any globals in php_lucene.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(lucene)
*/

/* True global resources - no need for thread safety here */
static int le_lucene;

static int le_lucene_fs_directory;
#define le_lucene_fs_directory_name "Lucene FSDirectory"

static int le_lucene_index_searcher;
#define le_lucene_index_searcher_name "Lucene IndexSearcher"

static int le_lucene_term;
#define le_lucene_term_name "Lucene Term"

static int le_lucene_query;
#define le_lucene_query_name "Lucene Query"

static int le_lucene_hits;
#define le_lucene_hits_name "Lucene Hits"

static int le_lucene_document;
#define le_lucene_document_name "Lucene Document"

static int le_lucene_german_stemmer;
#define le_lucene_german_stemmer_name "German Stemmer"

/* {{{ lucene_functions[]
 *
 * Every user visible function must have an entry in lucene_functions[].
 */
function_entry lucene_functions[] = {

	PHP_FE(confirm_lucene_compiled,	       NULL)  /* For testing, remove later. */
	PHP_FE(lucene_open_directory,	       NULL)
    PHP_FE(lucene_create_index_searcher,   NULL)
    PHP_FE(lucene_create_term,             NULL)
    PHP_FE(lucene_create_term_query,       NULL)
    PHP_FE(lucene_create_boolean_query,    NULL)
    PHP_FE(lucene_create_phrase_query,     NULL)
    PHP_FE(lucene_create_prefix_query,     NULL)
    PHP_FE(lucene_boolean_query_add,       NULL)
    PHP_FE(lucene_boolean_query_to_string, NULL)
    PHP_FE(lucene_phrase_query_add_term,   NULL)
    PHP_FE(lucene_index_searcher_search,   NULL)
    PHP_FE(lucene_index_searcher_sort_by,  NULL)
    PHP_FE(lucene_hits_length,             NULL)
    PHP_FE(lucene_hits_doc,                NULL)
    PHP_FE(lucene_hits_score,              NULL)
    PHP_FE(lucene_document_get,            NULL)
    PHP_FE(lucene_create_german_stemmer,   NULL)
    PHP_FE(lucene_german_stemmer_stem,     NULL)

	{NULL, NULL, NULL}	/* Must be the last line in lucene_functions[] */
};
/* }}} */

/* {{{ lucene_module_entry
 */
zend_module_entry lucene_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"lucene",
	lucene_functions,
	PHP_MINIT(lucene),
	PHP_MSHUTDOWN(lucene),
	PHP_RINIT(lucene),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(lucene),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(lucene),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LUCENE
ZEND_GET_MODULE(lucene)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("lucene.global_value",      "42", PHP_INI_ALL, OnUpdateInt, global_value, zend_lucene_globals, lucene_globals)
    STD_PHP_INI_ENTRY("lucene.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_lucene_globals, lucene_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_lucene_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_lucene_init_globals(zend_lucene_globals *lucene_globals)
{
	lucene_globals->global_value = 0;
	lucene_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ php_lucene_free_fs_directory
 */
static void php_lucene_free_fs_directory(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    lucene_FSDirectory *self = (lucene_FSDirectory *) rsrc->ptr;
    self->free( self );
}
/* }}} */

/* {{{ php_lucene_free_index_searcher
 */
static void php_lucene_free_index_searcher(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    lucene_IndexSearcher *self = (lucene_IndexSearcher *) rsrc->ptr;
    self->free( self );
}
/* }}} */

/* {{{ php_lucene_free_term
 */
static void php_lucene_free_term(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    lucene_Term *self = (lucene_Term *) rsrc->ptr;
    self->free( self );
}
/* }}} */

/* {{{ php_lucene_free_query
 */
static void php_lucene_free_query(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    lucene_Query *self = (lucene_Query *) rsrc->ptr;
    self->free( self );
}
/* }}} */

/* {{{ php_lucene_free_hits
 */
static void php_lucene_free_hits(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    lucene_Hits *self = (lucene_Hits *) rsrc->ptr;
    self->free( self );
}
/* }}} */

/* {{{ php_lucene_free_document
 */
static void php_lucene_free_document(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    lucene_Document *self = (lucene_Document *) rsrc->ptr;
    self->free( self );
}
/* }}} */

/* {{{ php_lucene_free_german_stemmer
 */
static void php_lucene_free_german_stemmer(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    lucene_GermanStemmer *self = (lucene_GermanStemmer *) rsrc->ptr;
    self->free( self );
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(lucene)
{
	/* If you have INI entries, uncomment these lines 
	ZEND_INIT_MODULE_GLOBALS(lucene, php_lucene_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	*/

    le_lucene_fs_directory = zend_register_list_destructors_ex(
        php_lucene_free_fs_directory, NULL, le_lucene_fs_directory_name, module_number);

    le_lucene_index_searcher = zend_register_list_destructors_ex(
        php_lucene_free_index_searcher, NULL, le_lucene_index_searcher_name, module_number);

    le_lucene_term = zend_register_list_destructors_ex(
        php_lucene_free_term, NULL, le_lucene_term_name, module_number);

    le_lucene_query = zend_register_list_destructors_ex(
        php_lucene_free_query, NULL, le_lucene_query_name, module_number);

    le_lucene_hits = zend_register_list_destructors_ex(
        php_lucene_free_hits, NULL, le_lucene_hits_name, module_number);

    le_lucene_document = zend_register_list_destructors_ex(
        php_lucene_free_document, NULL, le_lucene_document_name, module_number);

    le_lucene_german_stemmer = zend_register_list_destructors_ex(
        php_lucene_free_german_stemmer, NULL, le_lucene_german_stemmer_name, module_number);

    REGISTER_LONG_CONSTANT("LUCENE_RELEVANCE",  RELEVANCE,  CONST_CS );
    REGISTER_LONG_CONSTANT("LUCENE_NATURAL",    NATURAL,    CONST_CS );
    REGISTER_LONG_CONSTANT("LUCENE_ASCENDING",  ASCENDING,  CONST_CS );
    REGISTER_LONG_CONSTANT("LUCENE_DESCENDING", DESCENDING, CONST_CS );

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(lucene)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(lucene)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(lucene)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(lucene)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "lucene support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_lucene_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_lucene_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char string[256];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = sprintf(string, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "lucene", arg);
	RETURN_STRINGL(string, len, 1);
}
/* }}} */

/* {{{ proto resource lucene_open_directory( string arg )
   Return a resource representing an open directory */
PHP_FUNCTION(lucene_open_directory)
{

    lucene_FSDirectory *self;

    char *filename;
    int   filename_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    if (PG(safe_mode) && (!php_checkuid(filename, NULL, CHECKUID_ALLOW_FILE_NOT_EXISTS)))
    {
        RETURN_FALSE;
    }

    if (php_check_open_basedir(filename TSRMLS_CC))
    {
        RETURN_FALSE;
    }

    if ( (self = lucene_create_FSDirectory( filename )) == NULL){
        php_error(E_WARNING, "%s() Cannot open lucene file %s", 
                  get_active_function_name(TSRMLS_C), filename);
        RETURN_FALSE;
    }

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_fs_directory);
}
/* }}} */


/* {{{ proto resource lucene_create_index_searcher( resource arg )
   Return a resource representing an index_searcher */
PHP_FUNCTION(lucene_create_index_searcher)
{
    lucene_IndexSearcher *self;

    zval *resource;
    lucene_FSDirectory *directory;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(directory, lucene_FSDirectory *, &resource, -1,
                        le_lucene_fs_directory_name, le_lucene_fs_directory);

    self = lucene_create_IndexSearcher( directory );

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_index_searcher);
}
/* }}} */

/* {{{ proto resource lucene_create_term( string field, string text )
   Return a resource representing a term */
PHP_FUNCTION(lucene_create_term)
{
    lucene_Term *self;

    char *field, *text;
    int   field_len, text_len;
    int i;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &field, &field_len,
                              &text, &text_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    self = lucene_create_Term( field, text );

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_term);
}
/* }}} */

/* {{{ proto resource lucene_create_term_query( resource term )
   Return a resource representing a term query */
PHP_FUNCTION(lucene_create_term_query)
{
    lucene_Query *self;

    zval *resource;
    lucene_Term *term;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(term, lucene_Term *, &resource, -1,
                        le_lucene_term_name, le_lucene_term);

    self = lucene_create_TermQuery( term );

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_query);
}
/* }}} */

/* {{{ proto resource lucene_create_prefix_query( resource term )
   Return a resource representing a prefix query */
PHP_FUNCTION(lucene_create_prefix_query)
{
    lucene_Query *self;

    zval *resource;
    lucene_Term *term;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(term, lucene_Term *, &resource, -1,
                        le_lucene_term_name, le_lucene_term);

    self = lucene_create_PrefixQuery( term );

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_query);
}
/* }}} */

/* {{{ proto resource lucene_create_boolean_query()
   Return a resource representing a boolean query */
PHP_FUNCTION(lucene_create_boolean_query)
{
    lucene_Query *self = lucene_create_BooleanQuery();

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_query);
}
/* }}} */

/* {{{ proto resource lucene_create_phrase_query()
   Return a resource representing a phrase query */
PHP_FUNCTION(lucene_create_phrase_query)
{
    lucene_Query *self = lucene_create_PhraseQuery();

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_query);
}
/* }}} */


/* {{{ proto void lucene_boolean_query_add( resource self, resource query, boolean required, boolean prohibited )
*/
PHP_FUNCTION(lucene_boolean_query_add)
{
    zval *self_resource;
    lucene_Query *self;

    zval *resource;
    lucene_Query *query;

    zend_bool required, prohibited;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrbb", &self_resource, &resource, &required, &prohibited) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_Query *, &self_resource, -1,
                        le_lucene_query_name, le_lucene_query);

    ZEND_FETCH_RESOURCE(query, lucene_Query *, &resource, -1,
                        le_lucene_query_name, le_lucene_query);

    self->add( self, query, (int) required, (int) prohibited );
}
/* }}} */

/* {{{ proto void lucene_phrase_query_add_term( resource self, resource term )
*/
PHP_FUNCTION(lucene_phrase_query_add_term)
{
    zval *self_resource;
    lucene_Query *self;

    zval *resource;
    lucene_Term *term;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &self_resource, &resource) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_Query *, &self_resource, -1,
                        le_lucene_query_name, le_lucene_query);

    ZEND_FETCH_RESOURCE(term, lucene_Term *, &resource, -1,
                        le_lucene_term_name, le_lucene_term);
    self->add_term( self, term );
}
/* }}} */


/* {{{ proto resource lucene_index_searcher_search( resource index_searcher, resource query )
   Return a resource representing hits */
PHP_FUNCTION(lucene_index_searcher_search)
{
    zval *self_resource;
    lucene_IndexSearcher *self;

    zval *resource;
    lucene_Query *query;

    lucene_Hits *hits;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &self_resource, &resource) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_IndexSearcher *, &self_resource, -1,
                        le_lucene_index_searcher_name, le_lucene_index_searcher);

    ZEND_FETCH_RESOURCE(query, lucene_Query *, &resource, -1,
                        le_lucene_query_name, le_lucene_query);

    hits = self->search( self, query );

    ZEND_REGISTER_RESOURCE(return_value, hits, le_lucene_hits);
}
/* }}} */

/* {{{ proto void lucene_index_searcher_sort_by( resource index_searcher, long order, long mode )
 */
PHP_FUNCTION(lucene_index_searcher_sort_by)
{
    int arg_count = ARG_COUNT(ht);

    zval *self_resource;
    lucene_IndexSearcher *self;

    int order, mode;

    if (arg_count < 2 || arg_count > 3 )
    {
        WRONG_PARAM_COUNT;
    }

    if ( arg_count == 3 )
    {
        if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll", &self_resource, &order, &mode) == FAILURE)
        {
            RETURN_FALSE;
        }
    }else if ( arg_count == 2 )
    {
        if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &self_resource, &order) == FAILURE)
        {
            RETURN_FALSE;
        }

        mode = ASCENDING;
    }

    ZEND_FETCH_RESOURCE(self, lucene_IndexSearcher *, &self_resource, -1,
                        le_lucene_index_searcher_name, le_lucene_index_searcher);
    
    self->sort_by( self, order, mode );
}
/* }}} */


/* {{{ proto int lucene_hits_length( resource hits )
   Return number of hits */
PHP_FUNCTION(lucene_hits_length)
{
    zval *resource;
    lucene_Hits *self;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_Hits *, &resource, -1,
                        le_lucene_hits_name, le_lucene_hits);

    RETURN_LONG( self->length( self ) );
}
/* }}} */

/* {{{ proto resource lucene_hits_doc( resource hits, long n )
   Return number of hits */
PHP_FUNCTION(lucene_hits_doc)
{
    zval *resource;
    lucene_Hits *self;
    int n;

    lucene_Document *doc;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &resource, &n) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_Hits *, &resource, - 1,
                        le_lucene_hits_name, le_lucene_hits);

    doc = self->doc( self, n );

    ZEND_REGISTER_RESOURCE(return_value, doc, le_lucene_document);
}
/* }}} */

/* {{{ proto float lucene_hits_score( resource hits, long n )
   Return number of hits */
PHP_FUNCTION(lucene_hits_score)
{
    zval *resource;
    lucene_Hits *self;
    int n;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &resource, &n) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_Hits *, &resource, - 1,
                        le_lucene_hits_name, le_lucene_hits);

    RETURN_DOUBLE( self->score( self, n ) );
}
/* }}} */

/* {{{ proto string lucene_document_get( resource document, string field_name )
   Return number of hits */
PHP_FUNCTION(lucene_document_get)
{
    zval *resource;
    lucene_Document *self;
    char *field_name;
    int field_len, result_len;
    char *result;
    char *buffer;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &resource, &field_name, &field_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_Document *, &resource, - 1,
                        le_lucene_document_name, le_lucene_document);

    result = self->get( self, field_name );
    result_len = strlen( result );

    buffer  = emalloc( sizeof(char) * result_len + 1 );

    memcpy( buffer, result,  result_len + 1);
    
    RETURN_STRINGL( buffer, result_len, 0);
}
/* }}} */

/* {{{ proto string lucene_boolean_query_to_string( resource query )
   Return string representation of the boolean query number of hits */
PHP_FUNCTION(lucene_boolean_query_to_string)
{
    zval *resource;
    lucene_Query *self;
    char *field_name;
    int field_len, result_len;
    char *result;
    char *buffer;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &resource, &field_name, &field_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_Query *, &resource, - 1,
                        le_lucene_query_name, le_lucene_query );

    result = self->to_string( self, field_name );
    result_len = strlen( result );

    buffer  = emalloc( sizeof(char) * result_len + 1 );

    memcpy( buffer, result,  result_len + 1);
    
    RETURN_STRINGL( buffer, result_len, 0);
}
/* }}} */

/* {{{ proto resource lucene_create_german_stemmer()
   Return a resource representing a german stemmer */
PHP_FUNCTION(lucene_create_german_stemmer)
{
    lucene_GermanStemmer *self = lucene_create_GermanStemmer();

    ZEND_REGISTER_RESOURCE(return_value, self, le_lucene_german_stemmer);
}
/* }}} */

/* {{{ proto string lucene_german_stemmer_stem( resource german_stemmer, string term )
   Return stemmed term. Term must be lower case */
PHP_FUNCTION(lucene_german_stemmer_stem)
{
    zval *resource;
    lucene_GermanStemmer *self;
    char *term;
    int term_len;
    char *buffer;
    

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &resource, &term, &term_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(self, lucene_GermanStemmer *, &resource, - 1,
                        le_lucene_german_stemmer_name, le_lucene_german_stemmer);

    buffer = emalloc( sizeof(char) * term_len + 1 );

    memcpy( buffer, term, term_len );
    buffer[ term_len ] = '\0';
    self->stem( self, buffer );
    
    RETURN_STRINGL( buffer, strlen( buffer ), 0);
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
