#ifndef SEARCHER_H
#define SEARCHER_H


#include "lcn_search.h"
#include "lcn_index.h"

#define LCN_HIT_COLLECTOR_INITIAL_SIZE (50)

struct lcn_searcher_t
{
    apr_pool_t *pool;

    lcn_index_reader_t* reader;
    lcn_bool_t close_reader;
    int order_by_flag;
    lcn_bool_t ordered_query_flag;
    lcn_list_t *sort_bitvector_list;
    lcn_list_t *counting_bitvector_list;
    lcn_bitvector_t *query_bitvector;

    lcn_bitvector_t *boost_bitvector;
    double boost_bitvector_boost;

    apr_status_t (*custom_counter)( void* custom_data,
                                    unsigned int doc );
    void* custom_data;


    const char *boost_field;
    const char *group_by_field;

    lcn_bool_t (*is_split_group_val)(unsigned int);

    /*
     * initial size of hit collector, Default = 50
     */
    unsigned int hit_collector_initial_size;

    lcn_similarity_t* similarity;

    apr_status_t
    (*doc)( lcn_searcher_t* searcher,
            lcn_document_t** document,
            unsigned int n,
            apr_pool_t* pool );

    unsigned int
    (*max_doc)( lcn_searcher_t* searcher );

    apr_status_t
    (*doc_freq)( lcn_searcher_t* searcher,
                 lcn_term_t* term,
                 unsigned int* doc_freq );

    apr_status_t
    (*search)( lcn_searcher_t* searcher,
               lcn_query_t* query,
               apr_pool_t* pool );
};


apr_status_t
lcn_index_searcher_search_top_docs_by_collector( lcn_searcher_t* index_searcher,
                                                 lcn_top_docs_t** top_docs,
                                                 lcn_weight_t* weight,
                                                 lcn_bitvector_t* bitvector,
                                                 unsigned int n_docs,
                                                 lcn_hit_collector_t* collector,
                                                 apr_pool_t* pool );

apr_status_t
lcn_index_searcher_default_hit_queue( lcn_searcher_t* index_searcher,
                                      lcn_hit_queue_t **hq,
                                      unsigned int n_docs,
                                      apr_pool_t* pool );

apr_status_t
lcn_index_searcher_sort_fields_hit_queue( lcn_searcher_t* index_searcher,
                                          lcn_hit_queue_t **hq,
                                          unsigned int n_docs,
                                          lcn_list_t *sort_fields,
                                          apr_pool_t* pool );



#endif /* SEARCHER_H */
