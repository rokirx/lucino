#include "phrase_scorer.h"
#include "phrase_positions.h"
#include "phrase_queue.h"
#include "similarity.h"

struct lcn_scorer_private_t
{
    lcn_weight_t* weight;
    lcn_byte_array_t* norms;
    float value;

    apr_status_t (*phrase_freq)( lcn_scorer_t*, float* );

    lcn_bool_t first_time;
    lcn_bool_t more;
    lcn_priority_queue_t* pq;
    lcn_phrase_positions_t* first, *last;
    lcn_phrase_positions_t* min, *max;


    /* non repeating pps ordered by their query offset */
    lcn_phrase_positions_t** non_rep_pps;
    unsigned int nrp_len;

    lcn_bool_t checked_repeats;
    lcn_bool_t has_repeats;

    float freq;

    unsigned int slop;
};

void
lcn_phrase_scorer_first_to_last( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;

    p->last->next = p->first;
    p->last = p->first;
    p->first = p->first->next;
    p->last->next = NULL;
}

void
lcn_phrase_scorer_pq_to_list( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;

    p->last = p->first = NULL;

    while( lcn_priority_queue_top( p->pq ) != NULL )
    {
        lcn_phrase_positions_t* pp = lcn_priority_queue_pop( p->pq );

        if( p->last != NULL )
        {
            p->last->next = pp;
        }
        else
        {
            p->first = pp;
        }

        p->last  = pp;
        pp->next = NULL;
    }
}

void
lcn_phrase_scorer_sort( lcn_scorer_t* scorer )
{
    lcn_phrase_positions_t* pp;
    lcn_scorer_private_t* p = scorer->priv;

    lcn_priority_queue_clear( p->pq );

    for( pp = p->first; NULL != pp; pp = pp->next )
    {
        lcn_priority_queue_put( p->pq, pp );
    }

    lcn_phrase_scorer_pq_to_list( scorer );
}

apr_status_t
lcn_phrase_scorer_init( lcn_scorer_t* scorer )
{
    lcn_phrase_positions_t* pp;
    lcn_scorer_private_t* p = scorer->priv;

    for( pp = p->first; p->more && pp != NULL; pp = pp->next )
    {

        p->more = ( APR_SUCCESS == ( lcn_phrase_positions_next( pp ) ) );
    }
    if( p->more )
    {
        lcn_phrase_scorer_sort( scorer );
    }


    return APR_SUCCESS;
}


static unsigned int
lcn_phrase_scorer_doc( lcn_scorer_t* scorer )
{
    return scorer->priv->first->doc;
}

int counter;

static apr_status_t
lcn_ordered_phrase_scorer_do_next( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;

    while( p->more )
    {
        while( p->more && ( p->first->doc < p->last->doc ) )
        {
            p->more = ( APR_SUCCESS ==
                        lcn_phrase_positions_skip_to( p->first, p->last->doc ) );

            lcn_phrase_scorer_first_to_last( scorer );
        }

        if( p->more )
        {
            apr_status_t s;

            LCNCR( p->phrase_freq( scorer, &(p->freq) ) );

            if( p->freq == 0.0f )
            {
                p->more = ( APR_SUCCESS ==
                            lcn_phrase_positions_next( p->last ));
            }
            else
            {
                return APR_SUCCESS;
            }
        }
    }
    return LCN_ERR_NO_SUCH_DOC;
}

static apr_status_t
lcn_phrase_scorer_do_next( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;

    while( p->more )
    {
        while( p->more && ( p->first->doc < p->last->doc ) )
        {
            p->more = ( APR_SUCCESS == lcn_phrase_positions_skip_to( p->first, p->last->doc ) );
            lcn_phrase_scorer_first_to_last( scorer );
        }

        if( p->more )
        {
            /* ignore return value */
            (void) p->phrase_freq( scorer, &(p->freq) );

            if( p->freq == 0.0f )
            {
                p->more = ( APR_SUCCESS == lcn_phrase_positions_next( p->last ));
            }
            else
            {
                return APR_SUCCESS;
            }
        }
    }

    return LCN_ERR_NO_SUCH_DOC;
}

static apr_status_t
lcn_phrase_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;
    if( p->first_time )
    {
        LCNCR( lcn_phrase_scorer_init( scorer ) );

        p->first_time = LCN_FALSE;

    }
    else if( p->more )
    {
        p->more = ( APR_SUCCESS ==
                    ( s = lcn_phrase_positions_next( p->last ) ) );
    }

    s = lcn_phrase_scorer_do_next( scorer );

    return s;
}


