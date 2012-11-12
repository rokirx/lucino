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
    unsigned int act_doc;
    unsigned int act_freq;
    unsigned int act_pos;

    lcn_int_array_t* docs;
    lcn_int_array_t* freqs;

    unsigned int pointer;
    unsigned int pointer_max;

    lcn_float_array_t* score_cache;

    unsigned int pos;
};



static apr_status_t
lcn_term_scorer_score_max( lcn_scorer_t* scorer,
                           lcn_hit_collector_t* hc,
                           unsigned int end,
                           lcn_bool_t* more_possible )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        lcn_similarity_t* sim = lcn_scorer_similarity( scorer );
        float* norm_decoder = lcn_similarity_norm_decoder( sim );

        while( p->act_doc < end )
        {
            lcn_score_t score = {0};
            unsigned int f =  p->freqs->arr[ p->pointer ];

            score.float_val = f < SCORE_CACHE_SIZE
                ?  p->score_cache->arr[ f ]
                : lcn_similarity_tf( sim, f ) * p->weight_value;

            score.float_val *= (p->norms == NULL ? 1 : norm_decoder[ p->norms->arr[p->act_doc] & 0xFF ]);

            lcn_hit_collector_collect( hc, p->act_doc, score );

            if( ++( p->pointer ) >= p->pointer_max )
            {
                LCNCE( lcn_term_docs_read( p->term_docs,
                                           p->docs,
                                           p->freqs,
                                           &( p->pointer_max ) ) );
                if( p->pointer_max != 0 )
                {
                    p->pointer = 0;
                }
                else
                {
                    LCNCE( lcn_term_docs_close( p->term_docs ) );
                    p->act_doc = INT_MAX_VALUE;
                    *more_possible = FALSE;
                    return s;
                }
            }

            p->act_doc =  p->docs->arr[ p->pointer ];
        }

        *more_possible = TRUE;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_term_scorer_score( lcn_scorer_t* scorer, lcn_hit_collector_t* hc )
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

    LCNCR( lcn_term_scorer_score_max( scorer, hc, INT_MAX_VALUE, &dummy ) );

    return s;
}

static apr_status_t
lcn_term_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    p->pointer++;

    if( p->pointer >= p->pointer_max )
    {
        LCNCR( lcn_term_docs_read( p->term_docs,
                                   p->docs,
                                   p->freqs,
                                   &(p->pointer_max ) ) );

        if( p->pointer_max != 0 )
        {
            p->pointer = 0;
        }
        else
        {
            LCNCR( lcn_term_docs_close( p->term_docs ) );
            p->act_doc = INT_MAX_VALUE;
            s = LCN_ERR_NO_SUCH_DOC;
        }
    }

    p->act_doc = p->docs->arr[p->pointer];

    return s;
}

static apr_status_t
lcn_term_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    lcn_scorer_private_t* p = scorer->priv;
    unsigned int f = p->freqs->arr[p->pointer];
    float raw = f < SCORE_CACHE_SIZE
        ? p->score_cache->arr[f]
        : lcn_similarity_tf( scorer->similarity, f ) * p->weight_value;

    score->float_val = raw * ( p->norms == NULL
                               ? 1
                               : lcn_similarity_decode_norm( scorer->similarity,
                                                             p->norms->arr[p->act_doc] ) );

    return APR_SUCCESS;
}

static unsigned int
lcn_term_scorer_doc( lcn_scorer_t* scorer )
{
    return scorer->priv->act_doc;
}

static apr_status_t
lcn_term_scorer_skip_to( lcn_scorer_t* scorer,
                         unsigned int target )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        for( p->pointer++; p->pointer < p->pointer_max; p->pointer++ )
        {
            if( p->docs->arr[p->pointer] >= target )
            {
                p->act_doc = p->docs->arr[p->pointer];

                return APR_SUCCESS;
            }
        }

        if( APR_SUCCESS ==
            ( s = lcn_term_docs_skip_to( p->term_docs, target ) ) )
        {
            p->pointer_max = 1;
            p->pointer     = 0;

            p->docs->arr[p->pointer] =  lcn_term_docs_doc( p->term_docs );
            p->act_doc = p->docs->arr[p->pointer];

            p->freqs->arr[p->pointer] =lcn_term_docs_freq( p->term_docs );
        }
        else if( LCN_ERR_NO_SUCH_DOC == s)
        {
            s = LCN_ERR_NO_SUCH_DOC;
            p->act_doc = INT_MAX_VALUE;
        }
        else
        {
            LCNCE( s );
        }
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_term_scorer_create( lcn_scorer_t** scorer,
                        lcn_weight_t* weight,
                        lcn_term_docs_t* term_docs,
                        lcn_similarity_t* sim,
                        lcn_byte_array_t* norms,
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
        p->weight_value = lcn_weight_value( weight );

        LCNCE( lcn_int_array_create( &( p->docs ),         SCORE_CACHE_SIZE, pool ) );
        LCNCE( lcn_int_array_create( &( p->freqs ),        SCORE_CACHE_SIZE, pool ) );
        LCNCE( lcn_float_array_create( &(p->score_cache ), SCORE_CACHE_SIZE, pool ) );

        for( i = 0; i < SCORE_CACHE_SIZE; i++ )
        {
            p->score_cache->arr[i] = lcn_similarity_tf( sim, i ) * p->weight_value;
        }

        (*scorer)->doc          = lcn_term_scorer_doc;

        (*scorer)->score_max    = lcn_term_scorer_score_max;
        (*scorer)->next         = lcn_term_scorer_next;
        (*scorer)->score        = lcn_term_scorer_score;
        (*scorer)->score_get    = lcn_term_scorer_score_get;

        (*scorer)->type    = "term_scorer";
        (*scorer)->skip_to = lcn_term_scorer_skip_to;

        (*scorer)->priv         = p;
    }
    while( FALSE );

    return s;
}

