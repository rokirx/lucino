#include "top_doc_collector.h"
#include "index_reader.h"
#include "field_sorted_hit_queue.h"

/** Return the {@link IndexReader} this searches. */

lcn_index_reader_t*
lcn_index_searcher_reader_get( lcn_searcher_t* index_searcher )
{
    return index_searcher->reader;
}

apr_status_t
lcn_index_searcher_doc_freq( lcn_searcher_t* index_searcher,
                             lcn_term_t* term,
                             unsigned int* doc_freq )
{
    return lcn_index_reader_doc_freq( index_searcher->reader,
                                      term ,
                                      (int*)doc_freq );
}

static unsigned int
lcn_index_searcher_max_doc( lcn_searcher_t* index_searcher )
{
    return lcn_index_reader_max_doc( index_searcher->reader );

}

apr_status_t
lcn_index_searcher_doc( lcn_searcher_t* index_searcher,
                        lcn_document_t** document,
                        unsigned int n,
                        apr_pool_t* pool )
{
    return lcn_index_reader_document( index_searcher->reader,
                                      document,
                                      n,
                                      pool );
}

apr_status_t
lcn_index_searcher_search_top_docs_by_collector( lcn_searcher_t* index_searcher,
                                                 lcn_top_docs_t** top_docs,
                                                 lcn_weight_t* weight,
                                                 lcn_bitvector_t* bitvector,
                                                 unsigned int n_docs,
                                                 lcn_hit_collector_t* collector,
                                                 apr_pool_t* pool )
{
    apr_status_t s;
    unsigned int size;

    do
    {
        /* prepare query bitvector */

        if ( NULL != index_searcher->query_bitvector )
        {
            collector->query_bitvector = index_searcher->query_bitvector;
        }

        /* prepare boost bitvector */

        if ( NULL != index_searcher->boost_bitvector )
        {
            collector->boost_bitvector = index_searcher->boost_bitvector;
            collector->boost_bitvector_boost = index_searcher->boost_bitvector_boost;
        }

        /* prepare split group value funcion */

        if ( NULL != index_searcher->is_split_group_val )
        {
            collector->is_split_group_val = index_searcher->is_split_group_val;
        }

        /* prepare counting bitvectors */

        if ( NULL != index_searcher->counting_bitvector_list &&
             0 < (size = lcn_list_size( index_searcher->counting_bitvector_list )))
        {
            unsigned int i;

            LCNPV( collector->counting_bitvectors = apr_palloc( collector->pool,
                                                                size * sizeof(lcn_bitvector_t*)),
                   APR_ENOMEM );

            collector->counting_bitvectors_size = size;

            for( i = 0; i < size; i++ )
            {
                collector->counting_bitvectors[i] = (lcn_bitvector_t*)
                    lcn_list_get( index_searcher->counting_bitvector_list, i );
            }

            collector->bv_counts_size = 1 + size;
            collector->bv_counts = (unsigned int*)
                apr_pcalloc( pool, sizeof(unsigned int) * collector->bv_counts_size );
        }

        /* prepare custom counter */

        if ( NULL != index_searcher->custom_counter )
        {
            collector->custom_counter = index_searcher->custom_counter;
            collector->custom_data    = index_searcher->custom_data;
        }

        /* prepare group integer array */

        if ( NULL != index_searcher->group_by_field )
        {
            /* default value 0 marks documents not to be grouped */

            LCNCE( lcn_index_reader_get_int_field_values( index_searcher->reader,
                                                          &(collector->group_array),
                                                          index_searcher->group_by_field,
                                                          0 ));

            LCNCE( lcn_list_create( &(collector->queue_list), 1 + n_docs * 2, collector->pool ));
        }

        /* prepare boost field integer array */

        if ( NULL != index_searcher->boost_field )
        {
            lcn_bool_t has_field = LCN_FALSE;

            LCNCE( lcn_index_reader_has_field( index_searcher->reader,
                                               &has_field,
                                               index_searcher->boost_field ));

            if ( has_field )
            {
                LCNCE( lcn_index_reader_get_int_field_values( index_searcher->reader,
                                                              &(collector->boost_array),
                                                              index_searcher->boost_field,
                                                              0 ));
            }
        }

        s = lcn_index_searcher_search_into_hc( index_searcher,
                                               weight,
                                               bitvector,
                                               collector,
                                               pool );

        if( s )
        {
            if( ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ) ||
                ( LCN_ERR_SCAN_ENUM_NO_MATCH == s ) )
            {
                break;
            }
            LCNCE( s );
        }

        LCNCE( lcn_top_doc_collector_top_docs( collector, top_docs, pool ) );
    }
    while(0);

    return s;
}



apr_status_t
lcn_index_searcher_search_top_docs_pq( lcn_searcher_t* index_searcher,
                                       lcn_top_docs_t** top_docs,
                                       lcn_weight_t* weight,
                                       lcn_bitvector_t* bitvector,
                                       unsigned int n_docs,
                                       lcn_priority_queue_t *hit_queue,
                                       apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;

    /* create top field doc collector */

    // TODO hier weitermachen

    do
    {
#if 0
        lcn_hit_collector_t* collector;

        LCNCE( lcn_top_field_doc_collector_create( &collector,
                                                   index_searcher->reader,
                                                   sort_fields,
                                                   n_docs,
                                                   pool ) );

        LCNCE( lcn_index_searcher_search_top_docs_by_collector( index_searcher,
                                                                top_docs,
                                                                weight,
                                                                bitvector,
                                                                n_docs,
                                                                collector,
                                                                pool ));
#endif
    }
    while(0);

    return s;
}