static apr_status_t
lcn_phrase_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    lcn_scorer_private_t* p = scorer->priv;

    float raw = lcn_similarity_tf( scorer->similarity, p->freq ) * p->value;

    score->float_val = raw * (p->norms == NULL ? 1.0f :
                              lcn_similarity_decode_norm( scorer->similarity,  p->norms->arr[p->first->doc] ));

    return APR_SUCCESS;
}

static apr_status_t
lcn_phrase_scorer_skip_to( lcn_scorer_t* scorer, unsigned int target )
{
    lcn_phrase_positions_t* pp;
    lcn_scorer_private_t* p = scorer->priv;


    /* http://svn.apache.org/viewvc/lucene/java/trunk/src/java/org/apache/lucene/search/PhraseScorer.java?diff_format=h&revision=531733&view=markup&pathrev=531733 line 111 */
    p->first_time = LCN_FALSE;

    for( pp = p->first; p->more && ( pp != NULL ); pp = pp->next )
    {
        p->more = ( APR_SUCCESS == lcn_phrase_positions_skip_to( pp, target ) );
    }

    if( p->more )
    {
        lcn_phrase_scorer_sort( scorer );
    }

    return lcn_phrase_scorer_do_next( scorer );
}

static apr_status_t
lcn_ordered_phrase_scorer_skip_to( lcn_scorer_t* scorer, unsigned int target )
{
    lcn_phrase_positions_t* pp;
    lcn_scorer_private_t* p = scorer->priv;

    for( pp = p->first; p->more && ( pp != NULL ); pp = pp->next )
    {
        p->more = ( APR_SUCCESS == lcn_phrase_positions_skip_to( pp, target ) );
    }

    if( p->more )
    {
        lcn_phrase_scorer_sort( scorer );
    }

    return lcn_ordered_phrase_scorer_do_next( scorer );
}


static apr_status_t
lcn_ordered_phrase_scorer_create( lcn_scorer_t** scorer,
                                  lcn_weight_t* weight,
                                  lcn_ptr_array_t* term_positions,
                                  lcn_size_array_t* positions,
                                  lcn_similarity_t* similarity,
                                  lcn_byte_array_t* norms,
                                  apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, similarity, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ), APR_ENOMEM );

        p->norms      = norms;
        p->weight     = weight;
        p->value      = lcn_weight_value( weight );

        p->first_time = LCN_TRUE;
        p->more       = LCN_TRUE;

        for( i = 0; i < term_positions->length; i++ )
        {
            lcn_phrase_positions_t* pp;

            LCNCE( lcn_phrase_positions_create( &pp,
                                                term_positions->arr[i],
                                                positions->arr[i],
                                                pool ) );
            if( NULL != p->last )
            {
                p->last->next = pp;
            }
            else
            {
                p->first = pp;
            }

            p->last = pp;
        }

        LCNCE( lcn_ordered_phrase_queue_create( &( p->pq ), term_positions->length, pool ) );

        (*scorer)->priv      = p;
        (*scorer)->doc       = lcn_phrase_scorer_doc;
        (*scorer)->next      = lcn_phrase_scorer_next;
        (*scorer)->skip_to   = lcn_ordered_phrase_scorer_skip_to;
        (*scorer)->score_get = lcn_phrase_scorer_score_get;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_phrase_scorer_create( lcn_scorer_t** scorer,
                          lcn_weight_t* weight,
                          lcn_ptr_array_t* term_positions,
                          lcn_size_array_t* positions,
                          lcn_similarity_t* similarity,
                          lcn_byte_array_t* norms,
                          apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, similarity, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        p->norms      = norms;
        p->weight     = weight;
        p->value      = lcn_weight_value( weight );

        p->first_time = LCN_TRUE;
        p->more       = LCN_TRUE;

        for( i = 0; i < term_positions->length; i++ )
        {
            lcn_phrase_positions_t* pp;

            LCNCE( lcn_phrase_positions_create( &pp,
                                                term_positions->arr[i],
                                                positions->arr[i],
                                                pool ) );
            if( NULL != p->last )
            {
                p->last->next = pp;
            }
            else
            {
                p->first = pp;
            }

            p->last = pp;
        }

        LCNCE( lcn_phrase_queue_create( &( p->pq ),
                                        term_positions->length,
                                        pool ) );

        (*scorer)->priv      = p;
        (*scorer)->doc       = lcn_phrase_scorer_doc;
        (*scorer)->next      = lcn_phrase_scorer_next;
        (*scorer)->skip_to   = lcn_phrase_scorer_skip_to;
        (*scorer)->score_get = lcn_phrase_scorer_score_get;
    }
    while( FALSE );

    return s;
}

