#ifndef QUERY_H
#define QUERY_H

#include "lcn_search.h"

typedef struct lcn_query_private_t lcn_query_private_t;

struct lcn_query_t
{
    lcn_query_type_t type;
    float boost;
    char *name;
    lcn_query_private_t* priv;

    apr_status_t
    (*to_string)( lcn_query_t* query,
                  char** result,
                  const char* field,
                  apr_pool_t* pool );

    apr_status_t
    (*clone)( lcn_query_t* query, lcn_query_t** clone, apr_pool_t* pool );

    apr_status_t
    (*extract_terms)( lcn_query_t* query,
                      lcn_list_t* terms );

    apr_status_t
    (*weight)( lcn_query_t* query,
               lcn_weight_t** weight,
               lcn_searcher_t* searcher,
               apr_pool_t* pool );

    apr_status_t
    (*create_weight)( lcn_query_t* query,
                      lcn_weight_t** weight,
                      lcn_searcher_t* searcher,
                      apr_pool_t* pool );

    apr_status_t
    (*rewrite)( lcn_query_t* query,
                lcn_query_t** result,
                lcn_index_reader_t* reader,
                apr_pool_t* pool );


    /* term_query */

    lcn_term_t* term;
    unsigned int pos;
    lcn_bool_t stop_term;

    /* filtered_query */

    lcn_bitvector_t *bv;

    /* boolean_query */

    apr_pool_t* pool;
};

apr_status_t
lcn_query_create( lcn_query_t** query, apr_pool_t* pool );

#endif /* QUERY_H */
