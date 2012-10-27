#include "scorer.h"

struct lcn_scorer_private_t
{
    lcn_bool_t first_time;
    lcn_scorer_t* req_scorer;
    lcn_scorer_t* opt_scorer;
    lcn_scorer_t* excl_scorer;
};

static apr_status_t
lcn_re_scorer_to_non_excluded( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p;
    unsigned int excl_doc;

    p = scorer->priv;
    excl_doc = lcn_scorer_doc( p->excl_scorer );


    do
    {
        unsigned int req_doc = lcn_scorer_doc( p->req_scorer );

        if( req_doc < excl_doc )
        {
            return APR_SUCCESS;
        }
        else if( req_doc > excl_doc )
        {
            if( LCN_ERR_NO_SUCH_DOC ==
                ( s = lcn_scorer_skip_to( p->excl_scorer, req_doc ) ) )
            {
                p->excl_scorer = NULL;
                return APR_SUCCESS;
            }
            LCNCE( s );

            excl_doc = lcn_scorer_doc( p->excl_scorer );

            if( excl_doc > req_doc )
            {
                return APR_SUCCESS;
            }
        }
    }
    while( APR_SUCCESS == ( s = lcn_scorer_next( p->req_scorer ) ) );

    if( LCN_ERR_NO_SUCH_DOC == s )
    {
        p->req_scorer = NULL;
        return LCN_ERR_NO_SUCH_DOC;
    }
    else if( s )
    {
        LCNRM( s, "Error getting next non excluded Document" );
    }

    return s;
}

static apr_status_t
lcn_re_scorer_skip_to( lcn_scorer_t* scorer, unsigned int target )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( p->first_time )
        {
            p->first_time = FALSE;

            if( LCN_ERR_NO_SUCH_DOC ==
                ( s = lcn_scorer_skip_to( p->excl_scorer, target ) ) )
            {
                p->excl_scorer = NULL;
            }
        }
        LCNCE( s );
        if( NULL == p->req_scorer )
        {
            return LCN_ERR_NO_SUCH_DOC;
        }
        if( NULL == p->excl_scorer )
        {
            return lcn_scorer_skip_to( p->req_scorer, target );
        }
        if( LCN_ERR_NO_SUCH_DOC ==
            ( s = lcn_scorer_skip_to( p->req_scorer, target ) ) )
        {
            p->req_scorer = NULL;
            return LCN_ERR_NO_SUCH_DOC;
        }

        LCNCE( lcn_re_scorer_to_non_excluded( scorer ) );
    }
    while( FALSE );

    if( s && s != LCN_ERR_NO_SUCH_DOC )
    {
        LCNRM( s, "Error skipping to document" );
    }

    return s;
}

static apr_status_t
lcn_re_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    if( p->first_time )
    {
        if( LCN_ERR_NO_SUCH_DOC ==
            ( s = lcn_scorer_next( p->excl_scorer ) ) )
        {
            s = APR_SUCCESS;
            p->excl_scorer = NULL;
        }
        p->first_time = FALSE;
    }
    if( NULL == p->req_scorer )
    {
        return LCN_ERR_NO_SUCH_DOC;
    }
    if( LCN_ERR_NO_SUCH_DOC ==
        ( s = lcn_scorer_next( p->req_scorer ) ) )
    {
        p->req_scorer = NULL;
        return LCN_ERR_NO_SUCH_DOC;
    }
    if( NULL == p->excl_scorer )
    {
        return APR_SUCCESS;
    }


    return lcn_re_scorer_to_non_excluded( scorer );
}

static unsigned int
lcn_re_scorer_doc( lcn_scorer_t* scorer )
{
    return lcn_scorer_doc( scorer->priv->req_scorer );
}

static apr_status_t
lcn_re_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    return lcn_scorer_score_get( scorer->priv->req_scorer, score );
}

apr_status_t
lcn_req_excl_scorer_create( lcn_scorer_t** scorer,
                            lcn_scorer_t* req_scorer,
                            lcn_scorer_t* excl_scorer,
                            apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, NULL, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        p->first_time      = TRUE;
        p->req_scorer      = req_scorer;
        p->excl_scorer     = excl_scorer;

        (*scorer)->pool            = pool;
        (*scorer)->doc             = lcn_re_scorer_doc;
        (*scorer)->score_get       = lcn_re_scorer_score_get;
        (*scorer)->next            = lcn_re_scorer_next;
        (*scorer)->skip_to         = lcn_re_scorer_skip_to;
        (*scorer)->type            = "req_excl_scorer";
        (*scorer)->priv            = p;
    }
    while( FALSE );

    return s;
}
