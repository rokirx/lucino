#ifndef DISJUNCTION_SUM_SCORER_H
#define DISJUNCTION_SUM_SCORER_H

#include "scorer.h"

struct lcn_scorer_private_t
{
    unsigned int  nr_scorers;
    unsigned int  minimum_nr_matchers;
    unsigned int  nr_matchers;
    lcn_list_t* sub_scorers;
    lcn_score_t  current_score;
    unsigned int  current_doc;

    unsigned int last_scored_doc;
    lcn_scorer_t* boolean_scorer;

    apr_status_t
    (*super_score_get)( lcn_scorer_t* scorer, lcn_score_t* score );

    lcn_priority_queue_t* scorer_queue;
};

#endif
