#ifndef BOOLEAN_CLAUSE_H
#define BOOLEAN_CLAUSE_H

#include "lcn_search.h"


struct lcn_boolean_clause_t
{
    lcn_query_t* query;
    lcn_bool_t required;
    lcn_bool_t prohibited;

    lcn_boolean_clause_occur_t occur;
};

#endif /* BOOLEAN_CLAUSE_H */
