#include "scorer.h"
#include "hit_collector.h"

struct lcn_scorer_private_t
{
    lcn_coordinator_t* coordinator;
    lcn_similarity_t* default_similarity;
    lcn_scorer_t* counting_sum_scorer;

    unsigned int min_nr_should_match;

    lcn_list_t* req_scorers;
    lcn_list_t* opt_scorers;
    lcn_list_t* prohib_scorers;
};

static apr_status_t
lcn_bs_add_prohibited_scorers( lcn_scorer_t* scorer,
                               lcn_scorer_t* req_count_sum_scorer,
                               lcn_scorer_t** result )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( lcn_list_size( p->prohib_scorers ) == 0 )
        {
            *result = req_count_sum_scorer;
            return APR_SUCCESS;
        }
        else
        {
            lcn_scorer_t* excl_scorer;

            if( lcn_list_size( p->prohib_scorers ) == 1 )
            {
                LCNPV( excl_scorer = lcn_list_get( p->prohib_scorers,
                                                   0 ),
                       LCN_ERR_INDEX_OUT_OF_RANGE );
            }
            else
            {
                LCNCE( lcn_disjunction_sum_scorer_create( &excl_scorer,
                                                          p->prohib_scorers,
                                                          1,
                                                          scorer->pool ) );

            }

            s = lcn_req_excl_scorer_create( result,
                                            req_count_sum_scorer,
                                            excl_scorer,
                                            scorer->pool );
            return s;
        }
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_bs_counting_conjunction_sum_scorer( lcn_scorer_t* scorer,
                                        lcn_list_t* required_scorers,
                                        lcn_scorer_t** result )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        lcn_scorer_t* cs;
        unsigned int i;

        LCNCE( lcn_counting_conjunction_sum_scorer_create( &cs,
                                                           scorer,
                                                           p->req_scorers,
                                                           scorer->pool ) );

        for( i = 0; i < lcn_list_size( p->req_scorers ); i++ )
        {
            lcn_scorer_t* act_rs = lcn_list_get( p->req_scorers, i );
            LCNPV( act_rs, LCN_ERR_NULL_PTR );
            LCNCE( lcn_conjunction_scorer_add( cs, act_rs ) );
        }

        *result = cs;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_bs_counting_disjunction_sum_scorer( lcn_scorer_t* scorer,
                                        lcn_list_t* scorers,
                                        unsigned int min_nr_should_match,
                                        lcn_scorer_t** result )
{
    return lcn_counting_disjunction_sum_scorer_create( result,
                                                       scorer,
                                                       scorers,
                                                       min_nr_should_match,
                                                       scorer->pool );
}

