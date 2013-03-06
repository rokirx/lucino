#include "lcn_index.h"

#define LCN_DOCDUMP_DOC    "lcn_document"
#define LCN_DOCDUMP_FIELD  "lcn_field"
#define LCN_DOCDUMP_PROPS  "properties"
#define LCN_DOCDUMP_AZ     "lcn_analyzer"
#define LCN_DOCDUMP_VAL    "value"
#define LCN_DOCDUMP_BINVAL "bin_value"
#define LCN_DOCDUMP_DEFVAL "default_value"
#define LCN_DOCDUMP_SIZE   "size"

#define SKIP_WHITESPACES( STR ) { while( *STR && LCN_IS_WHITESPACE( *STR ) ) STR++; }
#define PVB( STR, E ) if( NULL == ( STR ) || '\0' == *(STR) ) { s = E; break; }
#define PVR( STR ) if( NULL == ( STR ) || '\0' == *(STR) ) return LCN_ERR_DOCUMENT_DUMP_FMT;
#define MAX_LINE_LEN ( 1024 )

struct lcn_document_dump_iterator_t
{
    apr_pool_t *pool;
    const char* pos;
    apr_hash_t* analyzers;
    lcn_list_t *excluded_fields;
};

apr_status_t
lcn_document_dump_iterator_add_excluded_field( lcn_document_dump_iterator_t *iterator,
                                               const char* field )
{
    apr_status_t s;

    do
    {
        char *field_name;

        if ( NULL == iterator->excluded_fields )
        {
            LCNCE( lcn_list_create( &(iterator->excluded_fields), 5, iterator->pool ));
        }

        LCNPV( field_name = apr_pstrdup( iterator->pool, field ), APR_ENOMEM );
        LCNCE( lcn_list_add( iterator->excluded_fields, field_name ));
    }
    while(0);

    return s;
}

apr_status_t
lcn_document_read_properties_line( const char* input,
                                   const char** end_pos,
                                   lcn_field_type_t* field_type )
{
    const char* start, *end;
    char buf[MAX_LINE_LEN];

    PVR( start = strstr( input, LCN_DOCDUMP_PROPS ) );
    PVR( end = strchr( start, '\n' ) );

    apr_snprintf( buf, ( end - start + 1 ), "%s", start );

    if( strstr( buf, "BINARY"     ) )    { lcn_field_type_set_binary( field_type, LCN_TRUE );      }
    if( strstr( buf, "INDEXED"    ) )    { lcn_field_type_set_indexed( field_type, LCN_TRUE );     }
    if( strstr( buf, "OMIT_NORMS" ) )    { lcn_field_type_set_omit_norms( field_type, LCN_TRUE );  }
    if( strstr( buf, "STORED"     ) )    { lcn_field_type_set_stored( field_type, LCN_TRUE );      }
    if( strstr( buf, "TOKENIZED"  ) )    { lcn_field_type_set_tokenized( field_type, LCN_TRUE );   }
    if( strstr( buf, "FIXED_SIZE" ) )    { lcn_field_type_set_fixed_size( field_type, LCN_TRUE );  }

    *end_pos = ( end + 1 );

    return APR_SUCCESS;

}


static apr_status_t
lcn_document_read_document_line( const char* input,
                                 const char** end_pos,
                                 unsigned int* n_fields )
{
    const char* start, *end, *loop;
    char buf[MAX_LINE_LEN];

    PVR( start = strstr( input, LCN_DOCDUMP_DOC ) );
    PVR( end = strchr( start, '\n' ) );

    apr_snprintf( buf, ( end - start + 1), "%s", start );

    lcn_string_purge_whitespaces( buf );
    PVR( loop = strstr( buf, "=" ) );

    *n_fields = (unsigned int)atoi( (loop+1 ) );
    *end_pos = ( end + 1 );

    return APR_SUCCESS;
}

