#include "scorer.h"

BEGIN_C_DECLS

struct lcn_scorer_private_t
{
    lcn_scorer_t* req_scorer;
    lcn_scorer_t* opt_scorer;
    lcn_bool_t first_time_opt_scorer;
};

static apr_status_t
lcn_ros_scorer_next( lcn_scorer_t* scorer )
{
    return lcn_scorer_next( scorer->priv->req_scorer );
}

static apr_status_t
lcn_ros_scorer_skip_to( lcn_scorer_t* scorer, unsigned int target )
{
    return lcn_scorer_skip_to( scorer->priv->req_scorer, target );
}

static unsigned int
lcn_ros_scorer_doc( lcn_scorer_t* scorer )
{
    return lcn_scorer_doc( scorer->priv->req_scorer );
}

static apr_status_t
lcn_ros_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    apr_status_t s;
    unsigned int cur_doc;
    lcn_score_t req_score = {0};
    lcn_score_t opt_score = {0};
    lcn_scorer_private_t* p = scorer->priv;

    score->float_val = 0.0f;

    do
    {
        cur_doc = lcn_scorer_doc( p->req_scorer );
        LCNCE( lcn_scorer_score_get( p->req_scorer, &req_score ) );

        if( p->first_time_opt_scorer )
        {
            p->first_time_opt_scorer = FALSE;

            if( LCN_ERR_NO_SUCH_DOC ==
                ( s = lcn_scorer_skip_to( p->opt_scorer, cur_doc ) ) )
            {
                p->opt_scorer = NULL;
                *score = req_score;
                s = APR_SUCCESS;
                break;
            }
            LCNCE( s );
        }
        else if( NULL == p->opt_scorer )
        {
            *score = req_score;
            break;
        }
        else if( ( lcn_scorer_doc( p->opt_scorer ) < cur_doc ) &&
                 ( LCN_ERR_NO_SUCH_DOC ==
                   ( s = lcn_scorer_skip_to( p->opt_scorer,
                                             cur_doc ) ) ) )
        {
            p->opt_scorer = NULL;
            *score = req_score;
            s = APR_SUCCESS;
            break;
        }

        LCNCE( s );

        LCNCE( lcn_scorer_score_get( p->opt_scorer, &opt_score ) );



        score->float_val = ( lcn_scorer_doc( p->opt_scorer ) == cur_doc )
            ? req_score.float_val + opt_score.float_val
            : req_score.float_val;
    }
    while( FALSE );


    return s;
}

apr_status_t
lcn_req_opt_sum_scorer_create( lcn_scorer_t** scorer,
                               lcn_scorer_t* req_scorer,
                               lcn_scorer_t* opt_scorer,
                               apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, NULL, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        p->req_scorer         = req_scorer;
        p->opt_scorer         = opt_scorer;

        p->first_time_opt_scorer = LCN_TRUE;

        (*scorer)->type       = "ReqOptSumScorer";

        (*scorer)->next       = lcn_ros_scorer_next;
        (*scorer)->skip_to    = lcn_ros_scorer_skip_to;
        (*scorer)->doc        = lcn_ros_scorer_doc;
        (*scorer)->score_get  = lcn_ros_scorer_score_get;
        (*scorer)->priv       = p;
    }
    while( FALSE );

    return s;
}


END_C_DECLS

