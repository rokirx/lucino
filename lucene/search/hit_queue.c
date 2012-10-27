#include "top_docs.h"
#include "priority_queue.h"
#include "lcn_bitvector.h"

BEGIN_C_DECLS

lcn_bool_t
lcn_hit_queue_score_docs_compare( lcn_priority_queue_t *hit_queue,
                                  lcn_score_doc_t* a,
                                  lcn_score_doc_t* b )
{
    return hit_queue->less_than( hit_queue, a, b );
}

static lcn_bool_t
lcn_hit_queue_relevance_less_than( lcn_priority_queue_t* pq, void* a, void* b )
{
    lcn_score_doc_t *score_a = (lcn_score_doc_t*)a;
    lcn_score_doc_t *score_b = (lcn_score_doc_t*)b;

    if( score_a->score.float_val == score_b->score.float_val )
    {
        return score_a->doc > score_b->doc;
    }

    return score_a->score.float_val < score_b->score.float_val;
}

static lcn_bool_t
lcn_hit_queue_relevance_less_than_ordered( lcn_priority_queue_t* pq, void* a, void* b )
{
    lcn_score_doc_t* score_a, *score_b;

    score_a = (lcn_score_doc_t*)a;
    score_b = (lcn_score_doc_t*)b;

    if ( score_a->score.int_val == score_b->score.int_val )
    {
        if( score_a->score.float_val == score_b->score.float_val )
        {
            return score_a->doc > score_b->doc;
        }

        return score_a->score.float_val < score_b->score.float_val;
    }

    return score_a->score.int_val > score_b->score.int_val;
}

static lcn_bool_t
lcn_hit_queue_relevance_filtered_less_than( lcn_priority_queue_t* pq, void* a, void* b )
{
    lcn_score_doc_t* score_a, *score_b;
    unsigned int i;
    lcn_bool_t bit_a, bit_b;

    score_a = (lcn_score_doc_t*) a;
    score_b = (lcn_score_doc_t*) b;

    for( i = 0; i < ((lcn_hit_queue_t*)pq)->sort_bitvectors_size; i++ )
    {
        lcn_bitvector_t *bitvector = ((lcn_hit_queue_t*)pq)->sort_bitvectors[i];

        bit_a = lcn_bitvector_get_bit( bitvector, score_a->doc );
        bit_b = lcn_bitvector_get_bit( bitvector, score_b->doc );

        if ( bit_a & bit_b )
        {
            if( score_a->score.float_val == score_b->score.float_val )
            {
                return score_a->doc > score_b->doc;
            }

            return score_a->score.float_val < score_b->score.float_val;
        }

        if ( bit_a )
        {
            return LCN_FALSE;
        }

        if ( bit_b )
        {
            return LCN_TRUE;
        }
    }

    if( score_a->score.float_val == score_b->score.float_val )
    {
        return score_a->doc > score_b->doc;
    }

    return score_a->score.float_val < score_b->score.float_val;
}

static lcn_bool_t
lcn_hit_queue_relevance_filtered_less_than_ordered( lcn_priority_queue_t* pq, void* a, void* b )
{
    lcn_score_doc_t* score_a, *score_b;
    unsigned int i;
    lcn_bool_t bit_a, bit_b;

    score_a = (lcn_score_doc_t*) a;
    score_b = (lcn_score_doc_t*) b;

    if ( score_a->score.int_val == score_b->score.int_val )
    {
        for( i = 0; i < ((lcn_hit_queue_t*)pq)->sort_bitvectors_size; i++ )
        {
            lcn_bitvector_t *bitvector = ((lcn_hit_queue_t*)pq)->sort_bitvectors[i];

            bit_a = lcn_bitvector_get_bit( bitvector, score_a->doc );
            bit_b = lcn_bitvector_get_bit( bitvector, score_b->doc );

            if ( bit_a & bit_b )
            {
                if ( score_a->score.int_val == score_b->score.int_val )
                {
                    if( score_a->score.float_val == score_b->score.float_val )
                    {
                        return score_a->doc > score_b->doc;
                    }

                    return score_a->score.float_val < score_b->score.float_val;
                }

                return score_a->score.int_val > score_b->score.int_val;
            }

            if ( bit_a )
            {
                return LCN_FALSE;
            }

            if ( bit_b )
            {
                return LCN_TRUE;
            }
        }

        if( score_a->score.float_val == score_b->score.float_val )
        {
            return score_a->doc > score_b->doc;
        }

        return score_a->score.float_val < score_b->score.float_val;
    }

    return score_a->score.int_val > score_b->score.int_val;
}