static apr_status_t
lcn_document_read_field_line( const char* input,
                              char** name,
                              const char** end_pos,
                              apr_pool_t* pool )
{
    apr_pool_t* cp;
    apr_status_t s;
    const char* start, *end;
    char* buf, *loop, *val_end = NULL;

    PVR( start = strstr( input, LCN_DOCDUMP_FIELD ) );
    PVR( end = strchr( start, '\n' ) );

    *end_pos = ( end + 1 );

    do
    {
        char* ustr;

        LCNCE( apr_pool_create( &cp, pool ) );

        LCNPV( buf = apr_pstrndup( pool,  start, (end-start) ), APR_ENOMEM );

        PVR( loop = strstr( buf, "=" ) );
        PVR( loop = strstr( buf, "\"" ) );
        loop++;
        PVR( loop );

        PVR( val_end = (char*)lcn_string_next_unescaped( loop, '"', '\\' ) );
        *val_end = 0;
        LCNCE( lcn_string_unescape( &ustr, loop, "\"", '\\', pool ) );
        *name = ustr;
    }
    while( 0 );

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

static apr_status_t
lcn_document_read_analyzer_line( const char* input,
                                 const char** end_pos,
                                 lcn_analyzer_t** analyzer,
                                 apr_hash_t* analyzers )
{
    const char* start, *end, *loop;
    char buf[MAX_LINE_LEN];

    PVR( start = strstr( input, LCN_DOCDUMP_AZ ) );
    PVR( end = strchr( start, '\n' ) );

    apr_snprintf( buf, ( end - start + 1), "%s", start );

    lcn_string_purge_whitespaces( buf );
    PVR( loop = strstr( buf, "=" ) );
    loop++;
    PVR( loop );

    *end_pos = ( end + 1 );
    if( NULL == (*analyzer = apr_hash_get( analyzers, loop,
                                           APR_HASH_KEY_STRING ) ) )
    {
        fprintf( stderr, "<<<%s>>>\n", loop );
        return LCN_ERR_DOCUMENT_DUMP_NO_SUCH_ANALYZER;
    }

    return APR_SUCCESS;
}

static apr_status_t
lcn_document_read_size_line( const char* input,
                             const char** end_pos,
                             unsigned int *size )
{
    const char* start, *end, *loop;
    char buf[MAX_LINE_LEN];

    PVR( start = strstr( input, LCN_DOCDUMP_SIZE ) );
    PVR( end = strchr( start, '\n' ) );

    apr_snprintf( buf, ( end - start + 1), "%s", start );

    lcn_string_purge_whitespaces( buf );
    PVR( loop = strstr( buf, "=" ) );
    loop++;
    PVR( loop );
    *end_pos = ( end + 1 );
    *size = atoi( loop );

    return APR_SUCCESS;
}

static apr_status_t
lcn_document_read_string_value( const char* input,
                                const char** end_pos,
                                char** value,
                                apr_pool_t* pool )
{
    apr_pool_t* cp;

    apr_status_t s;
    const char* start, *end;
    char* buf, *result;

    PVR( start = strstr( input, LCN_DOCDUMP_VAL ) );
    PVR( start = strchr( start, '"' ) );

    start++;
    PVR( start );
    PVR( end = lcn_string_next_unescaped( start, '"', '\\' ) );
    *end_pos = ( end + 1 );

    do
    {
        LCNCE( apr_pool_create( &cp, pool ) );
        LCNPV( buf = apr_pstrndup( pool,  start, (end-start) ), APR_ENOMEM );
        LCNCE( lcn_string_unescape( &result, buf, "\"", '\\', pool ) );
        *value = result;
    }
    while( 0 );

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_document_read_bin_value( const char* input,
                             const char** end_pos,
                             char** data,
                             unsigned int* field_len,
                             const char *val_name,
                             apr_pool_t* pool )
{
    apr_status_t s;
    const char* start, *end, *loop;
    char* buf;
    char* result;
    unsigned int n, i;
    apr_pool_t* cp;

    do
    {
        LCNCE( apr_pool_create( &cp, pool ) );

        PVR( start = strstr( input, val_name ) );
        PVR( start = strchr( start, '[' ) );
        start++;
        PVR( start );

        SKIP_WHITESPACES( start );

        PVR( start );

        n = atoi( start );

        PVR( start = strchr( start, '{' ) );
        PVR( ( start++ ) );

        PVR( end   = strchr( start, '}' ) );
        LCNPV( buf = apr_pstrndup( cp, start, ( end-start+1 ) ), APR_ENOMEM );
        LCNPV( result = apr_palloc( pool, n ), APR_ENOMEM );
        lcn_string_purge_whitespaces( buf );
        loop = buf;

        for( i = 0; i < n; i++ )
        {
            PVR( loop );
            result[i] = atoi( loop );

            if( i < (n-1) )
            {
                PVR( loop = strchr( loop, ',' ) );
                loop++;
            }
        }

        PVR( end = strchr( end, '\n' ) );

        *end_pos = end + 1;
        *field_len = n;
        *data = result;
    }
    while( 0 );

    if( NULL != cp  )
    {
        apr_pool_destroy( cp );
    }
    return APR_SUCCESS;
}

lcn_bool_t
lcn_document_dump_iterator_field_is_excluded( const lcn_document_dump_iterator_t *iterator,
                                              const char* name )
{
    unsigned int i;

    if ( NULL != iterator->excluded_fields )
    {
        for( i = 0; i < lcn_list_size( iterator->excluded_fields ); i++ )
        {
            if ( 0 == strcmp( name, (char*) lcn_list_get( iterator->excluded_fields, i ) ) )
            {
                return LCN_TRUE;
            }
        }
    }

    return LCN_FALSE;
}

apr_status_t
lcn_document_create_from_dump( lcn_document_t** doc,
                               const lcn_document_dump_iterator_t *iterator,
                               const char* input,
                               const char** end_pos,
                               apr_pool_t* pool )
{
    apr_status_t s;
    lcn_document_t* result;
    const char* loop;
    unsigned int n_fields=0, i;

    loop = input;
    LCNCR( lcn_document_create( &result, pool ) );

    SKIP_WHITESPACES( loop );

    lcn_document_read_document_line( loop, &loop, &n_fields );

    for( i = 0; i < n_fields; i++ )
    {
        lcn_field_t* field;
        char* name=NULL, *value=NULL;
        lcn_analyzer_t* az = NULL;
        lcn_field_type_t field_type = {0};

        lcn_document_read_field_line( loop, &name, &loop, pool );
        lcn_document_read_properties_line( loop, &loop, &field_type );

        if ( lcn_field_type_is_fixed_size( &field_type ))
        {
            unsigned int size;
            char* data, *def_data = NULL;
            unsigned int data_len, def_data_len = 0;

            LCNCR( lcn_document_read_size_line( loop, &loop, &size ));

            LCNCE( lcn_document_read_bin_value( loop,
                                                &loop,
                                                &data,
                                                &data_len,
                                                LCN_DOCDUMP_BINVAL,
                                                pool ) );

            LCNCE( lcn_document_read_bin_value( loop,
                                                &loop,
                                                &def_data,
                                                &def_data_len,
                                                LCN_DOCDUMP_DEFVAL,
                                                pool ) );

            LCNCE( lcn_field_create_fixed_size( &field,
                                                name,
                                                data,
                                                def_data,
                                                size,
                                                pool ) );
        }
        else
        {
            if( lcn_field_type_is_tokenized( &field_type ))
            {
                LCNCR( lcn_document_read_analyzer_line( loop, &loop, &az,
                                                        iterator->analyzers ) );
            }

            if( !( lcn_field_type_is_binary( &field_type )))
            {
                LCNCE( lcn_document_read_string_value( loop, &loop, &value, pool ) );
                LCNCE( lcn_field_create( &field, name, apr_pstrdup(pool, value), &field_type, pool ) );

                if( az )
                {
                    lcn_field_set_analyzer( field, az );
                }
            }
            else
            {
                char* data;
                unsigned int data_len;

                LCNCE( lcn_document_read_bin_value( loop,
                                                    &loop,
                                                    &data,
                                                    &data_len,
                                                    LCN_DOCDUMP_BINVAL,
                                                    pool ) );

                LCNCE( lcn_field_create_binary( &field,
                                                name,
                                                data,
                                                data_len,
                                                pool ));
            }
        }

        SKIP_WHITESPACES( loop );
        *end_pos = loop;

        if ( ! lcn_document_dump_iterator_field_is_excluded( iterator, name ) )
        {
            LCNCE( lcn_document_add_field( result, field ) );
        }
    }

    *doc = result;
    return s;
}

apr_status_t
lcn_document_dump_iterator_create( lcn_document_dump_iterator_t** iterator,
                                   const char* input,
                                   apr_hash_t* analyzers,
                                   apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_document_dump_iterator_t* result;

        LCNPV( result = apr_pcalloc( pool, sizeof( lcn_document_dump_iterator_t ) ), APR_ENOMEM );

        result->pool = pool;
        result->pos = input;
        result->analyzers = analyzers;

        *iterator = result;
    }
    while( 0 );

    return s;
}

apr_status_t
lcn_document_dump_iterator_next( lcn_document_dump_iterator_t* iterator,
                                 lcn_document_t** doc,
                                 apr_pool_t* pool )
{
    apr_status_t s;
    const char* end_pos;
    lcn_document_t* result;
    const char* start, *end;

    if( *(iterator->pos) == '\0' )
    {
        return LCN_ERR_ITERATOR_NO_NEXT;
    }

    do
    {
        char* buf;
        start = strstr( iterator->pos, LCN_DOCDUMP_DOC );

        if( NULL == start )
        {
            return LCN_ERR_ITERATOR_NO_NEXT;
        }

        end = strstr( ( start + 1 ), LCN_DOCDUMP_DOC );

        if( NULL == end )
        {
            iterator->pos = ( start + strlen( start ) );
        }
        else
        {
            iterator->pos = end ;
        }

        buf = apr_pstrndup( pool, start, ( end - start ) );

        s = lcn_document_create_from_dump( &result,
                                           iterator,
                                           buf,
                                           &end_pos,
                                           pool );
    }
    while( 0 );

    *doc = result;

    return s;
}
