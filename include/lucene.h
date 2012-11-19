#ifndef LUCENE_H
#define LUCENE_H

#ifdef __cplusplus

#define BEGIN_C_DECLS extern "C" {
#define END_C_DECLS   }

#else /* !__cplusplus */

#define BEGIN_C_DECLS
#define END_C_DECLS

#ifndef bool
typedef int bool;
#endif

#endif /* __cplusplus */

/*

j=0; for i in `wc -l [A-Z]*c | grep c | cut -c4-8`; do j=$(($i + $j));  done; echo $j

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <apr_errno.h>
#include <apr_file_info.h>
#include <apr_file_io.h>
#include <apr_strings.h>
#include <apr_hash.h>
#include <apr_time.h>
#include <apr.h>
#include <apr_lib.h>
#include <apr_pools.h>

#ifdef _WIN32
#include <windows.h>
#       ifdef LUCENE_IMPORT_DLL
#               define LUCENE_EXTERN __declspec(dllimport)
//#             define LUCENE_EXTERN
#               define LUCENE_API(func) func
#       elif LUCENE_EXPORT_DLL
#               define LUCENE_EXTERN __declspec(dllexport)
#               define LUCENE_API(func) func
# else
#         define LUCENE_API(func) func
#         define LUCENE_EXTERN
#       endif
#else
#       define LUCENE_API(func) func
#       define LUCENE_EXTERN
#endif

BEGIN_C_DECLS

/**
 * @defgroup lucene Lucene
 * @{
 */

/*@} */
#ifndef lcn_log_stream
extern FILE* lcn_log_stream;
#endif

#define JAVA_INTEGER_MAX_VALUE (2147483647)

#define BUCKET_SIZE            (1000)
#define PREFIX_QUERY_MAX_TERMS (1024)

#define RELEVANCE  (0)
#define NATURAL    (1)
#define ASCENDING  (0)
#define DESCENDING (1)


/* this constants define the token types of simple tokenizer */

#define ST_WORD     (0)
#define ST_NUM      (1)
#define ST_ABBREV   (2)
#define ST_STOPWORD (3)

#define LUCENE_BOOLEAN_QUERY        (1)
#define LUCENE_TERM_QUERY           (2)
#define LUCENE_PHRASE_QUERY         (3)
#define LUCENE_PREFIX_QUERY         (4)
#define LUCENE_PHRASE_PREFIX_QUERY  (5)
#define LUCENE_TERM_POS_QUERY       (6)

/* maximal assumed length of the segment file name */
#define MAX_LEN_SEGMENT_NAME (256)

#define LCN_INDEX_WRITER_DELETABLE_FILE_NAME  "deletable"
#define LCN_INDEX_WRITER_FIXED_SIZE_FIELD_DEF "fsf"

extern FILE* lcn_log_stream;
extern int CHECKx;


typedef char lcn_bool_t;
typedef char lcn_byte_t;
typedef struct file_entry lcn_file_entry_t;

#define LCN_TRUE  (1)
#define LCN_FALSE (0)
#define flog  fprintf(stderr,"%s, %s, %d: ",__FUNCTION__,apr_filepath_name_get(__FILE__),__LINE__);fprintf
#define nolog lcn_debug_no_out
//#define flog  fprintf

#define lcn_object_create( TYPE, POOL ) ((TYPE*) apr_pcalloc( (POOL), sizeof( TYPE ) ) )

#ifdef _LCNTRACE

#define LCNLOG( MSG )                                          \
            if( lcn_log_stream )                                        \
            {                                                           \
                apr_time_t now;                                         \
                unsigned int secs, msecs;                               \
                now = apr_time_now();                                   \
                secs  = (unsigned int) apr_time_sec(  now );            \
                msecs = (unsigned int) apr_time_msec( now );            \
                fprintf( lcn_log_stream, "%u.%u ", secs, msecs);        \
                fprintf( lcn_log_stream,                                \
                         "%s, %d: LOG: %s\n",                           \
                         apr_filepath_name_get( __FILE__ ),             \
                         __LINE__,                                      \
                         MSG );                                         \
            }

