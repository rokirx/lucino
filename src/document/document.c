#include "lcn_index.h"
#include "document.h"
#include "field.h"

#define BITS2BYTE( x )  (((x)>>3) + ((x)%8 ? 1 : 0))

apr_pool_t *
lcn_document_get_pool( lcn_document_t *document)
{
    return document->pool;
}

lcn_list_t *
lcn_document_get_fields( lcn_document_t *document )
{
    return document->field_list;
}

float
lcn_document_get_boost( lcn_document_t *document )
{
    return document->boost;
}

lcn_bool_t
lcn_document_field_exists( lcn_document_t *document,
                           const char *field_name )
{
    unsigned int i;

    for( i = 0; i < lcn_list_size( document->field_list ); i++ )
    {
        lcn_field_t *field = lcn_list_get( document->field_list, i );

        if ( 0 == strcmp( field_name, lcn_field_name( field )))
        {
            return LCN_TRUE;
        }
    }

    return LCN_FALSE;
}

apr_status_t
lcn_document_get_field ( lcn_document_t *document,
                         const char *field_name,
                         lcn_field_t **field )
{
    unsigned int i;

    for( i = 0; i < lcn_list_size( document->field_list ); i++ )
    {
        *field = lcn_list_get( document->field_list, i );

        if ( 0 == strcmp( field_name, lcn_field_name( *field )))
        {
            return APR_SUCCESS;
        }
    }

    return LCN_ERR_DOCUMENT_NO_SUCH_FIELD;
}

apr_status_t
lcn_document_get_binary_field_value( lcn_document_t *document,
                                     const char *field_name,
                                     char **binary,
                                     unsigned int *len,
                                     apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        lcn_field_t *field;

        LCNCE( lcn_document_get_field( document, field_name, &field ) );
        LCNASSERTM( lcn_field_is_binary( field ), LCN_ERR_INVALID_ARGUMENT, field_name );
        LCNCE( lcn_field_binary_value( field, binary, len, pool ));
    }
    while( 0 );

    return s;
}

apr_status_t
lcn_document_get_binary_field_values( lcn_document_t* document,
                                      const char* field_name,
                                      lcn_list_t** list_binary_values,
                                      apr_pool_t* pool )
{
    apr_status_t s;
    lcn_list_t *list_fields;
    int i = 0;

    do
    {
        LCNCE( lcn_list_create( list_binary_values, 2, pool ) );
        list_fields = lcn_document_get_fields( document );

        for( i = 0; i < lcn_list_size( list_fields ); i++ )
        {
            char* field_binary_value;
            lcn_field_t *field = lcn_list_get( list_fields, i );

            if ( 0 == strcmp( field_name, lcn_field_name( field ) ) )
            {
                unsigned int field_size = lcn_field_size( field );

                LCNASSERTM( lcn_field_is_binary( field ), LCN_ERR_INVALID_ARGUMENT, field_name );
                LCNCE( lcn_field_binary_value( field,
                                               &field_binary_value,
                                               &field_size,
                                               pool ) );

                LCNCE( lcn_list_add( *list_binary_values, field_binary_value ) );
            }
        }
        LCNCE( s );
    }
    while( 0 );

    return s;
}

apr_status_t
lcn_document_get_int( lcn_document_t *document,
                      const char *field_name,
                      unsigned int *val )
{
    apr_status_t s;
    lcn_field_t *field;

    LCNCR( lcn_document_get_field( document, field_name, &field ) );
    LCNCR( lcn_field_int_value( field, val ));

    return s;
}

unsigned int
lcn_document_id( lcn_document_t* document )
{
    return document->index_pos;
}

lcn_score_t
lcn_document_score( lcn_document_t* document )
{
    return document->score;
}

apr_status_t
lcn_document_get( lcn_document_t* document,
                  char** result,
                  const char* field_name,
                  apr_pool_t* pool )
{
    apr_status_t s;
    lcn_field_t* field;

    LCNRM( lcn_document_get_field( document, field_name, &field ), field_name );

    if( lcn_field_is_binary( field ) )
    {
        return LCN_ERR_DOCUMENT_FIELD_IS_BINARY;
    }

    LCNPR( *result = apr_pstrdup( pool, lcn_field_value( field ) ), APR_ENOMEM );

    return s;
}

apr_status_t
lcn_document_add_field( lcn_document_t *document,
                        lcn_field_t *field )
{
    apr_status_t s;

    LCNCR( lcn_list_add( document->field_list, field ) );

    return s;
}

apr_status_t
lcn_document_create ( lcn_document_t **document,
                      apr_pool_t *pool )
{
    apr_status_t s;

    LCNPR( *document = (lcn_document_t*) apr_pcalloc( pool, sizeof(lcn_document_t) ),
           APR_ENOMEM );

    LCNCR( lcn_list_create( &((*document)->field_list), 10, pool ));

    (*document)->boost = (float) 1.0;
    (*document)->score.int_val = 0;
    (*document)->score.float_val = 0.0f;
    (*document)->pool = pool;

    return s;
}
