#include "weight.h"


lcn_query_t*
lcn_weight_query( lcn_weight_t* weight )
{
    return weight->query;
}

float
lcn_weight_value( lcn_weight_t* weight )
{
    return weight->value;
}

float
lcn_weight_sum_of_squared_weights( lcn_weight_t* weight )
{
    return weight->sum_of_squared_weights( weight );
}

void
lcn_weight_normalize( lcn_weight_t* weight, float norm )
{
    weight->normalize( weight, norm );
}

apr_status_t
lcn_weight_scorer( lcn_weight_t* weight,
                   lcn_scorer_t** scorer,
                   lcn_index_reader_t* index_reader,
                   apr_pool_t* pool )
{
    return weight->scorer( weight, scorer, index_reader, pool );
}