#define LCNLOG_STR( MSG, STR )                                          \
            if( lcn_log_stream )                                        \
            {                                                           \
                apr_time_t now;                                         \
                unsigned int secs, msecs;                               \
                now = apr_time_now();                                   \
                secs  = (unsigned int) apr_time_sec(  now );            \
                msecs = (unsigned int) apr_time_msec( now );            \
                fprintf( lcn_log_stream, "%u.%u ", secs, msecs);        \
                fprintf( lcn_log_stream,                                \
                         "%s, %d: LOG: %s : <%s>\n",                    \
                         apr_filepath_name_get( __FILE__ ),             \
                         __LINE__,                                      \
                         MSG, STR );                                    \
            }


#define LCNLOG_INT( MSG, I )                    \
    {                                           \
        char lcn_log_buf[20];                   \
        sprintf( lcn_log_buf, "%d", I );        \
        LCNLOG_STR( MSG, lcn_log_buf );         \
    }

#define LCNLOG_SIZE( MSG, I )                   \
    {                                           \
        char lcn_log_buf[20];                   \
        sprintf( lcn_log_buf, "%u", I );        \
        LCNLOG_STR( MSG, lcn_log_buf );         \
    }

#define LCNLOG_FLT( MSG, F )                    \
    {                                           \
        char lcn_log_buf[30];                   \
        sprintf( lcn_log_buf, "%.7f", F );      \
        LCNLOG_STR( MSG, lcn_log_buf );         \
    }

#define LCNLOG_STAT( MSG, S )                                   \
{                                                               \
    char error_buf[1000];                                       \
    LCNLOG_STR( MSG, lcn_strerror( S, error_buf, 1000 ) );      \
}

#define LCNLOG_BOOL( MSG, BOOL )                        \
{                                                       \
    LCNLOG_STR( MSG, ( BOOL ) ? "true" : "false" );     \
}

#define LCNLOG_QUERY( MSG, QUERY )                              \
{                                                               \
    apr_pool_t* sub_pool;                                       \
    char* query_str;                                            \
                                                                \
    apr_pool_create( &sub_pool, NULL );                         \
                                                                \
    lcn_query_to_string( QUERY, &query_str, "", sub_pool );     \
    fprintf( stderr, "%s %s\n", MSG, query_str );               \
    apr_pool_destroy( sub_pool );                               \
}

#define LCNCE( FUNC )                                                     \
        if( ( s = (FUNC) ) )                                              \
        {                                                                 \
            if( lcn_log_stream )                                          \
            {                                                             \
                char error_buf[1000];                                     \
                apr_time_t now = apr_time_now();                          \
                unsigned int secs = (unsigned int) apr_time_sec(  now );  \
                unsigned int msecs = (unsigned int) apr_time_msec( now ); \
                fprintf( lcn_log_stream, "%u.%u ", secs, msecs);          \
                fprintf( lcn_log_stream,                                  \
                         "%s, %d: %s <%d>\n",                             \
                         apr_filepath_name_get( __FILE__ ),               \
                         __LINE__,                                        \
                         lcn_strerror( s, error_buf, 1000 ), s );         \
            }                                                             \
            break;                                                        \
        }

#define LCNCM( FUNC, MSG )                                              \
        if( ( s = (FUNC) ) )                                            \
        {                                                               \
            if( lcn_log_stream )                                        \
            {                                                           \
                char error_buf[1000];                                   \
                apr_time_t now;                                         \
                unsigned int secs, msecs;                               \
                now = apr_time_now();                                   \
                secs  = (unsigned int) apr_time_sec(  now );            \
                msecs = (unsigned int) apr_time_msec( now );            \
                fprintf( lcn_log_stream, "%u.%u ", secs, msecs);        \
                fprintf( lcn_log_stream,                                \
                         "%s, %d: %s: %s <%d>\n",                       \
                         apr_filepath_name_get( __FILE__ ),             \
                         __LINE__,                                      \
                         MSG ,                                          \
                         lcn_strerror( s, error_buf, 1000 ), s );       \
            }                                                           \
            break;                                                      \
        }

#define LCNCR( FUNC )                                                   \
        if( ( s = (FUNC) ) )                                            \
        {                                                               \
            if( lcn_log_stream )                                        \
            {                                                           \
                char error_buf[1000];                                   \
                apr_time_t now;                                         \
                unsigned int secs, msecs;                               \
                now = apr_time_now();                                   \
                secs  = (unsigned int) apr_time_sec(  now );            \
                msecs = (unsigned int) apr_time_msec( now );            \
                fprintf( lcn_log_stream, "%u.%u ", secs, msecs);        \
                fprintf( lcn_log_stream,                                \
                         "%s, %d: %s <%d>\n",                           \
                         apr_filepath_name_get( __FILE__ ),             \
                         __LINE__,                                      \
                         lcn_strerror( s, error_buf, 1000 ), s );       \
            }                                                           \
            return s;                                                   \
        }

