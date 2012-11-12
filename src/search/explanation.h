#ifndef EXPLANATION_H
#define EXPLANATION_H

#include "lcn_search.h"
#include "lcn_util.h"

struct lcn_explanation_t
{
    float value;
    char* description;

    lcn_list_t* details;

    apr_pool_t* pool;
};

#endif /* EXPLANATION_H */
