#include "disjunction_sum_scorer.h"

static apr_status_t
lcn_cds_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    lcn_scorer_private_t* p = scorer->priv;

    if( lcn_scorer_doc( scorer ) > p->last_scored_doc ||
        ( lcn_scorer_doc( scorer ) == 0 && p->last_scored_doc == 0 ))
    {
        lcn_coordinator_t* coordinator = lcn_boolean_scorer_coordinator( p->boolean_scorer );

        p->last_scored_doc = lcn_scorer_doc( scorer );

        coordinator->nr_matchers += p->nr_matchers;
    }

    return p->super_score_get( scorer, score );
}

apr_status_t
lcn_counting_disjunction_sum_scorer_create( lcn_scorer_t** scorer,
                                            lcn_scorer_t* boolean_scorer,
                                            lcn_list_t* scorers,
                                            unsigned int min_nr_should_match,
                                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        if ( 0 == strcmp( "ordering_scorer", boolean_scorer->type ))
        {
            LCNCE( lcn_disjunction_ordered_scorer_create( scorer,
                                                          scorers,
                                                          min_nr_should_match,
                                                          pool ));
        }
        else
        {
            LCNCE( lcn_disjunction_sum_scorer_create( scorer,
                                                      scorers,
                                                      min_nr_should_match,
                                                      pool ));
        }

        (*scorer)->priv->super_score_get =  (*scorer)->score_get;
        (*scorer)->priv->boolean_scorer  = boolean_scorer;
        (*scorer)->score_get = lcn_cds_scorer_score_get;
        (*scorer)->type = "counting_disjunction_sum_scorer";
    }
    while( FALSE );

    return s;
}