static lcn_bool_t
lcn_hit_queue_natural_filtered_less_than( lcn_priority_queue_t* pq, void* a, void* b )
{
    lcn_score_doc_t* score_a, *score_b;
    unsigned int i;
    lcn_bool_t bit_a, bit_b;

    score_a = (lcn_score_doc_t*) a;
    score_b = (lcn_score_doc_t*) b;

    for( i = 0; i < ((lcn_hit_queue_t*)pq)->sort_bitvectors_size; i++ )
    {
        lcn_bitvector_t *bitvector = ((lcn_hit_queue_t*)pq)->sort_bitvectors[i];

        bit_a = lcn_bitvector_get_bit( bitvector, score_a->doc );
        bit_b = lcn_bitvector_get_bit( bitvector, score_b->doc );

        if ( bit_a & bit_b )
        {
            return score_a->doc > score_b->doc;
        }

        if ( bit_a )
        {
            return LCN_FALSE;
        }

        if ( bit_b )
        {
            return LCN_TRUE;
        }
    }

    return score_a->doc > score_b->doc;
}

static lcn_bool_t
lcn_hit_queue_natural_filtered_less_than_ordered( lcn_priority_queue_t* pq, void* a, void* b )
{
    unsigned int i;
    lcn_bool_t bit_a, bit_b;

    lcn_score_doc_t* score_a = (lcn_score_doc_t*) a;
    lcn_score_doc_t* score_b = (lcn_score_doc_t*) b;

    if ( score_a->score.int_val == score_b->score.int_val )
    {
        for( i = 0; i < ((lcn_hit_queue_t*)pq)->sort_bitvectors_size; i++ )
        {
            lcn_bitvector_t *bitvector = ((lcn_hit_queue_t*)pq)->sort_bitvectors[i];

            bit_a = lcn_bitvector_get_bit( bitvector, score_a->doc );
            bit_b = lcn_bitvector_get_bit( bitvector, score_b->doc );

            if ( bit_a & bit_b )
            {
                return score_a->doc > score_b->doc;
            }

            if ( bit_a )
            {
                return LCN_FALSE;
            }

            if ( bit_b )
            {
                return LCN_TRUE;
            }
        }

        return score_a->doc > score_b->doc;
    }

    return score_a->score.int_val > score_b->score.int_val;
}

static lcn_bool_t
lcn_hit_queue_natural_less_than( lcn_priority_queue_t* pq, void* a, void* b )
{
    return ((lcn_score_doc_t*)a)->doc > ((lcn_score_doc_t*)b)->doc;
}

static lcn_bool_t
lcn_hit_queue_natural_less_than_ordered( lcn_priority_queue_t* pq, void* a, void* b )
{
    lcn_score_doc_t* score_a = (lcn_score_doc_t*)a;
    lcn_score_doc_t* score_b = (lcn_score_doc_t*)b;

    if ( score_a->score.int_val == score_b->score.int_val )
    {
        return score_a->doc > score_b->doc;
    }

    return score_a->score.int_val > score_b->score.int_val;
}

apr_status_t
lcn_hit_queue_create( lcn_hit_queue_t** hit_queue,
                      unsigned int capacity,
                      apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_priority_queue_t *pq;

        LCNPV( (*hit_queue) = apr_pcalloc( pool, sizeof( lcn_hit_queue_t ) ), APR_ENOMEM );

        pq = lcn_cast_priority_queue( *hit_queue );

        LCNCE( apr_pool_create( &(pq->pool), pool ) );
        pq->less_than = lcn_hit_queue_relevance_less_than;
        LCNCE( lcn_priority_queue_initialize( pq, capacity ) );
    }
    while( FALSE );

    return s;
}