#define LCNRM( FUNC, MSG )                                              \
        if( ( s = (FUNC) ) )                                            \
        {                                                               \
            if( lcn_log_stream )                                        \
            {                                                           \
                char error_buf[1000];                                   \
                apr_time_t now;                                         \
                unsigned int secs, msecs;                               \
                now = apr_time_now();                                   \
                secs  = (unsigned int) apr_time_sec(  now );            \
                msecs = (unsigned int) apr_time_msec( now );            \
                fprintf( lcn_log_stream, "%u.%u ", secs, msecs);        \
                fprintf( lcn_log_stream,                                \
                         "%s, %d: %s: %s <%d>\n",                       \
                         apr_filepath_name_get( __FILE__ ),             \
                         __LINE__,                                      \
                         MSG ,                                          \
                         lcn_strerror( s, error_buf, 1000 ), s );       \
            }                                                           \
            return s;                                                   \
        }

#else
#define LCNLOG( MSG )
#define LCNLOG_STR( MSG, STR )
#define LCNLOG_INT( MSG, I )
#define LCNLOG_STAT( MSG, S )
#define LCNLOG_SIZE( MSG, I )
#define LCNLOG_QUERY( MSG, QUERY )
#define LCNCE(statement)         if((s=statement)){break;}
#define LCNCR(statement)         if((s=statement)){return s;}
#define LCNCM(statement, MSG )   if((s=statement)){break;}
#define LCNRM(statement, MSG )   if((s=statement)){return s;}
#endif

#define LCNPV( PTR, ERRCODE )      LCNCE( ( NULL == (PTR) ? ERRCODE : APR_SUCCESS) )

#define LCNPR( PTR, ERRCODE ) LCNCR( ( NULL == (PTR) ) ? ERRCODE : APR_SUCCESS )

#define LCNASSERT( EXPR, ERRCODE )  LCNCE( (    (EXPR)     ? APR_SUCCESS : ERRCODE) )
#define LCNASSERTR( EXPR, ERRCODE ) LCNCR( (    (EXPR)     ? APR_SUCCESS : ERRCODE) )
#define LCNASSERTM( EXPR, ERRCODE, MSG ) LCNCM( ((EXPR)     ? APR_SUCCESS : ERRCODE), MSG )

#define LCN_IS_CHAR(c)  (                                               \
                         ( c >= 'a' && c <= 'z' ) ||                    \
                         ( c >= 'A' && c <= 'Z' ) ||                    \
                         c == 'ä' || c == 'ü' || c == 'ö' ||            \
                         c == 'Ä' || c == 'Ü' || c == 'Ö' || c == 'ß' || \
                         c == 'é' || c == 'á' )

#define LCN_IS_UPPER(c) ( ( c >= 'A' && c <= 'Z' ) || c == 'Ä' || c == 'Ö' || c == 'Ü' )

#define LCN_IS_DIGIT(c) ( c >= '0' && c <= '9' )

#define LCN_IS_WHITESPACE(c) ( ( c == ' ' )  || \
                               ( c == '\t' ) || \
                               ( c == '\n' )  || \
                               ( c == '\r' ) )

/***************************************************************************
 *
 *            Definitions of abstract types
 *
 ***************************************************************************/

typedef struct lcn_analyzer_t lcn_analyzer_t;
typedef struct lcn_bitvector_t lcn_bitvector_t;
typedef struct lcn_directory_t lcn_directory_t;
typedef struct lcn_document_t lcn_document_t;
typedef struct lcn_field_info_t lcn_field_info_t;
typedef struct lcn_field_t lcn_field_t;
typedef struct lcn_index_input_t lcn_index_input_t;
typedef struct lcn_index_input_t lcn_index_input_t;
typedef struct lcn_list_t lcn_list_t;
typedef struct lcn_ostream_t lcn_ostream_t;
typedef struct lcn_ram_file_t lcn_ram_file_t;
typedef struct lcn_similarity_t lcn_similarity_t;
typedef struct lcn_term_enum_t lcn_term_enum_t;
typedef struct lcn_term_info_t lcn_term_info_t;
typedef struct lcn_term_t lcn_term_t;