#if 0
static void
print_positions( lcn_scorer_t *scorer, char *title )
{
    lcn_scorer_private_t* p = scorer->priv;
    int k = 0;

    if ( p->non_rep_pps != NULL )
    {
        for( k = 0; k < p->nrp_len; k++ )
        {
            //lcn_phrase_positions_t *pp = p->non_rep_pps[ k ];
        }
    } else {
//      for (PhrasePositions pp=min; 0==k || pp!=min; pp = pp.next) {
//        ps.println("  " + k++ + "  " + pp);
//    }
    }
}
#endif

#if 0
static unsigned int
init_phrase_positions( lcn_scorer_t *scorer )
{
    lcn_scorer_private_t* p = scorer->priv;
    unsigned int end = 0;

    /* no repeats at all (most common case is also the simplest one) */

    if ( p->checked_repeats && ! p->has_repeats )
    {
        flog( stderr, "TODO: checked repeats = true\n" );
    }

    print_positions( scorer, "Init 1: Bef position" );

    return end;
}
#endif

static apr_status_t
lcn_ordered_sps_phrase_freq( lcn_scorer_t* scorer, float* result )
{
    unsigned int end = 0; //init_phrase_positions( scorer );

    float freq = 0.0f;
    lcn_bool_t done = FALSE;
    lcn_phrase_positions_t* pp;
    lcn_scorer_private_t* p = scorer->priv;

    lcn_priority_queue_clear( p->pq );

    for( pp = p->first; pp != NULL; pp = pp->next )
    {
        lcn_phrase_positions_first_position( pp );

        if( pp->position > end )
        {
            end = pp->position;
        }

        lcn_priority_queue_put( p->pq, pp );
    }

    do
    {
        apr_status_t s;
        unsigned int pos, match_length;
        lcn_phrase_positions_t* pp = lcn_priority_queue_pop( p->pq );
        unsigned int start = pp->position;
        lcn_phrase_positions_t* top = (lcn_phrase_positions_t*) lcn_priority_queue_top( p->pq );
        unsigned int next  = top->position;

        while( next < start )
        {
            if ( APR_SUCCESS == (s = lcn_phrase_positions_next_position( pp )))
            {
                next = pp->position;

                if ( next > end )
                {
                    end = next;
                }
            }
            else
            {
                done = TRUE;
                break;
            }
        }

        if ( done )
        {
            break;
        }

        for( pos = start; pos <= next; pos = pp->position )
        {
            start = pos;

            if( APR_SUCCESS != ( s = lcn_phrase_positions_next_position( pp ) ) )
            {
                if( s != LCN_ERR_NO_SUCH_DOC )
                {
                    LCNCR( s );
                }

                done = TRUE;
                break;
            }
        }

        /* end:   last position of a phrase term  */
        /* start: first position of a phrase term */

        match_length = end - start;

        if( match_length <= p->slop )
        {
            freq += scorer->similarity->sloppy_freq( match_length );
        }

        if( pp->position > end )
        {
            end = pp->position;
        }

        lcn_priority_queue_put( p->pq, pp );
    }
    while( !done );

    *result = freq;

    return APR_SUCCESS;
}

static apr_status_t
lcn_sps_phrase_freq( lcn_scorer_t* scorer, float* result )
{
    unsigned int end = 0; //init_phrase_positions( scorer );

    float freq = 0.0f;
    lcn_bool_t done = FALSE;
    lcn_phrase_positions_t* pp;
    lcn_scorer_private_t* p = scorer->priv;

    lcn_priority_queue_clear( p->pq );

    for( pp = p->first; pp != NULL; pp = pp->next )
    {
        lcn_phrase_positions_first_position( pp );

        if( pp->position > end )
        {
            end = pp->position;
        }

        lcn_priority_queue_put( p->pq, pp );
    }

    do
    {
        apr_status_t s;
        unsigned int pos, match_length;
        lcn_phrase_positions_t* pp = lcn_priority_queue_pop( p->pq );
        unsigned int start = pp->position;
        lcn_phrase_positions_t* top = (lcn_phrase_positions_t*) lcn_priority_queue_top( p->pq );
        unsigned int next  = top->position;

        for( pos = start; pos <= next; pos = pp->position )
        {
            start = pos;

            if( APR_SUCCESS != ( s = lcn_phrase_positions_next_position( pp ) ) )
            {
                if( s != LCN_ERR_NO_SUCH_DOC )
                {
                    LCNCR( s );
                }

                done = TRUE;
                break;
            }
        }

        /* end:   last position of a phrase term  */
        /* start: first position of a phrase term */

        match_length = end - start;

        if( match_length <= p->slop )
        {
            freq += scorer->similarity->sloppy_freq( match_length );
        }

        if( pp->position > end )
        {
            end = pp->position;
        }

        lcn_priority_queue_put( p->pq, pp );
    }
    while( !done );

    *result = freq;

    return APR_SUCCESS;
}