apr_status_t
lcn_index_searcher_sort_fields_hit_queue( lcn_searcher_t* index_searcher,
                                          lcn_hit_queue_t **hq,
                                          unsigned int n_docs,
                                          lcn_list_t *sort_fields,
                                          apr_pool_t* pool )
{
    apr_status_t s;

    /* create top field doc collector */

    do
    {
        lcn_field_sorted_hit_queue_t *hit_queue;

        LCNCR( lcn_field_sorted_hit_queue_create( &hit_queue, index_searcher->reader, sort_fields, n_docs, pool ) );

        *hq = (lcn_hit_queue_t*) hit_queue;
    }
    while(0);

    return s;
}

apr_status_t
lcn_index_searcher_default_hit_queue( lcn_searcher_t* index_searcher,
                                      lcn_hit_queue_t **hq,
                                      unsigned int n_docs,
                                      apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCR( lcn_hit_queue_create( hq, n_docs, pool ) );
        LCNCE( lcn_hit_queue_order_by( *hq,
                                       index_searcher->order_by_flag,
                                       index_searcher->ordered_query_flag ) );

        if ( NULL != index_searcher->sort_bitvector_list )
        {
            LCNCE( lcn_hit_queue_order_by_bitvectors( *hq, index_searcher->sort_bitvector_list ));
        }
    }
    while( FALSE );

    return s;
}

lcn_index_reader_t*
lcn_index_searcher_reader( lcn_searcher_t* index_searcher )
{
    return index_searcher->reader;
}

apr_status_t
lcn_index_searcher_search_into_hc( lcn_searcher_t* index_searcher,
                                   lcn_weight_t* weight,
                                   lcn_bitvector_t* bitvector,
                                   lcn_hit_collector_t* results,
                                   apr_pool_t* pool )
{
    apr_status_t s = APR_ENOMEM;

    do
    {
        lcn_scorer_t* scorer;

        if( bitvector != NULL )
        {
            lcn_hit_collector_set_filter_bits( results, bitvector );
        }

        s = lcn_weight_scorer( weight,
                               &scorer,
                               index_searcher->reader,
                               pool );

        if( s )
        {
            if( ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ) ||
                ( LCN_ERR_SCAN_ENUM_NO_MATCH == s ) )
            {
                break;
            }
            LCNCE( s );
        }

        LCNCE( lcn_scorer_score( scorer, results ) );
    }
    while( FALSE );

    return s;
}


static void
lcn_index_searcher_init( lcn_searcher_t* index_searcher )
{
    index_searcher->max_doc         = lcn_index_searcher_max_doc;
    index_searcher->doc_freq        = lcn_index_searcher_doc_freq;
    index_searcher->doc             = lcn_index_searcher_doc;

    index_searcher->boost_bitvector_boost = 1.0;

    index_searcher->order_by_flag   = LCN_ORDER_BY_RELEVANCE;
    index_searcher->hit_collector_initial_size = LCN_HIT_COLLECTOR_INITIAL_SIZE;
}

apr_status_t
lcn_index_searcher_create_by_directory( lcn_searcher_t** index_searcher,
                                        lcn_directory_t* dir,
                                        apr_pool_t* pool )
{

    apr_status_t s = APR_SUCCESS;

    do
    {
        LCNPV( *index_searcher = apr_pcalloc( pool, sizeof( lcn_searcher_t ) ), APR_ENOMEM );
        (*index_searcher)->pool = pool;

        LCNCE( lcn_index_reader_create_by_directory( &((*index_searcher)->reader), dir, LCN_TRUE, pool ) );
        LCNCE( lcn_default_similarity_create( &((*index_searcher)->similarity ), pool ) );
        lcn_index_searcher_init( *index_searcher );
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_index_searcher_rewrite( lcn_searcher_t* index_searcher,
                            lcn_query_t* original,
                            lcn_query_t** result,
                            apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_t *rewritten, *query = original;

    do
    {
        LCNCE( lcn_query_rewrite( query, &rewritten, index_searcher->reader, pool ) );

        if( query == rewritten )
        {
            break;
        }

        query = rewritten;
    }
    while( 1);

    if ( APR_SUCCESS == s )
    {
        *result = query;
    }

    return s;
}

apr_status_t
lcn_index_searcher_close( lcn_searcher_t* index_searcher )
{
    return lcn_index_reader_close( index_searcher->reader );
}

apr_status_t
lcn_index_searcher_create_by_reader( lcn_searcher_t** index_searcher,
                                     lcn_index_reader_t* index_reader,
                                     apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *index_searcher = apr_pcalloc( pool, sizeof( lcn_searcher_t ) ), APR_ENOMEM );

        (*index_searcher)->reader = index_reader;
        (*index_searcher)->pool = pool;

        LCNCE( lcn_default_similarity_create( &((*index_searcher)->similarity ), pool ) );

        lcn_index_searcher_init( *index_searcher );

    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_index_searcher_create_by_path( lcn_searcher_t** index_searcher,
                                   const char* path,
                                   apr_pool_t* pool )
{
    apr_status_t s;
    lcn_directory_t* dir;

    LCNCR( lcn_fs_directory_create( &dir, path, LCN_FALSE, pool ) );
    LCNCR( lcn_index_searcher_create_by_directory( index_searcher, dir, pool ) );

    return s;
}