typedef struct _lcn_index_writer_config_t lcn_index_writer_config_t;

typedef unsigned int lcn_field_type_t;
typedef unsigned int lcn_io_context_t;


/***************************************************************************
 *
 *                Atoms interface
 *
 ***************************************************************************/

/**
 * Represents a score of a hit
 */
struct lcn_score_t
{
    /**
     * Float value of the score
     */
    float float_val;

    /**
     * Discrete value of the score, used in the ordered query
     */
    char int_val;
};

typedef struct lcn_score_t lcn_score_t;

apr_status_t
lcn_atom_init ( apr_pool_t *pool );

void
lcn_debug_no_out( FILE *, const char* fmt, ... );

const char*
lcn_atom_get_str ( const char *str );


/**
 * @defgroup lcn_document Dokument
 * @ingroup lucene
 * @{
 */

/**
 * @defgroup lcn_document_desc Document
 * @ingroup lcn_document
 * @{
 */

/**
 * @brief Creates a new document
 *
 * @param document  Stores the new document
 * @param pool      APR pool
 */
apr_status_t
lcn_document_create( lcn_document_t **document,
                     apr_pool_t *pool );

/**
 * @brief Returns a field with the given name.
 *
 * If no such field exists, returns LCN_ERR_DOCUMENT_NO_SUCH_FIELD.
 *
 * @param document    Document to retrieve the field from
 * @param field_name  Name of the field to be retrieved
 * @param field       Stores new field
 */
apr_status_t
lcn_document_get_field( lcn_document_t *document,
                        const char * field_name,
                        lcn_field_t **field );

/**
 * @brief Returns an int value of a field
 *
 * @param document    Document to retrieve the field value from
 * @param field_name  Name of the field to retrieved
 * @param val         Integer value of the field
 */
apr_status_t
lcn_document_get_int( lcn_document_t *document,
                      const char *field_name,
                      unsigned int *val );

/**
 * @brief Returns true if the stored field exists
 *
 * @param document    Document to check
 * @param field_name  Field name to check
 */
lcn_bool_t
lcn_document_field_exists( lcn_document_t *document,
                           const char *field_name );


/**
 * @brief Adds a field to a document.
 *
 * A field with a name may be added only once, it is an error to
 * add a field a second time. If there is a non NULL pool provided,
 * then the field value is copied to a buffer from pool. If the pool
 * is NULL, then the field value pointer is stored in the document and it
 * is up to client code to assert, that the lifetime of the field value is
 * longer than the lifetime of the document.
 */
apr_status_t
lcn_document_add_field( lcn_document_t *document,
                        lcn_field_t *field,
                        apr_pool_t *pool);

/**
 * @brief Retrieves the content of the first binary field that was found
 *
 * @param document   Lucene document
 * @param field_name Name of the field to retrieve the value of
 * @param binary     Stores the contents of the fields
 * @param len        Stores the size of the fields contents
 * @param pool       APR pool
 */
apr_status_t
lcn_document_get_binary_field_value( lcn_document_t *document,
                                     const char *field_name,
                                     char **binary,
                                     unsigned int *len,
                                     apr_pool_t *pool );

/**
 * @brief Retrieves the contents of all binary fields by fieldname
 *
 * @param document   Lucene document
 * @param field_name Name of the field to retrieve the value of
 * @param lcn_list_t Value of the found fields
 * @param pool       APR pool
 */
apr_status_t
lcn_document_get_binary_field_values( lcn_document_t* document, 
                                      const char* field_name, 
                                      lcn_list_t** list_binary_values,  
                                      apr_pool_t* pool );

/**
 * @brief  Retrieves the field value of the given field
 *
 * Works only for non binary fields. For binary fields use
 * #lcn_document_get_binary_field
 *
 * @param document   Lucene document
 * @param result     Stores the value of the field
 * @param field      Name of the field to retrieve the value of
 * @param pool       APR pool
 */
apr_status_t
lcn_document_get( lcn_document_t* document,
                  char** result,
                  const char* field,
                  apr_pool_t* pool );

/**
 * @brief Returns boost of the document
 *
 * @param document  Lucene document
 */
float
lcn_document_get_boost( lcn_document_t *document );

/**
 * @brief  Returns fields of the document
 *
 * The result list is a list of fields (#lcn_field_t)
 *
 * @param document   Document to fetch list of fields for.
 */
lcn_list_t *
lcn_document_get_fields( lcn_document_t *document );

