#ifndef SCORE_DOC_COMPARATOR_H
#define SCORE_DOC_COMPARATOR_H

#include "lucene.h"
#include "lcn_index.h"
#include "top_docs.h"

/**
 * @defgroup lcn_score_doc_comparator  ScoreDocComparator
 * @ingroup lcn_search_priv
 * @{
 */

BEGIN_C_DECLS

/**
 * Comparator to compare score docs
 */
typedef struct lcn_score_doc_comparator_t
{
    int (*compare)( struct lcn_score_doc_comparator_t *,
                    lcn_score_doc_t *,
                    lcn_score_doc_t* );

    /**
     * Array containing sort field int values
     */
    lcn_int_array_t *field_order;
}
lcn_score_doc_comparator_t;

/**
 * @brief   Creates a new score doc comparator
 *
 * Java implementation is in FieldSortedHitQueue.comparatorInt.
 *
 * @param comparator   Stores new comparator
 * @param index_reader Index reader to retrief the integer field values
 * @param field_name   Field name of the sort field
 * @param default_val  Default int value to take for documents lacking the field value
 * @param pool         APR Pool
 */
apr_status_t
lcn_score_doc_int_comparator_create( lcn_score_doc_comparator_t **comparator,
                                     lcn_index_reader_t *index_reader,
                                     const char *field_name,
                                     int default_val,
                                     apr_pool_t *pool );

/**
 * @brief    Compares score doc objects
 *
 * @param comparator       The comparator to use
 * @param score_doc_a      First score doc to compare
 * @param score_doc_b      Second score doc to compare
 */
int
lcn_score_doc_comparator_compare( lcn_score_doc_comparator_t *comparator,
                                  lcn_score_doc_t *score_doc_a,
                                  lcn_score_doc_t *score_doc_b );

END_C_DECLS

/** @} */

#endif /* SCORE_DOC_COMPARATOR_H */