apr_status_t
lcn_sloppy_phrase_scorer_create( lcn_scorer_t** scorer,
                                 lcn_weight_t* weight,
                                 lcn_ptr_array_t* term_positions,
                                 lcn_size_array_t* positions,
                                 lcn_similarity_t* similarity,
                                 unsigned int slop,
                                 lcn_byte_array_t* norms,
                                 apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_phrase_scorer_create( scorer,
                                         weight,
                                         term_positions,
                                         positions,
                                         similarity,
                                         norms,
                                         pool ) );

        (*scorer)->type = "sloppy_phrase_scorer";
        (*scorer)->priv->phrase_freq = lcn_sps_phrase_freq;
        (*scorer)->priv->slop = slop;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_sloppy_ordered_phrase_scorer_create( lcn_scorer_t** scorer,
                                         lcn_weight_t* weight,
                                         lcn_ptr_array_t* term_positions,
                                         lcn_size_array_t* positions,
                                         lcn_similarity_t* similarity,
                                         unsigned int slop,
                                         lcn_byte_array_t* norms,
                                         apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_ordered_phrase_scorer_create( scorer,
                                                 weight,
                                                 term_positions,
                                                 positions,
                                                 similarity,
                                                 norms,
                                                 pool ) );

        (*scorer)->type = "sloppy_ordered_phrase_scorer";
        (*scorer)->priv->phrase_freq = lcn_ordered_sps_phrase_freq;
        (*scorer)->priv->slop = slop;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_eps_phrase_freq( lcn_scorer_t* scorer, float* result )
{
    apr_status_t s = APR_SUCCESS;
    lcn_phrase_positions_t* pp;
    unsigned int freq = 0;

    lcn_scorer_private_t* p = scorer->priv;

    /* http://svn.apache.org/viewvc/lucene/java/trunk/src/java/org/apache/lucene/search/ExactPhraseScorer.java?view=markup&pathrev=531733 line 32 */

    lcn_priority_queue_clear( p->pq ) ;

    for( pp = p->first; pp != NULL; pp = pp->next )
    {
        s = lcn_phrase_positions_first_position( pp );

        if ( LCN_ERR_NO_SUCH_DOC == s )
        {
            *result = (float)freq;
            return APR_SUCCESS;
        }

        /* throws exception in origin Lucene, so we do */

        if ( APR_SUCCESS != ( s = lcn_priority_queue_put( p->pq, pp ) ) )
        {
            return s;
        }
    }


    lcn_phrase_scorer_pq_to_list( scorer );

    do
    {
        while( p->first->position < p->last->position )
        {
            do
            {
                if( APR_SUCCESS != ( s = lcn_phrase_positions_next_position( p->first ) ) )
                {
                    if( LCN_ERR_NO_SUCH_DOC == s )
                    {
                        *result = (float)freq;
                        return APR_SUCCESS;
                    }
                }
            }
            while( p->first->position < p->last->position );

            lcn_phrase_scorer_first_to_last( scorer );
        }
        freq++;
    }
    while( APR_SUCCESS == ( s = lcn_phrase_positions_next_position( p->last ) ) );

    while( APR_SUCCESS == ( s = lcn_phrase_positions_next_position( p->last ) ) );

    *result = (float)freq;

    return APR_SUCCESS;
}

apr_status_t
lcn_exact_phrase_scorer_create( lcn_scorer_t** scorer,
                                lcn_weight_t* weight,
                                lcn_ptr_array_t* term_positions,
                                lcn_size_array_t* positions,
                                lcn_similarity_t* similarity,
                                lcn_byte_array_t* norms,
                                apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_phrase_scorer_create( scorer,
                                         weight,
                                         term_positions,
                                         positions,
                                         similarity,
                                         norms,
                                         pool ) );

        (*scorer)->priv->checked_repeats = LCN_FALSE;
        (*scorer)->priv->has_repeats     = LCN_FALSE;
        (*scorer)->priv->non_rep_pps     = NULL;
        (*scorer)->priv->nrp_len         = 0;
        (*scorer)->priv->phrase_freq = lcn_eps_phrase_freq;

        (*scorer)->type = "exact_phrase_scorer";
    }
    while( FALSE );

    return s;
}