/**
 * @brief Returns the calculated score of the document
 *
 * @param document Lucene document
 */
lcn_score_t
lcn_document_score( lcn_document_t* document );

/**
 * @brief Returns the pool of the document
 *
 * @param document Lucene document
 */
apr_pool_t *
lcn_document_pool( lcn_document_t *document);

/**
 * @brief This returns the internal document number
 */
unsigned int
lcn_document_id( lcn_document_t* document );


/** @} */
/** @} */

/**
 * @defgroup lcn_field Field
 * @ingroup lcn_document
 * @{
 */

/**
 * @brief  Returns the name of the field
 *
 * The field names are represented as atomic strings. These a the
 * strings which can be compared by pointers, because the atomic
 * strings exist only once in memory.
 *
 * @param field   The field to retrieve the name of.
 */
const char *
lcn_field_name( lcn_field_t *field );

/**
 * @brief  Returns the default field value
 *
 * If the field is not of fixed size, the the default value
 * is undefined and the function returns NULL. Otherwise
 * a real default value is return which can be NULL though,
 * meaning all bits of default value are 0.
 *
 * @param field  The field to retrieve the default value of.
 */
const char *
lcn_field_default_value( lcn_field_t *field );

/**
 * @brief   Gets the value of the field
 *
 * @param  field   Field to retrieve the value from
 */
const char *
lcn_field_value( lcn_field_t *field );

/**
 * @brief Copies the binary value of the field to a buffer
 *
 * If the field is not binary an error is triggered
 *
 * @param field    Lucene binary field
 * @param value    Stores the binary value
 * @param len      Stores the length of the value buffer in bytes
 * @param pool     APR pool
 */
apr_status_t
lcn_field_binary_value( lcn_field_t *field,
                        char **value,
                        unsigned int *len,
                        apr_pool_t *pool );

/**
 * @brief  Retrieves the int value of the field
 *
 * If the field is not fixed size an error is triggered
 *
 * @param field   Lucene fixed size field
 * @param val     Stores the int value of the field
 */
apr_status_t
lcn_field_int_value( lcn_field_t *field,
                     unsigned int *val );

/**
 * @brief   Returns true if the field is indexed (#LCN_FIELD_INDEXED)
 *
 * An indexed field can be searched using terms and queries.
 *
 * @param field   Field to get the property
 */
lcn_bool_t
lcn_field_is_indexed( lcn_field_t *field );

/**
 * @brief Not implemented yet
 */
lcn_bool_t
lcn_field_is_term_vector_stored( lcn_field_t *field );

/**
 * @brief Not implemented yet
 */
lcn_bool_t
lcn_field_store_position_with_term_vector( lcn_field_t *field );

/**
 * @brief Not implemented yet
 */
lcn_bool_t
lcn_field_store_offset_with_term_vector( lcn_field_t *field );

/**
 * @brief  Disable storing of norms for this field
 *
 * @param  field  The field for which to disable the norms
 */
lcn_bool_t
lcn_field_omit_norms( lcn_field_t *field );

/**
 * @brief  Returns the STORED property of the field
 *
 * If a field is STORED, then the field value can be retrieved
 * after finding the document by a query
 *
 * @param field   The field to ask the property
 */
lcn_bool_t
lcn_field_is_stored( lcn_field_t *field );

/**
 * @brief  Returns the BINARY property of the field
 *
 * @param field  The field to ask the property
 */
lcn_bool_t
lcn_field_is_binary( lcn_field_t *field );

/**
 * @brief   Returns the FIXED_SIZE property of the field
 *
 * The fixed size fields are stored in memory blocks of fixed
 * size. This allows very efficient access to fields. The size
 * is in bits.
 *
 * @param  field  The field to ask for property.
 */
lcn_bool_t
lcn_field_is_fixed_size( lcn_field_t *field );

/**
 * @brief   Return the size of the fixed size field
 *
 * If the field is binary, returns the length of the field
 * data.
 *
 * If the field is fixed size, returns the fixed size of
 * field data in bits.
 *
 * Otherwise returns the length of the field string data.
 */
unsigned int
lcn_field_size( lcn_field_t *field );

/**
 * @brief  Whether the field is tokenized
 *
 * @param  field  Underlying field
 */
lcn_bool_t
lcn_field_is_tokenized( lcn_field_t *field );

