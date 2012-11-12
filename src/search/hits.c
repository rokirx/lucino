#include "hits.h"
#include "top_docs.h"
#include "searcher.h"
#include "document.h"
#include "lcn_bitvector.h"
#include "top_doc_collector.h"
#include "priority_queue.h"

static apr_status_t
lcn_hit_doc_create( lcn_hit_doc_t** hit_doc,
                    lcn_score_t score,
                    unsigned int group_size,
                    unsigned int id,
                    apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *hit_doc = apr_pcalloc( pool, sizeof( lcn_hit_doc_t ) ),
               APR_ENOMEM );

        (*hit_doc)->score      = score;
        (*hit_doc)->id         = id;
        (*hit_doc)->group_size = group_size;
    }
    while( FALSE );

    return s;
}




static apr_status_t
lcn_hits_get_more_docs_impl( lcn_hits_t* hits,
                             lcn_top_docs_t *top_docs )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_ptr_array_t* score_doc_arr;
        float score_norm = 1.0f;
        unsigned int end, i;

        hits->length = top_docs->group_hits;
        hits->total  = top_docs->total_hits;

        if ( NULL != top_docs->bv_counts )
        {
            hits->bv_counts = top_docs->bv_counts;
            hits->bv_counts_size = top_docs->bv_counts_size;
        }

        score_doc_arr = top_docs->score_docs;

        if( ( hits->length > 0 ) &&
            ( top_docs->max_score.float_val > 1.0f ) )
        {
            score_norm = 1.0f / top_docs->max_score.float_val;
        }

        end = ( score_doc_arr->length < hits->length )
            ? score_doc_arr->length
            : hits->length;

        for( i = lcn_list_size( hits->hit_docs ); i < end; i++ )
        {
            lcn_hit_doc_t* hit_doc;
            lcn_score_t score;
            lcn_score_doc_t* act_score_doc = score_doc_arr->arr[i];

            score.int_val   = act_score_doc->score.int_val;
            score.float_val = act_score_doc->score.float_val * score_norm;

            LCNCE( lcn_hit_doc_create( &hit_doc,
                                       score,
                                       act_score_doc->group_size == 0 ? 1: act_score_doc->group_size,
                                       act_score_doc->doc,
                                       hits->pool ) );

            if ( NULL != act_score_doc->group_queue )
            {
                lcn_score_doc_t *sd;

                while((NULL != (sd = lcn_priority_queue_pop( lcn_cast_priority_queue( act_score_doc->group_queue )))) &&
                      sd->doc != hit_doc->id )
                {
                    lcn_hit_doc_t *hd;

                    score.int_val   = sd->score.int_val;
                    score.float_val = sd->score.float_val * score_norm;

                    LCNCE( lcn_hit_doc_create( &hd,
                                               score,
                                               1,
                                               sd->doc,
                                               hits->pool ));

                    hd->next_in_group = hit_doc->next_in_group;
                    hit_doc->next_in_group = hd;
                }
            }

            LCNCE( lcn_list_add( hits->hit_docs, hit_doc ) );
        }
    }
    while(0);

    return s;
}


