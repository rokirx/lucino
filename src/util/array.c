#include "lcn_util.h"

apr_status_t
lcn_ptr_array_create( lcn_ptr_array_t** array,
                      unsigned int length,
                      apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *array = apr_pcalloc( pool, sizeof( lcn_ptr_array_t ) ),
               APR_ENOMEM );
        LCNPV( (*array)->arr = apr_palloc( pool, length * sizeof( void* ) ),
               APR_ENOMEM );

        (*array)->length = length;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_byte_array_create( lcn_byte_array_t** array,
                       unsigned int length,
                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *array = apr_pcalloc( pool, sizeof( lcn_byte_array_t ) ),
               APR_ENOMEM );
        LCNPV( (*array)->arr = apr_palloc( pool,
                                           length * sizeof( lcn_byte_t ) ),
               APR_ENOMEM );

        (*array)->length = length;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_int_array_create_impl( lcn_int_array_t** array,
                           unsigned int length,
                           lcn_bool_t fill_with_zeros,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *array = apr_pcalloc( pool, sizeof( lcn_int_array_t ) ),
               APR_ENOMEM );
        LCNPV( (*array)->arr = fill_with_zeros ?
                                 apr_pcalloc( pool, length * sizeof( int ) ) :
                                 apr_palloc( pool, length * sizeof( int ) ),
               APR_ENOMEM );

        (*array)->length = length;
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_zero_int_array_create( lcn_int_array_t** array,
                           unsigned int length,
                           apr_pool_t* pool )
{
    return lcn_int_array_create_impl( array, length, 1, pool );
}

apr_status_t
lcn_int_array_create( lcn_int_array_t** array,
                      unsigned int length,
                      apr_pool_t* pool )
{
    return lcn_int_array_create_impl( array, length, 0, pool );
}

apr_status_t
lcn_float_array_create( lcn_float_array_t** array,
                        unsigned int length,
                        apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *array = apr_pcalloc( pool, sizeof( lcn_float_array_t ) ),
               APR_ENOMEM );
        LCNPV( (*array)->arr = apr_palloc( pool, length * sizeof( float ) ),
               APR_ENOMEM );

        (*array)->length = length;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_size_array_create( lcn_size_array_t** array,
                       unsigned int length,
                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *array = apr_pcalloc( pool, sizeof( lcn_size_array_t ) ),
               APR_ENOMEM );
        LCNPV( (*array)->arr = apr_palloc( pool,
                                           length * sizeof( unsigned int ) ),
               APR_ENOMEM );

        (*array)->length = length;
    }
    while( FALSE );

    return s;
}