static void
lcn_hit_queue_choose_sort_function( lcn_hit_queue_t *hit_queue,
                                    int order_by_flag,
                                    lcn_bool_t have_sort_bitvector,
                                    lcn_bool_t ordered_query_flag )
{
    lcn_priority_queue_t *pq = lcn_cast_priority_queue( hit_queue );

    if ( ! ordered_query_flag )
    {
        if ( ! have_sort_bitvector )
        {
            if ( LCN_ORDER_BY_NATURAL == order_by_flag )
            {
                pq->less_than = lcn_hit_queue_natural_less_than;
            }
            else
            {
                pq->less_than = lcn_hit_queue_relevance_less_than;
            }
        }
        else
        {
            if ( LCN_ORDER_BY_NATURAL == order_by_flag )
            {
                pq->less_than = lcn_hit_queue_natural_filtered_less_than;
            }
            else
            {
                pq->less_than = lcn_hit_queue_relevance_filtered_less_than;
            }
        }
    }
    else
    {
        if ( ! have_sort_bitvector )
        {
            if ( LCN_ORDER_BY_NATURAL == order_by_flag )
            {
                pq->less_than = lcn_hit_queue_natural_less_than_ordered;
            }
            else
            {
                pq->less_than = lcn_hit_queue_relevance_less_than_ordered;
            }
        }
        else
        {
            if ( LCN_ORDER_BY_NATURAL == order_by_flag )
            {
                pq->less_than = lcn_hit_queue_natural_filtered_less_than_ordered;
            }
            else
            {
                pq->less_than = lcn_hit_queue_relevance_filtered_less_than_ordered;
            }
        }
    }

    return;
}


apr_status_t
lcn_hit_queue_order_by( lcn_hit_queue_t *hit_queue,
                        int order_by_flag,
                        lcn_bool_t ordered_query_flag )
{
    lcn_hit_queue_choose_sort_function( hit_queue, order_by_flag, LCN_FALSE, ordered_query_flag );
    return APR_SUCCESS;
}

apr_status_t
lcn_hit_queue_order_by_bitvectors( lcn_hit_queue_t *hit_queue,
                                   lcn_list_t *bitvector_list )
{
    apr_status_t s;
    int order_by_flag = LCN_ORDER_BY_RELEVANCE;
    lcn_bool_t ordered_query_flag = LCN_FALSE;
    unsigned int size = lcn_list_size( bitvector_list );
    lcn_priority_queue_t *pq = lcn_cast_priority_queue( hit_queue );

    if ( size == 0 )
    {
        return APR_SUCCESS;
    }

    do
    {
        unsigned int i;

        LCNPV( hit_queue->sort_bitvectors = apr_palloc( pq->pool,
                                                        size * sizeof(lcn_bitvector_t*)),
               APR_ENOMEM );

        hit_queue->sort_bitvectors_size = size;

        for( i = 0; i < size; i++ )
        {
            hit_queue->sort_bitvectors[i] = (lcn_bitvector_t*) lcn_list_get( bitvector_list, i );
        }

        if ( pq->less_than == lcn_hit_queue_natural_less_than )
        {
            order_by_flag = LCN_ORDER_BY_NATURAL;
        }

        if ( pq->less_than == lcn_hit_queue_natural_less_than_ordered )
        {
            order_by_flag = LCN_ORDER_BY_NATURAL;
            ordered_query_flag = LCN_TRUE;
        }

        lcn_hit_queue_choose_sort_function( hit_queue,
                                            order_by_flag,
                                            (size>0), /* boolean to schow we have sort bitvectors */
                                            ordered_query_flag );
    }
    while(0);

    return APR_SUCCESS;
}


END_C_DECLS
