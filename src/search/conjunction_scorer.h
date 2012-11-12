#ifndef CONJUNCTION_SCORER_H
#define CONJUNCTION_SCORER_H

#include "scorer.h"

struct lcn_scorer_private_t
{
    lcn_linked_list_t* scorers;
    lcn_bool_t first_time;
    lcn_bool_t more;
    float coord;

    unsigned int req_nr_matchers;
    unsigned int last_scored_doc;
    lcn_bool_t last_doc_set;
    lcn_scorer_t* boolean_scorer;

    apr_status_t
    (*super_score_get)( lcn_scorer_t* scorer, lcn_score_t* score );
};

#endif /* CONJUNCTION_SCORER_H */
