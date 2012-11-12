#include "conjunction_scorer.h"


static apr_status_t
lcn_ccs_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    lcn_scorer_private_t* p = scorer->priv;

    if( lcn_scorer_doc( p->boolean_scorer ) > p->last_scored_doc ||
        !p->last_doc_set )
    {
        lcn_coordinator_t* coordinator;

        p->last_doc_set = LCN_TRUE;

        coordinator = lcn_boolean_scorer_coordinator( p->boolean_scorer );

        p->last_scored_doc = lcn_scorer_doc( p->boolean_scorer );

        coordinator->nr_matchers += p->req_nr_matchers;
    }

    return p->super_score_get( scorer, score );
}

apr_status_t
lcn_counting_conjunction_sum_scorer_create( lcn_scorer_t** scorer,
                                            lcn_scorer_t* boolean_scorer,
                                            lcn_list_t* scorers,
                                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_similarity_t* sim;

        LCNCE( lcn_default_similarity_create( &sim, pool ) );
        LCNCE( lcn_conjunction_scorer_create( scorer,
                                              sim,
                                              pool ) );

        (*scorer)->priv->super_score_get = (*scorer)->score_get;
        (*scorer)->priv->boolean_scorer  = boolean_scorer;
        (*scorer)->priv->req_nr_matchers = lcn_list_size( scorers );
        (*scorer)->score_get = lcn_ccs_scorer_score_get;
        (*scorer)->type = "counting_conjunction_sum_scorer";
    }
    while( FALSE );

    return s;
}

