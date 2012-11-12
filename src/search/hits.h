#ifndef HITS_H
#define HITS_H

#include "lcn_search.h"

typedef struct lcn_hit_doc_t lcn_hit_doc_t;

struct lcn_hit_doc_t
{
    lcn_score_t score;
    unsigned int id;
    unsigned int group_size;
    lcn_hit_doc_t* next_in_group;

    lcn_document_t* doc;

    lcn_hit_doc_t* next;
    lcn_hit_doc_t* prev;

};

struct lcn_hits_t
{
    lcn_weight_t* weight;
    lcn_searcher_t* searcher;
    lcn_bitvector_t* bitvector;
    lcn_list_t* sort_fields;

    unsigned int length;
    unsigned int total;
    lcn_list_t* hit_docs;
    float score_norm;

    lcn_hit_doc_t* first;
    lcn_hit_doc_t* last;

    unsigned int num_docs;
    unsigned int max_docs;

    unsigned int bv_counts_size;
    unsigned int *bv_counts;

    lcn_hit_queue_t *hit_queue;

    apr_pool_t* pool;
};

apr_status_t
lcn_hits_create( lcn_hits_t** hits,
                 lcn_searcher_t* searcher,
                 lcn_query_t* query,
                 lcn_bitvector_t* bv,
                 apr_pool_t* pool );

apr_status_t
lcn_hits_hit_doc( lcn_hits_t* hits,
                  lcn_hit_doc_t** result,
                  unsigned int n );

void
lcn_hits_add_to_front( lcn_hits_t* hits, lcn_hit_doc_t* hit_doc );

void
lcn_hits_remove( lcn_hits_t* hits, lcn_hit_doc_t* hit_doc );

apr_status_t
lcn_hits_sort_create( lcn_hits_t** hits,
                      lcn_searcher_t* searcher,
                      lcn_query_t* query,
                      lcn_bitvector_t* bv,
                      lcn_list_t *sort_fields,
                      apr_pool_t* pool );

apr_status_t
lcn_hits_create_custom_queue( lcn_hits_t **hits,
                              lcn_searcher_t* searcher,
                              lcn_query_t *query,
                              lcn_bitvector_t *bv,
                              lcn_hit_queue_t *hit_queue,
                              apr_pool_t* pool );


#endif /* HITS_H */
