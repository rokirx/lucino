#ifndef SORT_FIELD_H
#define SORT_FIELD_H

#include "lcn_search.h"

/**
 * @defgroup lcn_search_priv Search
 * @ingroup lucene_priv
 * @{
 */


#define LCN_SORT_FIELD_SCORE  ( 0 )
#define LCN_SORT_FIELD_DOC    ( 1 )
#define LCN_SORT_FIELD_AUTO   ( 2 )
#define LCN_SORT_FIELD_STRING ( 3 )

/**
 * Sort documents by int value of the field.
 * For this to be possible all field values should be
 * valid strings representing an integer, as they are
 * converted to integer values for sorting.
 */
#define LCN_SORT_FIELD_FLOAT  ( 5 )
#define LCN_SORT_FIELD_CUSTOM ( 9 )


BEGIN_C_DECLS

/**
 * Stores name, data type (integer, string,...) and
 * sort order (reverse or not) of a field
 */
struct lcn_sort_field_t
{
    /**
     * Name of the field
     */
    const char* name;

    /**
     * Data type of the field. Currently only #LCN_SORT_FIELD_INT is supported.
     */
    unsigned int type;

    /**
     * Sort order
     */
    lcn_bool_t reverse;

    /**
     * Default int value
     */
    int default_int;

    /* private SortComparatorSource factory; */
    /* Is not yet implemented                */
};

/**
 * @brief Generates a string representation of a sort_field.
 *
 * This is used as a hash key to cache field values in a hash.
 *
 * For a field "int_field" with type #LCN_SORT_FIELD_INT and reverse
 * sort order the string representation is '&lt;int&gt;"int_field"!'
 *
 * @param sort_field Sort field
 * @param str        Stores the string representation of sort_field
 * @param pool       APR pool
 */
apr_status_t
lcn_sort_field_to_string( lcn_sort_field_t *sort_field,
                          char **str,
                          apr_pool_t *pool );



END_C_DECLS

/** @} */

#endif /* SORT_FIELD_H */
