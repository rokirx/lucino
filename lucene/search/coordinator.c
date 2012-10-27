#include "scorer.h"

/* use after all scorers have been added */

apr_status_t
lcn_coordinator_init( lcn_coordinator_t* c )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        lcn_similarity_t* sim = c->scorer->similarity;
        LCNCE( lcn_float_array_create( &( c->coord_factors ),
                                       c->max_coord + 1,
                                       c->pool ) );

        for( i = 0; i <= c->max_coord; i++ )
        {
            c->coord_factors->arr[i] = lcn_similarity_coord( sim,
                                                             i,
                                                             c->max_coord );
        }
    }
    while( FALSE );

    return s;
}

void
lcn_coordinator_init_doc( lcn_coordinator_t* c )
{
    c->nr_matchers = 0;
}

float
lcn_coordinator_coord_factor( lcn_coordinator_t* c )
{
    return c->coord_factors->arr[ c->nr_matchers ];
}

unsigned int
lcn_coordinator_max_coord( lcn_coordinator_t* coordinator )
{
    return coordinator->max_coord;
}

apr_status_t
lcn_coordinator_create( lcn_coordinator_t** c,
                        lcn_scorer_t* scorer,
                        apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *c = apr_pcalloc( pool, sizeof( lcn_coordinator_t ) ),
               APR_ENOMEM );
        LCNCE( apr_pool_create( &((*c)->pool ), pool ) );
        (*c)->scorer = scorer;
    }
    while( FALSE );

    return s;
}
