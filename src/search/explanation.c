#include "explanation.h"

static apr_status_t
lcn_explanation_to_string_internal( lcn_explanation_t* explanation,
                                    unsigned int depth,
                                    lcn_string_buffer_t* sb )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        unsigned int i, j;

        for( i = 0; i < depth; i++ )
        {
            LCNCE( lcn_string_buffer_append( sb, "  " ) );
        }
        LCNCE( s );
        LCNCE( lcn_string_buffer_append_float( sb, explanation->value ) );
        LCNCE( lcn_string_buffer_append( sb, " = " ) );
        LCNCE( lcn_string_buffer_append( sb,
                                         lcn_explanation_description_get( explanation ) ) );
        LCNCE( lcn_string_buffer_append( sb, "\n" ) );

        for( j = 0; j < lcn_list_size( explanation->details ); j++ )
        {
            lcn_explanation_t* act_detail;

            act_detail = lcn_list_get( explanation->details, j );

            LCNCE( lcn_explanation_to_string_internal( act_detail,
                                                       depth + 1,
                                                       sb ) );
        }
        LCNCE( s );

    }
    while( FALSE );

    return s;
}

float
lcn_explanation_value_get( lcn_explanation_t* explanation )
{
    return explanation->value;
}

void
lcn_explanation_value_set( lcn_explanation_t* explanation, float value )
{
    explanation->value = value;
}


apr_status_t
lcn_explanation_description_set( lcn_explanation_t* explanation,
                                 const char* description )
{
    if( NULL == ( explanation->description =
                  apr_pstrdup( explanation->pool,
                               description ) ) )
    {
        return APR_ENOMEM;
    }

    return APR_SUCCESS;
}



const char*
lcn_explanation_description_get( lcn_explanation_t* explanation )
{
    if( explanation->description == NULL )
    {
        return "(null)";
    }
    return explanation->description;
}



lcn_list_t*
lcn_explanation_details_get( lcn_explanation_t* explanation )
{
    return explanation->details;
}


apr_status_t
lcn_explanation_to_string( lcn_explanation_t* explanation,
                           char** result,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_string_buffer_t* sb;

        LCNCE( lcn_string_buffer_create( &sb, pool ) );

        LCNCE( lcn_explanation_to_string_internal( explanation, 0, sb ) );

        LCNCE( lcn_string_buffer_to_string( sb, result, pool ) );
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_explanation_detail_add( lcn_explanation_t* explanation,
                            lcn_explanation_t* detail )
{
    lcn_list_add( explanation->details,
                  detail );

    return APR_SUCCESS;
}


apr_status_t
lcn_explanation_create( lcn_explanation_t** explanation,
                        apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *explanation = apr_pcalloc( pool, sizeof( lcn_explanation_t ) ),
               APR_ENOMEM );
        LCNCE( lcn_list_create( &((*explanation)->details ), 10, pool ) );
        (*explanation)->pool = pool;
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_explanation_create_values( lcn_explanation_t** explanation,
                               float value,
                               const char* description,
                               apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_explanation_create( explanation, pool ) );

        lcn_explanation_value_set( *explanation, value );
        LCNCE( lcn_explanation_description_set( *explanation, description ) );
    }
    while( FALSE );

    return s;
}



