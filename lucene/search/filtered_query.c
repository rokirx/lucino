#include "lcn_search.h"
#include "lcn_index.h"
#include "weight.h"
#include "query.h"
#include "scorer.h"
#include "lcn_bitvector.h"


/* Start of SCORER IMPLEMENTATION */

#define SCORE_CACHE_SIZE (32)
#define INT_MAX_VALUE    (2147483647)

struct lcn_scorer_private_t
{
    lcn_weight_t* weight;
    lcn_term_docs_t* term_docs;

    float weight_value;
    unsigned int act_doc;
    unsigned int act_freq;
    unsigned int act_pos;

    unsigned int pointer;
    unsigned int pointer_max;

    lcn_float_array_t* score_cache;

    lcn_bitvector_t *bv;

    unsigned int pos;
};



static apr_status_t
lcn_filtered_scorer_score_max( lcn_scorer_t* scorer,
                               lcn_hit_collector_t* hc,
                               unsigned int end,
                               lcn_bool_t* more_possible )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        end = lcn_bitvector_size( p->bv );

        while( p->act_doc < end )
        {
            lcn_score_t score = {0};
            unsigned int i;
            unsigned int bv_size = lcn_bitvector_size( p->bv );

            score.float_val = 1.0;
            lcn_hit_collector_collect( hc, p->act_doc, score );

            *more_possible = LCN_FALSE;

            for( i = p->act_doc + 1 ; i < bv_size; i++ )
            {
                if ( lcn_bitvector_get_bit( p->bv, i ) )
                {
                    *more_possible = LCN_TRUE;
                    p->act_doc = i;
                    break;
                }
            }

            if ( i == bv_size )
            {
                *more_possible = LCN_FALSE;
                return s;
            }
        }

        *more_possible = TRUE;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_filtered_scorer_score( lcn_scorer_t* scorer, lcn_hit_collector_t* hc )
{
    lcn_bool_t dummy;
    apr_status_t s = scorer->next( scorer );

    /* this had to be changed after switching lcn_index_reader_terms_from to     */
    /* creating term_enum of length zero (i.e. first call to next returns false) */
    /* for the case of term being alphanumerically greater than the last term in */
    /* term infos.                                                               */
    /*                                                                           */
    /* In case of LCN_ERR_NO_SUCH_DOC we just have 0 hits.                       */

    if ( LCN_ERR_NO_SUCH_DOC == s )
    {
        return APR_SUCCESS;
    }

    LCNCR( lcn_filtered_scorer_score_max( scorer, hc, INT_MAX_VALUE, &dummy ) );

    return s;
}

static apr_status_t
lcn_filtered_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;
    unsigned int i;
    unsigned int bv_size = lcn_bitvector_size( p->bv );

    for( i = p->act_doc + 1; i < bv_size; i++ )
    {
        if ( lcn_bitvector_get_bit( p->bv, i ) )
        {
            p->act_doc = i;
            return s;
        }
    }

    if ( i >= bv_size )
    {
        p->act_doc = INT_MAX_VALUE;
        s = LCN_ERR_NO_SUCH_DOC;
    }

    return s;
}

static apr_status_t
lcn_filtered_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    score->float_val = 1.0;
    return APR_SUCCESS;
}

static unsigned int
lcn_filtered_scorer_doc( lcn_scorer_t* scorer )
{
    return scorer->priv->act_doc;
}

