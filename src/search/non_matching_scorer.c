#include "scorer.h"

unsigned int
lcn_nm_scorer_doc( lcn_scorer_t* scorer )
{
    LCNLOG_STR( "Unsupported Operation",
                "lcn_scorer_doc on no_matching_scorer" );
    return 0;
}

apr_status_t
lcn_nm_scorer_next( lcn_scorer_t* scorer )
{
    return LCN_ERR_NO_SUCH_DOC;
}

apr_status_t
lcn_nm_scorer_skip_to( lcn_scorer_t* scorer, unsigned int target )
{
    return LCN_ERR_NO_SUCH_DOC;
}

apr_status_t
lcn_nm_scorer_score_get( lcn_scorer_t* scorer, float *score )
{
    apr_status_t s;

    LCNRM( LCN_ERR_UNSUPPORTED_OPERATION,
           "lcn_scorer_score_get on no_matching_scorer" );

    return s;
}

apr_status_t
lcn_non_matching_scorer_create( lcn_scorer_t** scorer, apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_default_scorer_create( scorer, NULL, pool ) );

    (*scorer)->next    = lcn_nm_scorer_next;
    (*scorer)->doc     = lcn_nm_scorer_doc;
    (*scorer)->skip_to = lcn_nm_scorer_skip_to;
    (*scorer)->type    = "non_matching_scorer";

    return s;
}

