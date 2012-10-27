#ifndef FIELD_H
#define FIELD_H

#include "lucene.h"

#define LCN_FIELDS_WRITER_TOKENIZED  (1)
#define LCN_FIELDS_WRITER_BINARY     (2)
#define LCN_FIELDS_WRITER_COMPRESSED (4)


/**
 * @defgroup field_accessor_priv FieldAccessor
 * @ingroup document_priv
 * @{
 */

/**
 * Accessor for lazy fields. Should implement loading of
 * field contents on demand. Is currently used for fixed size fields.
 *
 * Supports currently retrieving of int and binary values. Cen
 * be easily extended to string values if needed.
 */
typedef struct lcn_field_accessor_t lcn_field_accessor_t;

struct lcn_field_accessor_t {

    apr_status_t (*get_int_value)( lcn_field_accessor_t *accessor,
                                   unsigned int *val,
                                   unsigned int doc_id );

    apr_status_t (*get_binary_value)( lcn_field_accessor_t *accessor,
                                      char **val,
                                      unsigned int *len,
                                      unsigned int doc_id,
                                      apr_pool_t *pool );
};

/* @} */
/**
 * @defgroup document_priv_desc Document
 * @ingroup document_priv
 * @{
 */

/**
 * @brief Lucene field object
 *
 * Lucene fields can be added to lucene documents. Lucene
 * fields contain all relevant information to be indexed, stored
 * and searched for.
 */
struct lcn_field_t {

    /**
     * APR pool
     */
    apr_pool_t *pool;

    /**
     * Name of the field
     */
    const char *name;

    /**
     * Value of the field
     */
    char *value;

    /**
     * Field boost
     */
    float boost;

    /**
     * Bit field for holding field properties as flags
     */
    unsigned int flags;  /* @depricated */
    lcn_field_type_t field_type;

    /**
     * size of value buffer of binary field
     */
    unsigned int size;

    /**
     * Analyzer for tokenized fields
     */
    lcn_analyzer_t *analyzer;

    /**
     * Default field value for fixed sized fields.
     */
    char *default_value;

    /**
     * Accessor for lazy loading of stored fields.
     */
    lcn_field_accessor_t *accessor;

    /**
     * Flag indicating the loading of the field on demand
     */
    lcn_bool_t is_lazy;

    /**
     * Internal document of the current document.
     */
    unsigned int doc_id;
};

/** @} */


#endif
