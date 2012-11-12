#ifndef WEIGHT_H
#define WEIGHT_H

#include "lcn_search.h"

BEGIN_C_DECLS

typedef struct lcn_weight_private_t lcn_weight_private_t;

struct lcn_weight_t
{
    lcn_query_t* query;
    lcn_similarity_t* similarity;

    lcn_weight_private_t* priv;

    float
    (*value_get)( lcn_weight_t* weight );


    float
    (*sum_of_squared_weights)( lcn_weight_t* weight );

    void
    (*normalize)( lcn_weight_t* weight, float norm );

    apr_status_t
    (*scorer)( lcn_weight_t* weight,
               lcn_scorer_t** scorer,
               lcn_index_reader_t* reader,
               apr_pool_t* pool );

    apr_status_t
    (*to_string)( lcn_weight_t* weight, char** result, apr_pool_t* pool );

    /* term_weight */

    float value;
    float idf;
    float query_norm;
    float query_weight;

};

END_C_DECLS

#endif /* WEIGHT_H */