static apr_status_t
lcn_hits_get_more_docs( lcn_hits_t* hits,
                        lcn_hit_queue_t *hq,
                        unsigned int min )
{
    apr_status_t s;
    lcn_top_docs_t* top_docs;

    if( lcn_list_size( hits->hit_docs ) > min )
    {
        min = lcn_list_size( hits->hit_docs );
    }

    do
    {
        lcn_hit_collector_t *collector = NULL;

        if ( NULL == hq && NULL != hits->hit_queue )
        {
            lcn_priority_queue_clear( lcn_cast_priority_queue( hits->hit_queue ));
            LCNCE( lcn_priority_queue_resize( lcn_cast_priority_queue( hits->hit_queue ), min  * 2 ));
            hq = hits->hit_queue;
        }

        hits->hit_queue = hq;

        LCNCE( lcn_top_doc_collector_create_with_hit_queue( &collector, min * 2, hq, hits->pool ));

        s = lcn_index_searcher_search_top_docs_by_collector( hits->searcher,
                                                             &top_docs,
                                                             hits->weight,
                                                             hits->bitvector,
                                                             min * 2,
                                                             collector,
                                                             hits->pool );


        if( s )
        {
            if( ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ) ||
                ( LCN_ERR_SCAN_ENUM_NO_MATCH == s ) ||
                ( LCN_ERR_EMPTY_QUERY == s ))
            {
                hits->length = 0;
                s = APR_SUCCESS;
                break;
            }

            LCNCE( s );
        }

        LCNCE( lcn_hits_get_more_docs_impl( hits, top_docs ));
    }
    while( FALSE );

    return s;
}

unsigned int
lcn_hits_length( lcn_hits_t* hits )
{
    return hits->length;
}

unsigned int
lcn_hits_total( lcn_hits_t* hits )
{
    return hits->total;
}

unsigned int
lcn_hits_group_hits_size( lcn_hits_t *hits,
                          unsigned int n )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_hit_doc_t* hit_doc;

        LCNCE( lcn_hits_hit_doc( hits, &hit_doc, n ) );

        return hit_doc->group_size;
    }
    while(0);

    return 0;
}

unsigned int
lcn_hits_bitvector_count( lcn_hits_t *hits,
                          unsigned int n )
{
    if ( NULL == hits->bv_counts || n >= hits->bv_counts_size )
    {
        return 0;
    }

    return hits->bv_counts[ n ];
}


