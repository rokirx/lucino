#include "scorer.h"


BEGIN_C_DECLS


static apr_status_t
lcn_default_scorer_score( lcn_scorer_t* scorer,
                          lcn_hit_collector_t* hc )
{
    apr_status_t s;

    do
    {
        while( APR_SUCCESS == ( s = scorer->next( scorer ) ) )
        {
            lcn_score_t score = {0};

            LCNCE( lcn_scorer_score_get( scorer, &score ) );
            LCNCE( lcn_hit_collector_collect( hc,
                                              lcn_scorer_doc( scorer ),
                                              score ) );
        }

        if( s )
        {
            s = ( s == LCN_ERR_NO_SUCH_DOC ) ? APR_SUCCESS : s;
            LCNCE( s );
        }
    }
    while( FALSE );

    return s;
}

const char*
lcn_scorer_type( lcn_scorer_t* scorer )
{
    return scorer->type;
}

static apr_status_t
lcn_default_scorer_score_max( lcn_scorer_t* scorer,
                              lcn_hit_collector_t* hc,
                              unsigned int max,
                              lcn_bool_t* more_possible )
{
    apr_status_t s;

    *more_possible = FALSE;

    do
    {
        while( ( lcn_scorer_doc( scorer ) < max ) &&
               ( APR_SUCCESS == ( s = scorer->next( scorer ) ) )
            )
        {
            lcn_score_t score = {0};

            LCNCE( lcn_scorer_score_get( scorer, &score ) );
            LCNCE( lcn_hit_collector_collect( hc,
                                              scorer->doc( scorer ),
                                              score ) );
        }

        s = scorer->next( scorer );

        *more_possible = ( s == APR_SUCCESS );

        if( s && s != LCN_ERR_NO_SUCH_DOC )
        {
            LCNCE( s );
        }
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_scorer_next( lcn_scorer_t* scorer )
{
    return scorer->next( scorer );
}

apr_status_t
lcn_scorer_score( lcn_scorer_t* scorer,
                  lcn_hit_collector_t* hc )
{
    apr_status_t s;
    s = scorer->score( scorer, hc );
    return s;
}

lcn_similarity_t*
lcn_scorer_similarity_get( lcn_scorer_t* scorer )
{
    return scorer->similarity;
}


apr_status_t
lcn_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score  )
{
    apr_status_t s = scorer->score_get( scorer, score );

    if ( scorer->order > 0 )
    {
        score->int_val = scorer->order;
    }

    return s;
}

unsigned int
lcn_scorer_doc( lcn_scorer_t* scorer )
{
    return scorer->doc( scorer );
}

lcn_similarity_t*
lcn_scorer_similarity( lcn_scorer_t* scorer )
{
    return scorer->similarity;
}

apr_status_t
lcn_scorer_skip_to( lcn_scorer_t* scorer,
                    unsigned int target )
{
    return scorer->skip_to( scorer, target );
}

apr_status_t
lcn_default_scorer_create( lcn_scorer_t** scorer,
                           lcn_similarity_t* sim,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *scorer = apr_pcalloc( pool,
                                      sizeof( lcn_scorer_t ) ),
               APR_ENOMEM );

        /* lcn_disjunction_sum_scorer doesn't need a similarity-Object, */
        /* therefore input-checking                                     */

        if( NULL != sim )
        {
            LCNCE( lcn_similarity_clone( sim,
                                         &((*scorer)->similarity),
                                         pool ) );
        }

        (*scorer)->score_get = lcn_scorer_score_get;
        (*scorer)->score     = lcn_default_scorer_score;
        (*scorer)->score_max = lcn_default_scorer_score_max;
    }
    while( FALSE );

    return s;
}

END_C_DECLS