static apr_status_t
lcn_bs_dual_conjunction_sum_scorer( lcn_scorer_t* scorer,
                                    lcn_scorer_t* req1,
                                    lcn_scorer_t* req2,
                                    lcn_scorer_t** result )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_conjunction_scorer_create( result,
                                              scorer->priv->default_similarity,
                                              scorer->pool ) );

        LCNCE( lcn_conjunction_scorer_add( *result, req1 ) );
        LCNCE( lcn_conjunction_scorer_add( *result, req2 ) );

    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_bs_make_counting_sum_scorer_some_req( lcn_scorer_t* scorer,
                                          lcn_scorer_t** result )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {

        unsigned int i;
        lcn_list_t* all_req;
        lcn_list_t *opt_s = p->opt_scorers;
        lcn_list_t *req_s = p->req_scorers;

        if( lcn_list_size( opt_s ) < p->min_nr_should_match )
        {
            return lcn_non_matching_scorer_create( result, scorer->pool );
        }

        else if( lcn_list_size( opt_s ) == p->min_nr_should_match )
        {
            lcn_scorer_t* counting_conj_sum_scorer;

            LCNCE( lcn_list_create( &all_req,
                                    lcn_list_size( opt_s ) +
                                    lcn_list_size( req_s ),
                                    scorer->pool ) );

            for( i = 0; i < lcn_list_size( opt_s ); i++ )
            {
                LCNCE( lcn_list_add( all_req, lcn_list_get( opt_s, i ) ) );
            }

            LCNCE( s );

            for( i = 0; i < lcn_list_size( req_s ); i++ )
            {
                LCNCE( lcn_list_add( all_req, lcn_list_get( req_s, i ) ) );
            }
            LCNCE( s );
            LCNCE( lcn_bs_counting_conjunction_sum_scorer(
                       scorer,
                       all_req,
                       &counting_conj_sum_scorer )
                );

            s = lcn_bs_add_prohibited_scorers( scorer,
                                               counting_conj_sum_scorer,
                                               result );
            return s;
        }
        else
        {
            lcn_scorer_t* rcss;

            if( lcn_list_size( req_s ) == 1 )
            {
                LCNCE( lcn_single_match_scorer_create( &rcss,
                                                       lcn_list_get( req_s, 0 ),
                                                       p->coordinator,
                                                       scorer->similarity,
                                                       scorer->pool ) );
            }
            else
            {
                LCNCE( lcn_counting_conjunction_sum_scorer_create(
                           &rcss,
                           scorer,
                           req_s,
                           scorer->pool )
                    );
            }

            if( p->min_nr_should_match > 0 )
            {
                lcn_scorer_t* dcss, *cdss;

                LCNCE( lcn_bs_counting_disjunction_sum_scorer(
                           scorer,
                           opt_s,
                           p->min_nr_should_match,
                           &cdss )
                    );

                LCNCE( lcn_bs_dual_conjunction_sum_scorer( scorer,
                                                           rcss,
                                                           cdss,
                                                           &dcss ) );

                return lcn_bs_add_prohibited_scorers( scorer, dcss, result );
            }
            else
            {
                /* min_nr_should_match == 0 */

                lcn_scorer_t* rcss_prohib, *opt_scorer;

                LCNCE( lcn_bs_add_prohibited_scorers( scorer,
                                                      rcss,
                                                      &rcss_prohib ) );
                if( lcn_list_size( opt_s ) == 1 )
                {
                    LCNCE( lcn_single_match_scorer_create(
                               &opt_scorer,
                               lcn_list_get( opt_s, 0 ),
                               p->coordinator,
                               scorer->similarity,
                               scorer->pool ) );
                }
                else
                {
                    LCNCE( lcn_bs_counting_disjunction_sum_scorer(
                               scorer,
                               opt_s,
                               1,
                               &opt_scorer ) );
                }

                LCNCE( lcn_req_opt_sum_scorer_create(
                           result,
                           rcss_prohib,
                           opt_scorer,
                           scorer->pool ) );
            }
        }
    }
    while( FALSE );

    return s;
}


/*
 * Returns the scorer to be used for match counting and score summing.
 * Uses requiredScorers, optionalScorers and prohibitedScorers.
 */

static apr_status_t
lcn_bs_make_counting_sum_scorer_no_req( lcn_scorer_t* scorer,
                                        lcn_scorer_t** result )
{
    unsigned int nr_opt_required;
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    if( lcn_list_size( p->opt_scorers ) == 0 )
    {
        /* no clauses or only prohibited clauses */
        return lcn_non_matching_scorer_create( result, scorer->pool );
    }

    /* No required scorers. At least one optional scorer.
     * minNrShouldMatch optional scorers are required, but at least 1
     */
    nr_opt_required = ( p->min_nr_should_match < 1 )
        ? 1
        : p->min_nr_should_match;

    if( lcn_list_size( p->opt_scorers ) < nr_opt_required )
    {
        return lcn_non_matching_scorer_create( result, scorer->pool );
    }
    else
    {
        lcn_scorer_t* req_counting_sum_scorer;

        if( lcn_list_size( p->opt_scorers ) > nr_opt_required )
        {
            LCNCR( lcn_bs_counting_disjunction_sum_scorer(
                       scorer,
                       p->opt_scorers,
                       nr_opt_required,
                       &req_counting_sum_scorer ));
        }
        else
        {
            if( lcn_list_size( p->opt_scorers ) == 1 )
            {
                LCNCR( lcn_single_match_scorer_create(
                           &req_counting_sum_scorer,
                           (lcn_scorer_t*)lcn_list_get( p->opt_scorers, 0 ),
                           p->coordinator,
                           scorer->similarity,
                           scorer->pool ));
            }
            else
            {
                LCNCR( lcn_bs_counting_conjunction_sum_scorer(
                           scorer,
                           p->opt_scorers,
                           &req_counting_sum_scorer ));
            }
        }

        s = lcn_bs_add_prohibited_scorers( scorer,
                                           req_counting_sum_scorer,
                                           result );
        return s;
    }
}

static apr_status_t
lcn_bs_make_counting_sum_scorer( lcn_scorer_t* scorer,
                                 lcn_scorer_t** result )
{
    lcn_scorer_private_t* p = scorer->priv;

    return ( lcn_list_size( p->req_scorers ) == 0 )
        ? lcn_bs_make_counting_sum_scorer_no_req( scorer, result )
        : lcn_bs_make_counting_sum_scorer_some_req( scorer, result );
}