/**
 * @brief Set the analyzer for tokenized field
 *
 * @param field     Underlying field
 * @param analyzer  Analyzer to be used for tokenizing
 */
void
lcn_field_set_analyzer( lcn_field_t *field, lcn_analyzer_t *analyzer );

/**
 * @brief Gets the boost of the field
 *
 * @param field Field to get the properties
 */
float
lcn_field_get_boost( lcn_field_t *field );

/**
 * @brief Gets the analyzer of the field
 *
 * @param field     Field to get the analyzer
 * @param analyzer  Stores the analyzer
 */
apr_status_t
lcn_field_get_analyzer( lcn_field_t *field,
                        lcn_analyzer_t **analyzer );

/**
 * @depricated
 * @brief Gets the properties of the field as a bitmask
 *
 * @param field  Field to get the properties
 */
unsigned int
lcn_field_flags( lcn_field_t* field );

/**
 * @brief Sets field properties via a bitmask
 *
 * @param field   Field to set the properties of
 * @param flags   Bitmask with the properties
 */
apr_status_t
lcn_field_flags_set( lcn_field_t* field, unsigned int flags );


/**
 * @deprecated
 * @brief Constructs a Field
 *
 * @param field       Stores new field
 * @param name        field name
 * @param value       field value
 * @param flags       a bit mask defining field type
 * @param copy_value
 * @param pool        APR pool
 */
apr_status_t
lcn_field_create( lcn_field_t **field,
                  const char *name,
                  const char *value,
                  unsigned int flags,
                  unsigned int copy_value,
                  apr_pool_t *pool );

/**
 * @deprecated
 * @brief Constructs a binary field
 *
 * @param field      a field to initialize
 * @param name       field name
 * @param value      field value
 * @param copy_value on  #LCN_FIELD_VALUE_COPY the field value is copied in the field
 *                   on  #LCN_FIELD_NO_VALUE_COPY there is no copiing, the pointer to
 *                   is assigned as it is
 * @param size       size of the field data in bytes
 * @param pool       APR pool
 */
apr_status_t
lcn_field_create_binary( lcn_field_t **field,
                         const char *name,
                         const char *value,
                         unsigned int copy_value,
                         unsigned int size,
                         apr_pool_t *pool );

/**
 * @brief Constructs a binary field
 *
 * @param field       A field to initialize
 * @param name        Field name
 * @param value       Field value
 * @param value_size  Size of the field data in bytes
 * @param ft          Field type
 * @param copy_value  Flag indicating whether to copy the field value
 * @param pool        APR pool
 */
apr_status_t
lcn_field_create_binary_ft( lcn_field_t **field,
                            const char *name,
                            const char *value,
                            unsigned int value_size,
                            lcn_field_type_t ft,
                            apr_pool_t *pool );

/**
 * @brief Constructs a string field
 *
 * @param field       A field to initialize
 * @param name        Field name
 * @param value       Field value
 * @param ft          Field type
 * @param copy_value  Flag indicating whether to copy the field value
 * @param pool        APR pool
 */
apr_status_t
lcn_field_create_ft( lcn_field_t **field,
                     const char *name,
                     const char *value,
                     lcn_field_type_t ft,
                     apr_pool_t *pool );

/**
 * @brief  Create a field object with fixed binary value
 *
 * @param field         Stores new field
 * @param name          Name of the field to create
 * @param value         Field contents
 * @param default_value Default value must be provided for those documents,
 *                      which wont get this field set. Since a document must
 *                      have a fixed field value for technical reasons, the
 *                      default value must be taken.
 * @param size          Size of the field in bits
 * @param pool          APR pool
 */
apr_status_t
lcn_field_create_fixed_size( lcn_field_t **field,
                             const char *name,
                             const char *value,
                             const char *default_value,
                             unsigned int size,
                             apr_pool_t *pool );

/**
 * @brief  Create a field object with fixed binary value
 *
 * @param field         Stores new field
 * @param name          Name of the field to create
 * @param value         Field contents as an integer
 * @param default_value Default value must be provided for those documents,
 *                      which wont get this field set. Since a document must
 *                      have a fixed field value for technical reasons, the
 *                      default value must be taken.
 * @param size          Size of the field in bits
 * @param pool          APR pool
 */
apr_status_t
lcn_field_create_fixed_size_by_ints( lcn_field_t **field,
                                     const char *name,
                                     unsigned int value,
                                     unsigned int default_value,
                                     unsigned int size,
                                     apr_pool_t *pool );

