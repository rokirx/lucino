#include "scorer.h"
#include "lcn_index.h"
#include "lcn_util.h"
#include "hit_collector.h"

#define SCORE_CACHE_SIZE (32)
#define INT_MAX_VALUE    (2147483647)

struct lcn_scorer_private_t
{
    lcn_weight_t* weight;
    lcn_term_docs_t* term_docs;
    lcn_byte_array_t* norms;
    float weight_value;
    apr_ssize_t act_doc;
    apr_ssize_t act_freq;
    apr_ssize_t act_pos;

    lcn_int_array_t* docs;
    lcn_int_array_t* freqs;

    unsigned int pointer;
    unsigned int pointer_max;

    lcn_float_array_t* score_cache;

    unsigned int pos;
};

static apr_status_t
lcn_term_pos_scorer_check_doc( lcn_scorer_t* scorer, lcn_bool_t* matches )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        unsigned int i;
        *matches = LCN_FALSE;

        for( i = 0; i < lcn_term_docs_freq( p->term_docs ); i++ )
        {
            apr_ssize_t pos;

            LCNCE( lcn_term_positions_next_position( p->term_docs, &pos ) );

            if( pos == p->pos )
            {
                *matches = LCN_TRUE;
            }
        }
    }
    while( 0 );

    return s;
}

static apr_status_t
lcn_term_pos_scorer_score_max( lcn_scorer_t* scorer,
                               lcn_hit_collector_t* hc,
                               unsigned int end,
                               lcn_bool_t* more_possible )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        lcn_similarity_t *sim = lcn_scorer_similarity( scorer );
        float *norm_decoder = lcn_similarity_norm_decoder( sim );

        while( APR_SUCCESS == ( s = lcn_term_docs_next( p->term_docs ) ) &&
               ( p->act_doc < end  ))
        {
            lcn_bool_t position_matches = LCN_FALSE;
            lcn_score_t score = {0};
            unsigned int f =  lcn_term_docs_freq( p->term_docs );
            p->act_doc = lcn_term_docs_doc( p->term_docs );

            score.float_val = f < SCORE_CACHE_SIZE
                ?  p->score_cache->arr[ f ]
                : lcn_similarity_tf( sim, f ) * p->weight_value;

            score.float_val *= (p->norms == NULL ? 1 : norm_decoder[ p->norms->arr[p->act_doc] & 0xFF ]);

            LCNCE( lcn_term_pos_scorer_check_doc( scorer, &(position_matches) ) );

            if( position_matches )
            {
                lcn_hit_collector_collect( hc, p->act_doc, score );
            }

            *more_possible = TRUE;
        }

        if( LCN_ERR_ITERATOR_NO_NEXT == s )
        {
            s = APR_SUCCESS;
            *more_possible = LCN_FALSE;
        }
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_term_pos_scorer_score( lcn_scorer_t* scorer, lcn_hit_collector_t* hc )
{
    lcn_bool_t dummy;
    return lcn_term_pos_scorer_score_max( scorer, hc, INT_MAX_VALUE, &dummy );
}

static apr_status_t
lcn_term_pos_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s = APR_SUCCESS;
    lcn_bool_t pos_match = LCN_FALSE;

    while( !pos_match &&
           ( APR_SUCCESS ==
             ( s = lcn_term_docs_next( scorer->priv->term_docs ) ) ) )
    {
        LCNCE( lcn_term_pos_scorer_check_doc( scorer, &pos_match ) );
    }

    if( APR_SUCCESS == s )
    {
        scorer->priv->act_doc = lcn_term_docs_doc( scorer->priv->term_docs );
    }


    return s;
}

static apr_status_t
lcn_term_pos_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    unsigned int f;
    float raw;
    lcn_scorer_private_t* p = scorer->priv;
    unsigned int cur_doc;

    cur_doc = lcn_term_docs_doc( p->term_docs );
    f = lcn_term_docs_freq( p->term_docs );

    raw =
        f < SCORE_CACHE_SIZE
        ? p->score_cache->arr[f]
        : lcn_similarity_tf( scorer->similarity, f ) * p->weight_value;

    score->float_val = raw * ( p->norms == NULL
                               ? 1
                               : lcn_similarity_decode_norm( scorer->similarity,
                                                             p->norms->arr[ cur_doc ] ) );
    return APR_SUCCESS;
}

static unsigned int
lcn_term_pos_scorer_doc( lcn_scorer_t* scorer )
{
    return scorer->priv->act_doc;
}

static apr_status_t
lcn_term_pos_scorer_skip_to( lcn_scorer_t* scorer,
                             unsigned int target )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        lcn_bool_t pos_match = LCN_FALSE;

        s = lcn_term_docs_skip_to( p->term_docs, target );

        if( s )
        {
            return s;
        }

        do
        {
            LCNCE( lcn_term_pos_scorer_check_doc( scorer, &pos_match ) );
        }
        while( !pos_match &&
               APR_SUCCESS ==
               ( s = lcn_term_docs_next( scorer->priv->term_docs ) ) );

        if( APR_SUCCESS == s )
        {
            p->act_doc = lcn_term_docs_doc( p->term_docs );
        }

        s = ( LCN_ERR_ITERATOR_NO_NEXT == s ) ? LCN_ERR_NO_SUCH_DOC : s;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_term_pos_scorer_create( lcn_scorer_t** scorer,
                            lcn_weight_t* weight,
                            lcn_term_docs_t* term_docs,
                            lcn_similarity_t* sim,
                            lcn_byte_array_t* norms,
                            unsigned int pos,
                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;
        unsigned int i;

        LCNCE( lcn_default_scorer_create( scorer, sim, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ), APR_ENOMEM );

        p->weight       = weight;
        p->term_docs    = term_docs;
        p->norms        = norms;
        p->pos          = pos;
        p->weight_value = lcn_weight_value( weight );

        LCNCE( lcn_int_array_create( &( p->docs ),         SCORE_CACHE_SIZE, pool ) );
        LCNCE( lcn_int_array_create( &( p->freqs ),        SCORE_CACHE_SIZE, pool ) );
        LCNCE( lcn_float_array_create( &(p->score_cache ), SCORE_CACHE_SIZE, pool ) );

        for( i = 0; i < SCORE_CACHE_SIZE; i++ )
        {
            p->score_cache->arr[i] = lcn_similarity_tf( sim, i ) * p->weight_value;
        }

        (*scorer)->doc          = lcn_term_pos_scorer_doc;

        (*scorer)->score_max    = lcn_term_pos_scorer_score_max;
        (*scorer)->next         = lcn_term_pos_scorer_next;
        (*scorer)->score        = lcn_term_pos_scorer_score;
        (*scorer)->score_get    = lcn_term_pos_scorer_score_get;

        (*scorer)->type    = "term_pos_scorer";
        (*scorer)->skip_to = lcn_term_pos_scorer_skip_to;

        (*scorer)->priv         = p;
    }
    while( FALSE );

    return s;
}

