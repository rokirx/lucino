#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "lucene.h"
#include "lcn_search.h"

BEGIN_C_DECLS

/**
 * @defgroup document_priv Dokument
 * @ingroup lucene_priv
 * @{
 */

/**
 * @defgroup document_priv_desc Document
 * @ingroup document_priv
 * @{
 */

/**
 * @brief   Object representing lucene document
 *
 * Lucene documents a the entities which can be indexed
 * and searched for in a lucene index.
 */
struct lcn_document_t {

    /**
     * APR pool
     */
    apr_pool_t *pool;

    /**
     * Is not yet in use. TODO: is it correct?
     */
    float boost;

    /**
     * Internal id of the document.
     */
    unsigned int index_pos;

    /**
     * Score of the document
     */
    lcn_score_t score;

    /**
     * List for saving fields
     */
    lcn_list_t *field_list;

    /**
     * List of fixed sized fields, contains elements of type
     * lcn_fs_field_t
     */
    lcn_list_t *fs_fields;
};

/** @} */
/** @} */

END_C_DECLS

#endif /* DOCUMENT_H */