/** @} */


typedef struct lcn_document_dump_iterator_t lcn_document_dump_iterator_t;

apr_status_t
lcn_document_create_from_dump( lcn_document_t** doc,
                               const lcn_document_dump_iterator_t *iterator,
                               const char* input,
                               const char** end_pos,
                               apr_pool_t* pool );

apr_status_t
lcn_document_dump_iterator_create( lcn_document_dump_iterator_t** iterator,
                                   const char* input,
                                   apr_hash_t* analyzers,
                                   apr_pool_t* pool );

apr_status_t
lcn_document_dump_iterator_next( lcn_document_dump_iterator_t* iterator,
                                 lcn_document_t** doc,
                                 apr_pool_t* pool );

/**
 * Opens a new file system directory which must exist.
 * The *new_dir pointer is initialized with a valid
 * struct pointer on success, the pointer is undefined
 * on failure.
 */
apr_status_t
lcn_fs_directory_create ( lcn_directory_t **new_dir,
                          const char *dir_name,
                          lcn_bool_t create,
                          apr_pool_t *pool );


apr_status_t 
lcn_cfs_directory_create( lcn_directory_t **new_dir,
                          lcn_directory_t *cf_base_dir,
                          const char *cf_name,
                          apr_pool_t *pool);

/**
 * Creates a new directory in RAM.
 */
apr_status_t
lcn_ram_directory_create( lcn_directory_t **new_dir,
                          apr_pool_t *pool );

apr_status_t
lcn_directory_rename_file( lcn_directory_t *directory,
                           const char* old_name,
                           const char* new_name );

apr_status_t
lcn_directory_file_exists ( const lcn_directory_t *directory,
                            const char *file_name,
                            lcn_bool_t *flag );

apr_status_t
lcn_directory_list( const lcn_directory_t *directory,
                    lcn_list_t **file_list,
                    apr_pool_t *pool );

apr_status_t
lcn_directory_segments_format( lcn_directory_t *directory,
                               int *format );

apr_status_t
lcn_directory_delete_files( lcn_directory_t *directory,
                            lcn_list_t *list_file_names );

/**
 * Opens a file in the given directory and initializes the pointer
 * to the corresponding lcn_index_input_t
 *
 * @param new_in lcn_index_input_t to initialize
 * @param self   Directory where to create new lcn_index_input_t
 * @param file_name Name of the file in the directory
 */
apr_status_t
lcn_directory_open_input( lcn_directory_t *directory,
                          lcn_index_input_t **new_in,
                          const char  *file_name,
                          apr_pool_t *pool );


/**
 * Returns a stream reading an existing file, with the
 * specified read buffer size.  The particular Directory
 * implementation may ignore the buffer size.  Currently
 * the only Directory implementations that respect this
 * parameter are {@link FSDirectory} and {@link
 * CompoundFileDirectory}.
 */
apr_status_t
lcn_directory_open_input_( lcn_directory_t *directory,
                           const char *file_name,
                           lcn_io_context_t io_context,
                           lcn_index_input_t **index_input,
                           apr_pool_t *pool );

/**
 * Creates a new file in the directory with the given name. If the
 * file already exists it is deleted.
 *
 * @param            dir directory to contain newly created file
 * @param            new_os pointer container for output stream of the
 * @param file_name  name of the file to be created
 */
apr_status_t
lcn_directory_create_output( lcn_directory_t *directory,
                             lcn_ostream_t **new_os,
                             const char *file_name,
                             apr_pool_t *pool );

apr_status_t
lcn_directory_open_segment_file ( lcn_directory_t *directory,
                                  lcn_index_input_t **new_in,
                                  const char *segment,
                                  const char *ext,
                                  apr_pool_t *pool );

lcn_bool_t
lcn_directory_is_open( lcn_directory_t *directory );

apr_status_t
lcn_directory_delete_file( lcn_directory_t *directory,
                           const char  *file_name );

apr_status_t
lcn_directory_close( lcn_directory_t *directory );

apr_status_t
lcn_directory_remove( lcn_directory_t *directory );

/**
 * @brief Directory name as string for debugging
 *        purposes.
 *
 * @param directory  underlying object
 * @param pool       APR-pool
 */
char *
lcn_directory_name( lcn_directory_t *directory,
                    apr_pool_t *pool );

/**
 * @defgroup lcn_term Term
 * @ingroup search
 * @{
 */