/* Initialize the match counting scorer that sums all the
 * scores.
 * When "counting" is used in a name it means counting the number
 * of matching scorers.
 * When "sum" is used in a name it means score value summing
 * over the matching scorers
 */

static apr_status_t
lcn_bs_init_counting_sum_scorer( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        LCNCE( lcn_coordinator_init( p->coordinator ) );
        LCNCE( lcn_bs_make_counting_sum_scorer( scorer, &(p->counting_sum_scorer) ) );
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_bs_next( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( NULL == p->counting_sum_scorer )
        {
            LCNCE( lcn_bs_init_counting_sum_scorer( scorer ) );
        }

        s = lcn_scorer_next( p->counting_sum_scorer );

        if( s && s!= LCN_ERR_NO_SUCH_DOC )
        {
            LCNCE( s );
        }
    }
    while( FALSE );

    return s;
}

static unsigned int
lcn_bs_doc( lcn_scorer_t* scorer )
{
    return lcn_scorer_doc( scorer->priv->counting_sum_scorer );
}

static apr_status_t
lcn_bs_skip_to( lcn_scorer_t* scorer, unsigned int target )
{
    apr_status_t s;

    if( NULL == scorer->priv->counting_sum_scorer )
    {
        LCNCR( lcn_bs_init_counting_sum_scorer( scorer ) );
    }

    return lcn_scorer_skip_to( scorer->priv->counting_sum_scorer, target );
}

static apr_status_t
lcn_bs_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    apr_status_t s;
    lcn_score_t sum = {0};
    lcn_scorer_private_t* p = scorer->priv;

    lcn_coordinator_init_doc( p->coordinator );
    LCNCR( lcn_scorer_score_get( p->counting_sum_scorer, &sum ) );
    score->float_val = sum.float_val * lcn_coordinator_coord_factor( p->coordinator );

    /* TODO: score->int_val: check if this is what we wish */

    score->int_val = sum.int_val;

    return APR_SUCCESS;
}

apr_status_t
lcn_boolean_scorer_add( lcn_scorer_t* scorer,
                        lcn_scorer_t* to_add,
                        lcn_bool_t required,
                        lcn_bool_t prohibited )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    if( !prohibited )
    {
        p->coordinator->max_coord++;
    }

    do
    {
        if( required )
        {
            if( prohibited )
            {
                LCNCM( LCN_ERR_INVALID_ARGUMENT,
                       "boolean_scorer cannot be required and prohibited" );
            }
            LCNCE( lcn_list_add( p->req_scorers, to_add ) );
        }
        else if( prohibited )
        {
            LCNCE( lcn_list_add( p->prohib_scorers, to_add ) );
        }
        else
        {
            LCNCE( lcn_list_add( p->opt_scorers, to_add ) );
        }
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_ordered_scorer_add( lcn_scorer_t* scorer,
                        lcn_scorer_t* to_add )
{
    lcn_scorer_private_t* p = scorer->priv;

    p->coordinator->max_coord++;

    return lcn_list_add( p->opt_scorers, to_add );
}

static apr_status_t
lcn_bs_score( lcn_scorer_t* scorer,
              lcn_hit_collector_t* hc )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( NULL == p->counting_sum_scorer )
        {
            LCNCE( lcn_bs_init_counting_sum_scorer( scorer ) );
        }

        while( APR_SUCCESS == ( s = lcn_scorer_next( p->counting_sum_scorer ) ) )
        {
            lcn_score_t score = {0};
            unsigned int doc = lcn_scorer_doc( p->counting_sum_scorer );
            LCNCE( lcn_scorer_score_get( scorer, &score ) );
            LCNCE( lcn_hit_collector_collect( hc, doc, score ) );
        }
    }
    while( FALSE );

    if( s )
    {
        s = ( LCN_ERR_NO_SUCH_DOC == s ) ? APR_SUCCESS : s;
    }
    return s;
}

static apr_status_t
lcn_bs_one_hit_score( lcn_scorer_t* scorer,
                      lcn_hit_collector_t* hc )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( NULL == p->counting_sum_scorer )
        {
            LCNCE( lcn_bs_init_counting_sum_scorer( scorer ) );
        }

        while( APR_SUCCESS == ( s = lcn_scorer_next( p->counting_sum_scorer ) ) )
        {
            unsigned int doc;
            lcn_score_t score = {0};
            doc = lcn_scorer_doc( p->counting_sum_scorer );
            LCNCE( lcn_scorer_score_get( scorer, &score ) );
            LCNCE( lcn_hit_collector_collect( hc, doc, score ) );

            if ( 0 < lcn_hit_collector_total_hits( hc ))
            {
                break;
            }
        }
    }
    while( FALSE );

    if( s )
    {
        s = ( LCN_ERR_NO_SUCH_DOC == s ) ? APR_SUCCESS : s;
    }
    return s;
}

