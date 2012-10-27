#include "scorer.h"

struct lcn_scorer_private_t
{
    lcn_coordinator_t* coordinator;
    lcn_scorer_t* scorer;
    lcn_scorer_t* parent;
    unsigned int last_scored_doc;
};

static unsigned int
lcn_single_match_scorer_doc( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;

    return p->scorer->doc( p->scorer );
}

static apr_status_t
lcn_single_match_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    unsigned int doc;
    lcn_scorer_private_t* p = scorer->priv;

    if( ( doc = lcn_scorer_doc( scorer ) ) > p->last_scored_doc )
    {
        p->last_scored_doc = doc;
        p->coordinator->nr_matchers++;
    }

    return lcn_scorer_score_get( p->scorer, score );
}

static apr_status_t
lcn_single_match_scorer_next( lcn_scorer_t* scorer )
{
    return lcn_scorer_next( scorer->priv->scorer );
}

static apr_status_t
lcn_single_match_scorer_skip_to( lcn_scorer_t* scorer, unsigned int target )
{
    return lcn_scorer_skip_to( scorer->priv->scorer, target );
}

apr_status_t
lcn_single_match_scorer_create( lcn_scorer_t** single_match_scorer,
                                lcn_scorer_t* scorer,
                                lcn_coordinator_t* coordinator,
                                lcn_similarity_t* similarity,
                                apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( single_match_scorer,
                                          similarity,
                                          pool ) );

        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        p->coordinator = coordinator;
        p->scorer      = scorer;

        (*single_match_scorer)->priv = p;

        (*single_match_scorer)->type = "single_match_scorer";

        (*single_match_scorer)->doc        = lcn_single_match_scorer_doc;
        (*single_match_scorer)->score_get  = lcn_single_match_scorer_score_get;
        (*single_match_scorer)->next       = lcn_single_match_scorer_next;
        (*single_match_scorer)->skip_to    = lcn_single_match_scorer_skip_to;

    }
    while( FALSE );

    return s;
}