/**
 * If given as a text_copy_flag parameter to lcn_term_create function,
 * the term_text is copied to a buffer from pool.
 */
#define LCN_TERM_TEXT_COPY    (1)

/**
 * If given as a text_copy_flag parameter to lcn_term_create function,
 * the term_text is assigned to internal member of term, so term does
 * not own the memory of term_text. This is useful to avoid copying where
 * possible.
 */
#define LCN_TERM_NO_TEXT_COPY (0)

/**
 * @brief Creates a new Term
 *
 * @param new_term  Stores the new term
 * @param field     Field name of the new term
 * @param text      Term text
 * @param text_copy_flag if #LCN_TERM_TEXT_COPY, the term text is duplicated
                         if #LCN_TERM_NO_TEXT_COPY otherwise it is just
 *                        assigned to internal term pointer
 * @param pool     APR-pool
 *
 * @result Lucene status value
 */
apr_status_t
lcn_term_create ( lcn_term_t **new_term,
                  const char *field,
                  const char *text,
                  int text_copy_flag,
                  apr_pool_t *pool );

/**
 * @brief Retrieves text value of the term
 *
 * @param term  Term from which the text is to be retrieved
 */
const char *
lcn_term_text ( const lcn_term_t *term );

/**
 * @brief Retrieves field value of the term
 *
 * @param term  Term from which the field name is to be retrieved
 */
const char *
lcn_term_field ( const lcn_term_t *term );

/**
 * @brief Compares the terms in alphabetical order
 *
 * Compares fields of the terms first. If they are equal,
 * compares the text values. Two terms are equals if the
 * fields and texts of both terms are equal.
 *
 * @param t1 First term to compare
 * @param t2 Second term to compare
 */
int
lcn_term_compare ( const lcn_term_t *t1, const lcn_term_t *t2 );

/**
 * @brief Create a copy of term
 *
 * @param term  Term to be cloned
 * @param clone Stores the cloned term
 * @param pool  APR pool
 */
apr_status_t
lcn_term_clone( const lcn_term_t* term,
                lcn_term_t** clone,
                apr_pool_t* pool );

/** @} */


apr_status_t
lcn_term_enum_next( lcn_term_enum_t *term_enum );

const lcn_term_t *
lcn_term_enum_term( lcn_term_enum_t *term_enum );

apr_status_t
lcn_term_enum_close( lcn_term_enum_t *term_enum );

apr_uint64_t
lcn_term_enum_size( lcn_term_enum_t *term_enum );

unsigned int
lcn_term_enum_doc_freq( lcn_term_enum_t *term_enum );

lcn_bool_t
lcn_field_info_is_indexed( lcn_field_info_t *field_info );

lcn_bool_t
lcn_field_info_fixed_size( lcn_field_info_t *field_info );

lcn_bool_t
lcn_field_info_omit_norms( lcn_field_info_t *field_info );

lcn_bool_t
lcn_field_info_store_term_vector( lcn_field_info_t *field_info );

lcn_bool_t
lcn_field_info_store_offset_with_term_vector( lcn_field_info_t *field_info );

lcn_bool_t
lcn_field_info_store_position_with_term_vector( lcn_field_info_t *field_info );

const char*
lcn_field_info_name( lcn_field_info_t *field_info );

/**
 * @defgroup error_handling Error handling declarations
 * @ingroup lucene
 * @{
 */

#if HAVE_ERRNO_H
#  include <errno.h>
#endif /*HAVE_ERRNO_H*/
#ifndef errno
/**
 * Define errno if not yet defined
 */
extern int errno;
#endif

#if !defined(LCN_ERROR_ENUM_DEFINED)
#undef LCN_ERROR_BUILD_ARRAY
#include "lcn_error_codes.h"
#endif

/**
 * @brief  Returns error message for some error status
 *
 * The string value of max length bufsize is stored in preallocated
 * buffer buf, for convenience the buf is returnd to be immediatly used
 * for printing.
 *
 * @param statcode   Error code
 * @param buf        Buffer to store the error message
 * @param bufsize    Size of buffer to contain the string
 */
char* lcn_strerror(apr_status_t statcode,
                   char *buf,
                   unsigned int bufsize);
/** @} */

/**
 * @defgroup lucene_priv Lucene (private interfaces)
 * @{
 */
/** @} */


END_C_DECLS

#endif /* LUCENE_H */