static apr_status_t
lcn_bs_score_max( lcn_scorer_t* scorer,
                  lcn_hit_collector_t* hc,
                  unsigned int max,
                  lcn_bool_t* more_possible )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        unsigned int doc_nr = lcn_scorer_doc( p->counting_sum_scorer );

        while( doc_nr < max )
        {
            lcn_score_t score = {0};

            LCNCE( lcn_scorer_score_get( scorer, &score ) );
            lcn_hit_collector_collect( hc, doc_nr, score );

            if( APR_SUCCESS !=
                ( s = lcn_scorer_next( p->counting_sum_scorer ) ) )
            {
                *more_possible = FALSE;
                break;
            }
            doc_nr = lcn_scorer_doc( p->counting_sum_scorer );
        }
        *more_possible = TRUE;
    }
    while( FALSE );

    return s;
}

lcn_coordinator_t*
lcn_boolean_scorer_coordinator( lcn_scorer_t* scorer )
{
    return scorer->priv->coordinator;
}

lcn_similarity_t*
lcn_boolean_scorer_default_similarity( lcn_scorer_t* scorer )
{
    return scorer->priv->default_similarity;
}

apr_status_t
lcn_boolean_scorer_create( lcn_scorer_t** scorer,
                           lcn_similarity_t* similarity,
                           unsigned int min_nr_should_match,
                           lcn_query_type_t type,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;
        LCNCE( lcn_default_scorer_create( scorer, similarity, pool ) );

        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ), APR_ENOMEM );
        LCNCE( lcn_coordinator_create( &(p->coordinator ), *scorer, pool ) );
        LCNCE( lcn_default_similarity_create( &( p->default_similarity ), pool ) );

        p->min_nr_should_match = min_nr_should_match;

        LCNCE( lcn_list_create( &(p->req_scorers), 10, pool ) );
        LCNCE( lcn_list_create( &(p->opt_scorers), 10, pool ) );
        LCNCE( lcn_list_create( &(p->prohib_scorers), 10, pool ) );

        (*scorer)->pool      = pool;

        (*scorer)->type      = "boolean_scorer";
        (*scorer)->next      = lcn_bs_next;
        (*scorer)->doc       = lcn_bs_doc;
        (*scorer)->skip_to   = lcn_bs_skip_to;
        (*scorer)->score_get = lcn_bs_score_get;
        (*scorer)->score     = (type == LCN_QUERY_TYPE_ONE_HIT ? lcn_bs_one_hit_score : lcn_bs_score );
        (*scorer)->score_max = lcn_bs_score_max;
        (*scorer)->priv = p;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_ordered_scorer_create( lcn_scorer_t** scorer,
                           lcn_similarity_t* similarity,
                           unsigned int min_nr_should_match,
                           lcn_query_type_t type,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;
        LCNCE( lcn_default_scorer_create( scorer, similarity, pool ) );

        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ), APR_ENOMEM );
        LCNCE( lcn_coordinator_create( &(p->coordinator ), *scorer, pool ) );
        LCNCE( lcn_default_similarity_create( &( p->default_similarity ), pool ) );

        p->min_nr_should_match = min_nr_should_match;

        LCNCE( lcn_list_create( &(p->req_scorers), 10, pool ) );
        LCNCE( lcn_list_create( &(p->opt_scorers), 10, pool ) );
        LCNCE( lcn_list_create( &(p->prohib_scorers), 10, pool ) );

        (*scorer)->pool      = pool;

        (*scorer)->type      = "ordered_scorer";
        (*scorer)->next      = lcn_bs_next;
        (*scorer)->doc       = lcn_bs_doc;
        (*scorer)->skip_to   = lcn_bs_skip_to;
        (*scorer)->score_get = lcn_bs_score_get;
        (*scorer)->score     = (type == LCN_QUERY_TYPE_ONE_HIT ? lcn_bs_one_hit_score : lcn_bs_score );
        (*scorer)->score_max = lcn_bs_score_max;
        (*scorer)->priv = p;
    }
    while( FALSE );

    return s;
}