apr_status_t
lcn_hits_doc( lcn_hits_t* hits,
              lcn_document_t** doc,
              unsigned int n,
              apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_hit_doc_t* hit_doc;

        LCNCE( lcn_hits_hit_doc( hits, &hit_doc, n ) );
        lcn_hits_remove( hits, hit_doc );

        lcn_hits_add_to_front( hits, hit_doc );

        if( hits->num_docs > hits->max_docs )
        {
            lcn_hits_remove( hits, hits->last );
        }

        /* @todo: when is hit_doc->doc != NULL */

        if( hit_doc->doc == NULL )
        {
            LCNCE( lcn_searcher_doc( hits->searcher,
                                     doc,
                                     hit_doc->id,
                                     pool ) );

            (*doc)->score = hit_doc->score;
        }
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_hits_group_doc( lcn_hits_t* hits,
                    lcn_document_t** doc,
                    unsigned int n,
                    unsigned int group_index,
                    apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;

    if ( 0 == group_index )
    {
        return lcn_hits_doc( hits, doc, n, pool );
    }

    do
    {
        lcn_hit_doc_t* hit_doc, *cursor_doc;
        unsigned i = 0;

        LCNCE( lcn_hits_hit_doc( hits, &hit_doc, n ) );

        cursor_doc = hit_doc;

        while( i < group_index && cursor_doc->next_in_group )
        {
            cursor_doc = cursor_doc->next_in_group;
            i++;
        }

        if ( i != group_index )
        {
            s = LCN_ERR_GROUP_INDEX_OUT_OF_RANGE;
            break;
        }

        LCNCE( lcn_searcher_doc( hits->searcher,
                                 doc,
                                 cursor_doc->id,
                                 pool ) );

        (*doc)->score = cursor_doc->score;
    }
    while(0);

    return s;
}

static apr_status_t
lcn_hits_base_create( lcn_hits_t **hits,
                      lcn_searcher_t *searcher,
                      lcn_query_t *query,
                      lcn_bitvector_t *bv,
                      apr_pool_t *pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *hits = apr_pcalloc( pool, sizeof( lcn_hits_t ) ), APR_ENOMEM );

        LCNCE( lcn_query_weight( query,
                                 &((*hits)->weight),
                                 searcher,
                                 pool ) );

        (*hits)->pool      = pool;
        (*hits)->searcher  = searcher;

        if ( NULL != bv )
        {
            LCNCE( lcn_bitvector_clone( &((*hits)->bitvector), bv, pool ) );
        }

        LCNCE( lcn_list_create( &((*hits)->hit_docs), 200, pool ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_hits_create_custom_queue( lcn_hits_t **hits,
                              lcn_searcher_t* searcher,
                              lcn_query_t *query,
                              lcn_bitvector_t *bv,
                              lcn_hit_queue_t *hit_queue,
                              apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_hits_base_create( hits, searcher, query, bv, pool ) );
    LCNCR( lcn_hits_get_more_docs( *hits, hit_queue, searcher->hit_collector_initial_size ) );

    return s;
}


apr_status_t
lcn_hits_sort_create( lcn_hits_t** hits,
                      lcn_searcher_t* searcher,
                      lcn_query_t* query,
                      lcn_bitvector_t* bv,
                      lcn_list_t *sort_fields,
                      apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_hit_queue_t *hq;

        LCNCR( lcn_hits_base_create( hits, searcher, query, bv, pool ) );
        (*hits)->sort_fields = sort_fields;


        lcn_index_searcher_sort_fields_hit_queue( (*hits)->searcher,
                                                  &hq,
                                                  searcher->hit_collector_initial_size * 2,
                                                  (*hits)->sort_fields,
                                                  (*hits)->pool );
        LCNCR( lcn_hits_get_more_docs( *hits, hq, searcher->hit_collector_initial_size ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_hits_create( lcn_hits_t** hits,
                 lcn_searcher_t* searcher,
                 lcn_query_t* query,
                 lcn_bitvector_t* bv,
                 apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_hit_queue_t *hq;

        LCNCE( lcn_hits_base_create( hits, searcher, query, bv, pool ) );
        LCNCE( lcn_index_searcher_default_hit_queue( (*hits)->searcher,
                                                     &hq,
                                                     searcher->hit_collector_initial_size * 2,
                                                     (*hits)->pool ));
        LCNCE( lcn_hits_get_more_docs( *hits, hq, searcher->hit_collector_initial_size ) );
    }
    while(0);

    return s;
}

apr_status_t
lcn_hits_hit_doc( lcn_hits_t* hits,
                  lcn_hit_doc_t** result,
                  unsigned int n )
{
    apr_status_t s = APR_SUCCESS;

    if( n >= hits->length )
    {
        LCNLOG_SIZE( "Invalid document requested. " "Hits length", hits->length );
        LCNLOG_SIZE( "Requested document", n );
        return LCN_ERR_INDEX_OUT_OF_RANGE;
    }

    if( n >= lcn_list_size( hits->hit_docs ) )
    {
        LCNCR( lcn_hits_get_more_docs( hits, NULL, n ) );
    }

    *result = lcn_list_get( hits->hit_docs, n );

    return s;
}


void
lcn_hits_add_to_front( lcn_hits_t* hits, lcn_hit_doc_t* hit_doc )
{
    if( hits->first == NULL )
    {
        hits->last = hit_doc;
    }
    else
    {
        hits->first->prev = hit_doc;
    }

    hit_doc->next = hits->first;
    hits->first   = hit_doc;
    hit_doc->prev = NULL;

    hits->num_docs++;
}

void
lcn_hits_remove( lcn_hits_t* hits, lcn_hit_doc_t* hit_doc )
{
    if( hit_doc->doc == NULL )
    {
        return;
    }

    if( hit_doc->next == NULL )
    {
        hits->last = hit_doc->prev;
    }
    else
    {
        hit_doc->next->prev = hit_doc->prev;
    }

    if( hit_doc->prev == NULL )
    {
        hits->first = hit_doc->next;
    }
    else
    {
        hit_doc->prev->next = hit_doc->next;
    }

    hits->num_docs--;
}

