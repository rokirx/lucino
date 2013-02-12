#include "lcn_search.h"
#include "searcher.h"
#include "hits.h"
#include "document.h"

#if 0

/* for debugging only */

apr_pool_t *
lcn_searcher_pool( lcn_searcher_t *searcher )
{
    return searcher->pool;
}

#endif


lcn_bool_t
lcn_searcher_has_norms( lcn_searcher_t *searcher,
                        const char *field )
{
    return lcn_index_reader_has_norms( searcher->reader, field );
}


apr_status_t
lcn_searcher_field_exists( lcn_searcher_t *searcher,
                           const char *field_name,
                           lcn_bool_t *field_exists )
{
    apr_status_t s;
    apr_pool_t *cp = NULL;

    *field_exists = LCN_FALSE;

    do
    {
        lcn_list_t *field_infos;
        unsigned int i;

        LCNCE( apr_pool_create( &cp, searcher->pool ));
        LCNCE( lcn_list_create( &field_infos, 10, cp ));

        LCNCE( lcn_index_reader_get_field_infos( searcher->reader,
                                                 field_infos,
                                                 0,
                                                 0 ));

        for( i = 0; i < lcn_list_size( field_infos ); i++ )
        {
            lcn_field_info_t *f_info = (lcn_field_info_t*) lcn_list_get( field_infos, i );

            if ( 0 == strcmp( field_name, lcn_field_info_name( f_info )) )
            {
                *field_exists = LCN_TRUE;
                break;
            }
        }
    }
    while(0);

    if ( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

lcn_similarity_t*
lcn_searcher_similarity( lcn_searcher_t* searcher )
{
    return searcher->similarity;
}

unsigned int
lcn_searcher_max_doc( lcn_searcher_t* searcher )
{
    return searcher->max_doc( searcher );
}

apr_status_t
lcn_searcher_doc_freq( lcn_searcher_t* searcher,
                       lcn_term_t* term,
                       unsigned int* doc_freq )
{
    return searcher->doc_freq( searcher, term, doc_freq );
}

apr_status_t
lcn_searcher_search( lcn_searcher_t* searcher,
                     lcn_hits_t** hits,
                     lcn_query_t* query,
                     lcn_bitvector_t* bitvector,
                     apr_pool_t* pool )
{
    searcher->ordered_query_flag = ( LCN_QUERY_TYPE_ORDERED == lcn_query_type( query ) );
    return lcn_hits_create( hits, searcher, query, bitvector, pool );
}

apr_status_t
lcn_searcher_search_sort( lcn_searcher_t* searcher,
                          lcn_hits_t** hits,
                          lcn_query_t* query,
                          lcn_bitvector_t* bitvector,
                          lcn_list_t *sort_fields,
                          apr_pool_t* pool )
{
    if ( NULL == sort_fields || 0 == lcn_list_size( sort_fields ))
    {
        return lcn_searcher_search( searcher, hits, query, bitvector, pool );
    }

    return lcn_hits_sort_create( hits, searcher, query, bitvector, sort_fields, pool );
}

apr_status_t
lcn_searcher_search_custom_hit_queue( lcn_searcher_t* searcher,
                                      lcn_hits_t** hits,
                                      lcn_query_t* query,
                                      lcn_bitvector_t* bitvector,
                                      lcn_hit_queue_t *pq,
                                      apr_pool_t* pool )
{
    return lcn_hits_create_custom_queue( hits, searcher, query, bitvector, pq, pool );
}


apr_status_t
lcn_searcher_doc( lcn_searcher_t* searcher,
                  lcn_document_t** document,
                  unsigned int n,
                  apr_pool_t* pool )
{
    return searcher->doc( searcher, document, n, pool );
}

apr_status_t
lcn_searcher_get_next_doc( lcn_searcher_t* searcher,
                           lcn_document_t** new_doc,
                           lcn_document_t* document,
                           apr_pool_t *pool )
{
    if ( document->index_pos >= searcher->max_doc( searcher ) )
    {
        return LCN_ERR_NO_SUCH_DOC;
    }

    return searcher->doc( searcher, new_doc, document->index_pos + 1, pool );
}

apr_status_t
lcn_searcher_group_by( lcn_searcher_t* searcher, const char *fs_field_name )
{
    searcher->group_by_field = apr_pstrdup( searcher->pool, fs_field_name );
    return APR_SUCCESS;
}

apr_status_t
lcn_searcher_order_by( lcn_searcher_t* searcher, int order_by_flag )
{
    searcher->order_by_flag = order_by_flag;
    return APR_SUCCESS;
}

apr_status_t
lcn_searcher_order_by_bitvectors( lcn_searcher_t *searcher,
                                  lcn_list_t *bitvector_list )
{
    searcher->sort_bitvector_list = bitvector_list;
    return APR_SUCCESS;
}

apr_status_t
lcn_searcher_set_counting_bitvectors( lcn_searcher_t *searcher,
                                      lcn_list_t *bitvector_list )
{
    searcher->counting_bitvector_list = bitvector_list;
    return APR_SUCCESS;
}

apr_status_t
lcn_searcher_set_custom_counter( lcn_searcher_t *searcher,
                                 apr_status_t (*custom_counter)( void* custom_data, unsigned int doc ),
                                 void* custom_data )
{
    searcher->custom_counter = custom_counter;
    searcher->custom_data    = custom_data;
    return APR_SUCCESS;
}


apr_status_t
lcn_searcher_set_query_bitvector( lcn_searcher_t *searcher,
                                  lcn_bitvector_t *query_bitvector )
{
    searcher->query_bitvector = query_bitvector;
    return APR_SUCCESS;
}

apr_status_t
lcn_searcher_set_boost_bitvector( lcn_searcher_t *searcher,
                                  lcn_bitvector_t *bitvector,
                                  double boost )
{
    searcher->boost_bitvector = bitvector;
    searcher->boost_bitvector_boost = boost;
    return APR_SUCCESS;
}

apr_status_t
lcn_searcher_set_boost_field( lcn_searcher_t *searcher,
                              const char *boost_field_name )
{
    searcher->boost_field = apr_pstrdup( searcher->pool, boost_field_name );
    return APR_SUCCESS;
}

void
lcn_searcher_set_hit_collector_initial_size( lcn_searcher_t *searcher,
                                             unsigned int hc_initial_size )
{
    searcher->hit_collector_initial_size = hc_initial_size;
}

lcn_bool_t (*lcn_searcher_is_split_group_val( lcn_searcher_t *searcher ))(unsigned int)
{
    return searcher->is_split_group_val;
}

void
lcn_searcher_set_split_group_func( lcn_searcher_t *searcher,
                                   lcn_bool_t (*func)( unsigned int v ) )
{
    searcher->is_split_group_val = func;
}
