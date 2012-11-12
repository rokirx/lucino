#include "lcn_analysis.h"

BEGIN_C_DECLS

apr_status_t
lcn_analyzer_map_create( apr_hash_t** map, apr_pool_t* pool )
{
    apr_status_t s;
    apr_hash_t* result;
    lcn_analyzer_t* one_az;

    result = apr_hash_make( pool );

    LCNCR( lcn_simple_analyzer_create( &one_az, pool ) );
    apr_hash_set( result, lcn_analyzer_type( one_az ),
                  APR_HASH_KEY_STRING, one_az );

    LCNCR( lcn_german_analyzer_create( &one_az, pool ) );
    apr_hash_set( result, lcn_analyzer_type( one_az ),
                  APR_HASH_KEY_STRING, one_az );

    LCNCR( lcn_whitespace_analyzer_create( &one_az, pool ) );
    apr_hash_set( result, lcn_analyzer_type( one_az ),
                  APR_HASH_KEY_STRING, one_az );

    *map = result;

    return s;
}

void
lcn_analyzer_map_add( apr_hash_t* map,
                      lcn_analyzer_t* analyzer )
{
    apr_hash_set( map, 
                  lcn_analyzer_type( analyzer ),
                  APR_HASH_KEY_STRING,
                  analyzer );
}

END_C_DECLS
