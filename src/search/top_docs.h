#ifndef TOP_DOCS_H
#define TOP_DOCS_H

#include "lcn_search.h"
#include "searcher.h"

/**
 * @defgroup lcn_score_doc  ScoreDoc
 * @ingroup lcn_search_priv
 * @{
 */

/**
 * @brief Creates a new score doc object
 *
 * @param score_doc    Stores new score doc object
 * @param doc          Internal doc id
 * @param score        Calculated score of the document
 * @param pool         APR pool
 */
apr_status_t
lcn_score_doc_create( lcn_score_doc_t** score_doc,
                      unsigned int doc,
                      lcn_score_t score,
                      apr_pool_t* pool );

/* @} */

struct lcn_top_docs_t
{

    unsigned int total_hits;
    unsigned int group_hits;
    lcn_score_t max_score;
    lcn_ptr_array_t* score_docs;

    unsigned int bv_counts_size;
    unsigned int *bv_counts;
};

apr_status_t
lcn_top_docs_create( lcn_top_docs_t** top_docs,
                     unsigned int total_hits,
                     unsigned int group_hits,
                     lcn_ptr_array_t* score_docs,
                     lcn_score_t max_score,
                     apr_pool_t* pool );


#endif /* TOP_DOCS_H */
