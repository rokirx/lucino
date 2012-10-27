#ifndef FIELD_SORTED_HIT_QUEUE_H
#define FIELD_SORTED_HIT_QUEUE_H

#include "lucene.h"
#include "hit_queue.h"
#include "score_doc_comparator.h"
#include "priority_queue.h"

BEGIN_C_DECLS

/**
 * @defgroup lcn_field_sorted_hit_queue FieldSortedHitQueue
 * @ingroup lcn_search_priv
 * @{
 */

/**
 * A hit queue using field sorting to collect hits
 */
typedef struct lcn_field_sorted_hit_queue_t
{
    /**
     * Base class of field sorted hit queue.
     */
    lcn_hit_queue_t priority_queue;

    /**
     * Array containing sort doc comparators used to sort hits
     */
    lcn_score_doc_comparator_t **comparators;

    /**
     * Flags to store reverse/not reverse information
     */
    int *reverse;

    /**
     * Number of comparators to be used
     */
    unsigned int comparators_size;
}
lcn_field_sorted_hit_queue_t;


/**
 * @brief  Creates a field sorted hit queue
 *
 * This is used to collect hits in a sort order defined by
 * the list of sort fields.
 *
 * @param queue         Stores new queue
 * @param index_reader  Index reader to retrieve field values for sorting
 * @param sort_fields   List containing sort fields, these are objects of
 *                      type #lcn_sort_field_t
 * @param size          The number of hits to retain.  Must be greater than zero.
 * @param pool          APR pool
 *
 */
apr_status_t
lcn_field_sorted_hit_queue_create( lcn_field_sorted_hit_queue_t ** queue,
                                   lcn_index_reader_t *index_reader,
                                   lcn_list_t *sort_fields,
                                   unsigned int size,
                                   apr_pool_t *pool );

/* @} */

END_C_DECLS

/** @} */

#endif /* SORT_FIELD_H */
