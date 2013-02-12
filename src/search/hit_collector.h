#ifndef HIT_COLLECTOR_H
#define HIT_COLLECTOR_H

#include "lcn_search.h"
#include "lcn_bitvector.h"
#include "top_docs.h"

/**
 * @defgroup lcn_hit_collector HitCollector
 * @ingroup lcn_search_priv
 * @{
 */

/**
 * HitCollectors are primarily meant to be used to implement queries,
 * sorting and filtering.
 */
struct lcn_hit_collector_t
{

    /**
     * Called once for every non-zero scoring document, with the document number
     * and its score.
     */
    apr_status_t (*collect)( lcn_hit_collector_t* hit_collector,
                             unsigned int doc,
                             lcn_score_t score );

    /**
     * APR pool
     */
    apr_pool_t* pool;

    lcn_priority_queue_t* hq;
    lcn_list_t *sort_bitvector_list;

    /* top_doc_collector */

    unsigned int num_hits;
    lcn_score_t min_score;
    unsigned int min_doc;

    lcn_bitvector_t *query_bitvector;

    lcn_bitvector_t *boost_bitvector;
    double boost_bitvector_boost;

    lcn_bitvector_t **counting_bitvectors;
    unsigned int counting_bitvectors_size;

    apr_status_t (*custom_counter)( void* custom_data,
                                    unsigned int doc );

    void* custom_data;

    unsigned int *bv_counts;
    unsigned int bv_counts_size;

    /**
     * Array to implement grouping of hits by equal int values
     */
    lcn_int_array_t *group_array;
    unsigned int group_val;
    lcn_hit_queue_t *temp_queue;
    unsigned int temp_group_hits;
    lcn_list_t *queue_list;
    lcn_score_t group_min_score;
    unsigned int group_min_doc;

    /**
     * Array to implement custom boosting of documents
     */
    lcn_int_array_t *boost_array;

    /**
     * total number of hits irrespective of their possible grouping
     */
    unsigned int total_hits;

    /**
     * Total number of groups. Important for correct iterating hits.
     */
    unsigned int group_hits;

    /**
     * Vars to implement grouping by split groups
     */

    apr_pool_t *split_group_pool;
    lcn_linked_list_t *score_doc_hits;
    lcn_linked_list_t *score_doc_removed;
    lcn_list_t *score_doc_list;
    lcn_score_doc_t save_score_doc;

    lcn_bool_t (*is_split_group_val)(unsigned int gv );

    /**
     * Support for score doc pool
     */

    unsigned int score_docs_cursor;
    lcn_score_doc_t** score_docs;

    /**
     * score_doc_collector
     */
    lcn_bitvector_t* bitvector;
};

void
lcn_hit_collector_set_filter_bits( lcn_hit_collector_t* hit_collector,
                                   lcn_bitvector_t* bitvector );

unsigned int
lcn_hit_collector_total_hits( lcn_hit_collector_t *hit_collector );


/* @} */

#endif /* HIT_COLLECTOR_H */