static apr_status_t
lcn_filtered_scorer_skip_to( lcn_scorer_t* scorer,
                             unsigned int target )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;
    unsigned int i;
    unsigned int bv_size = lcn_bitvector_size( p->bv );

    do
    {
        p->act_doc = target;

        for( i = p->act_doc; i < bv_size; i++ )
        {
            if ( lcn_bitvector_get_bit( p->bv, i ) )
            {
                p->act_doc = i;
                return s;
            }
        }

        if ( i >= bv_size )
        {
            p->act_doc = INT_MAX_VALUE;
            s = LCN_ERR_NO_SUCH_DOC;
        }
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_filtered_scorer_create( lcn_scorer_t** scorer,
                            lcn_weight_t* weight,
                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, NULL, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ), APR_ENOMEM );

        p->bv           = weight->query->bv;
        p->weight       = weight;
        p->weight_value = lcn_weight_value( weight );

        LCNCE( lcn_float_array_create( &(p->score_cache ), SCORE_CACHE_SIZE, pool ) );

        (*scorer)->doc          = lcn_filtered_scorer_doc;

        (*scorer)->score_max    = lcn_filtered_scorer_score_max;
        (*scorer)->next         = lcn_filtered_scorer_next;
        (*scorer)->score        = lcn_filtered_scorer_score;
        (*scorer)->score_get    = lcn_filtered_scorer_score_get;

        (*scorer)->type    = "term_scorer";
        (*scorer)->skip_to = lcn_filtered_scorer_skip_to;

        (*scorer)->priv         = p;
    }
    while( FALSE );

    return s;
}


/* END OF FILTERED SCORER IMPLEMENTATION */





















static float
lcn_filtered_weight_sum_of_squared_weights( lcn_weight_t* w )
{
    w->query_weight = w->idf * lcn_query_boost( w->query );
    return ( w->query_weight * w->query_weight );
}

static void
lcn_term_weight_normalize( lcn_weight_t* w, float n )
{
    w->query_norm = n;
    w->query_weight *= n;
    w->value = w->query_weight * w->idf;
}

static float
lcn_term_weight_value_get( lcn_weight_t* w )
{
    return w->value;
}

static apr_status_t
lcn_filtered_weight_scorer( lcn_weight_t* w,
                            lcn_scorer_t** scorer,
                            lcn_index_reader_t* r,
                            apr_pool_t* pool )
{
    return lcn_filtered_scorer_create( scorer, w, pool );
}

apr_status_t
lcn_filtered_weight_create( lcn_weight_t** weight,
                            lcn_searcher_t* searcher,
                            lcn_query_t* filtered_query,
                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *weight = apr_pcalloc( pool, sizeof( lcn_weight_t ) ), APR_ENOMEM );

        (*weight)->idf = 1.0;
        (*weight)->sum_of_squared_weights = lcn_filtered_weight_sum_of_squared_weights;

        (*weight)->scorer    = lcn_filtered_weight_scorer;
        (*weight)->normalize = lcn_term_weight_normalize;
        (*weight)->value_get = lcn_term_weight_value_get;
        (*weight)->query     = filtered_query;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_filtered_query_to_string( lcn_query_t* query,
                              char** result,
                              const char* field,
                              apr_pool_t* pool )
{
    *result = apr_pstrdup( pool, "<filter>" );
    return APR_SUCCESS;
}

apr_status_t
lcn_filtered_query_create_weight( lcn_query_t* query,
                                  lcn_weight_t** weight,
                                  lcn_searcher_t* searcher,
                                  apr_pool_t* pool )
{
    return lcn_filtered_weight_create( weight, searcher, query, pool );
}

apr_status_t
lcn_filtered_query_clone( lcn_query_t* query,
                          lcn_query_t** clone,
                          apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_filtered_query_create( clone, query->bv, pool ));
    (*clone)->boost = query->boost;
    return s;
}


static apr_status_t
lcn_filtered_query_extract_terms( lcn_query_t* query,
                                  lcn_list_t* terms )
{
    apr_status_t s = APR_SUCCESS;
    LCNCR( lcn_list_create( &terms, 10, query->pool ));
    return s;
}

apr_status_t
lcn_filtered_query_create( lcn_query_t** query,
                           lcn_bitvector_t *bv,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_query_create( query, pool ) );

        (*query)->bv = bv;
        (*query)->type = LCN_QUERY_TYPE_FILTERED;

        (*query)->extract_terms = lcn_filtered_query_extract_terms;
        (*query)->create_weight = lcn_filtered_query_create_weight;
        (*query)->clone         = lcn_filtered_query_clone;
        (*query)->to_string     = lcn_filtered_query_to_string;

        (*query)->boost         = 1.0f;
        (*query)->pool          = pool;
    }
    while( FALSE );

    return s;
}

