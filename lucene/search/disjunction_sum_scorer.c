#include "disjunction_sum_scorer.h"
#include "scorer_queue.h"


static apr_status_t
lcn_ds_scorer_init_scorer_queue( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        unsigned int i;

        LCNCE( lcn_scorer_queue_create( &(p->scorer_queue),
                                        lcn_list_size( p->sub_scorers ),
                                        scorer->pool ) );

        for( i = 0; i < lcn_list_size( p->sub_scorers ); i++ )
        {
            lcn_scorer_t* act_scorer = lcn_list_get( p->sub_scorers, i );

            if( APR_SUCCESS == ( s = lcn_scorer_next( act_scorer ) ) )
            {
                lcn_priority_queue_insert( p->scorer_queue, act_scorer );
            }
            else
            {
                s = ( LCN_ERR_NO_SUCH_DOC == s )
                    ? APR_SUCCESS
                    : s;
            }
            LCNCE( s );
        }
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_ds_scorer_advance_after_current( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        lcn_scorer_t* top = lcn_priority_queue_top( p->scorer_queue );
        lcn_priority_queue_t* scorer_queue = p->scorer_queue;

        p->current_doc   = lcn_scorer_doc( top );
        LCNCE( lcn_scorer_score_get( top, &(p->current_score) ) );
        p->nr_matchers = 1;

        do
        {
            if( APR_SUCCESS == ( s = lcn_scorer_next( top ) ) )
            {
                lcn_priority_queue_adjust_top( scorer_queue );
            }
            else if( LCN_ERR_NO_SUCH_DOC == s )
            {
                lcn_priority_queue_pop( scorer_queue );

                if( ((int)lcn_priority_queue_size( scorer_queue )) <
                    ((int)( p->minimum_nr_matchers - p->nr_matchers )) )
                {
                    /* Not enough subscorers left for a match on this document, */
                    /* and also no more chance of any further match.            */

                    return LCN_ERR_NO_SUCH_DOC;
                }

                if( lcn_priority_queue_size( scorer_queue ) == 0 )
                {
                    /*  nothing more to advance, check for last match. */

                    s = APR_SUCCESS;
                    break;
                }

                s = APR_SUCCESS;
            }
            else
            {
               LCNCE( s );
            }

            top = lcn_priority_queue_top( scorer_queue );

            if( lcn_scorer_doc( top ) != p->current_doc )
            {
                break;
            }
            else
            {
                lcn_score_t top_score = {0};

                LCNCE( lcn_scorer_score_get( top, &top_score ) );
                p->current_score.float_val += top_score.float_val;

                /* TODO: match logic */
                p->nr_matchers++;
            }
        }
        while( TRUE );

        if( APR_SUCCESS == s )
        {
            if( p->nr_matchers >= p->minimum_nr_matchers )
            {
                return s;
            }
            else if( lcn_priority_queue_size( scorer_queue ) < p->minimum_nr_matchers )
            {
                return LCN_ERR_NO_SUCH_DOC;
            }
        }
    }
    while( APR_SUCCESS == s );

    return s;
}

static apr_status_t
lcn_dso_scorer_advance_after_current( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        lcn_scorer_t* top = lcn_priority_queue_top( p->scorer_queue );
        lcn_priority_queue_t* scorer_queue = p->scorer_queue;

        p->current_doc   = lcn_scorer_doc( top );
        LCNCE( lcn_scorer_score_get( top, &(p->current_score) ) );
        p->nr_matchers = 1;

        do
        {
            if( APR_SUCCESS == ( s = lcn_scorer_next( top ) ) )
            {
                lcn_priority_queue_adjust_top( scorer_queue );
            }
            else if( LCN_ERR_NO_SUCH_DOC == s )
            {
                lcn_priority_queue_pop( scorer_queue );

                if( ((int)lcn_priority_queue_size( scorer_queue )) <
                    ((int)( p->minimum_nr_matchers - p->nr_matchers )) )
                {
                    /* Not enough subscorers left for a match on this document, */
                    /* and also no more chance of any further match.            */

                    return LCN_ERR_NO_SUCH_DOC;
                }

                if( lcn_priority_queue_size( scorer_queue ) == 0 )
                {
                    /*  nothing more to advance, check for last match. */

                    s = APR_SUCCESS;
                    break;
                }

                s = APR_SUCCESS;
            }
            else
            {
               LCNCE( s );
            }

            top = lcn_priority_queue_top( scorer_queue );

            if( lcn_scorer_doc( top ) != p->current_doc )
            {
                break;
            }
            else
            {
                p->nr_matchers++;
            }
        }
        while( TRUE );

        if( APR_SUCCESS == s )
        {
            if( p->nr_matchers >= p->minimum_nr_matchers )
            {
                return s;
            }
            else if( lcn_priority_queue_size( scorer_queue ) < p->minimum_nr_matchers )
            {
                return LCN_ERR_NO_SUCH_DOC;
            }
        }
    }
    while( APR_SUCCESS == s );

    return s;
}

static apr_status_t
lcn_ds_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( NULL == p->scorer_queue )
        {
            LCNCE( lcn_ds_scorer_init_scorer_queue( scorer ) );
        }

        if( lcn_priority_queue_size( p->scorer_queue ) <
            p->minimum_nr_matchers )
        {
            s = LCN_ERR_NO_SUCH_DOC;
            break;
        }
        else
        {
            s = lcn_ds_scorer_advance_after_current( scorer );
        }
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_dso_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( NULL == p->scorer_queue )
        {
            LCNCE( lcn_ds_scorer_init_scorer_queue( scorer ) );
        }

        if( lcn_priority_queue_size( p->scorer_queue ) <
            p->minimum_nr_matchers )
        {
            s = LCN_ERR_NO_SUCH_DOC;
            break;
        }
        else
        {
            s = lcn_dso_scorer_advance_after_current( scorer );
        }
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_ds_scorer_skip_to( lcn_scorer_t* scorer,
                       unsigned int target )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( NULL == p->scorer_queue )
        {
            LCNCE( lcn_ds_scorer_init_scorer_queue( scorer ) );
        }

        if( lcn_priority_queue_size( p->scorer_queue ) < p->minimum_nr_matchers )
        {
            return LCN_ERR_NO_SUCH_DOC;
        }

        if( target <= p->current_doc )
        {
            target = p->current_doc + 1;
        }

        do
        {
            lcn_scorer_t* top = lcn_priority_queue_top( p->scorer_queue );

            if( lcn_scorer_doc( top ) >= target )
            {
                return lcn_ds_scorer_advance_after_current( scorer );
            }

            if ( APR_SUCCESS == (s = lcn_scorer_skip_to( top, target )) )
            {
                lcn_priority_queue_adjust_top( p->scorer_queue );
            }
            else if( s != LCN_ERR_NO_SUCH_DOC )
            {
                LCNCM( s, "Error when skipping documents" );
            }
            else
            {
                s = APR_SUCCESS;
                lcn_priority_queue_pop( p->scorer_queue );
                if( lcn_priority_queue_size( p->scorer_queue ) <
                    p->minimum_nr_matchers )
                {
                    return LCN_ERR_NO_SUCH_DOC;
                }
            }
        }
        while( APR_SUCCESS == s );
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_dso_scorer_skip_to( lcn_scorer_t* scorer,
                       unsigned int target )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        if( NULL == p->scorer_queue )
        {
            LCNCE( lcn_ds_scorer_init_scorer_queue( scorer ) );
        }

        if( lcn_priority_queue_size( p->scorer_queue ) < p->minimum_nr_matchers )
        {
            return LCN_ERR_NO_SUCH_DOC;
        }

        if( target <= p->current_doc )
        {
            target = p->current_doc + 1;
        }

        do
        {
            lcn_scorer_t* top = lcn_priority_queue_top( p->scorer_queue );

            if( lcn_scorer_doc( top ) >= target )
            {
                return lcn_dso_scorer_advance_after_current( scorer );
            }

            if ( APR_SUCCESS == (s = lcn_scorer_skip_to( top, target )) )
            {
                lcn_priority_queue_adjust_top( p->scorer_queue );
            }
            else if( s != LCN_ERR_NO_SUCH_DOC )
            {
                LCNCM( s, "Error when skipping documents" );
            }
            else
            {
                s = APR_SUCCESS;
                lcn_priority_queue_pop( p->scorer_queue );
                if( lcn_priority_queue_size( p->scorer_queue ) <
                    p->minimum_nr_matchers )
                {
                    return LCN_ERR_NO_SUCH_DOC;
                }
            }
        }
        while( APR_SUCCESS == s );
    }
    while( FALSE );

    return s;
}


apr_status_t
lcn_ds_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    lcn_scorer_private_t* p = scorer->priv;
    *score = p->current_score;
    return APR_SUCCESS;
}

static unsigned int
lcn_ds_scorer_doc( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;

    return p->current_doc;
}

unsigned int
lcn_disjunction_sum_scorer_nr_matchers( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;

    return p->nr_matchers;
}

void
lcn_disjunction_sum_scorer_nr_matchers_set( lcn_scorer_t* scorer,
                                            unsigned int nr_matchers )
{
    scorer->priv->nr_matchers = nr_matchers;
}

apr_status_t
lcn_disjunction_sum_scorer_create( lcn_scorer_t** scorer,
                                   lcn_list_t* sub_scorers,
                                   unsigned int minimum_nr_matchers,
                                   apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, NULL, pool ) );

        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        p->nr_scorers  = lcn_list_size( sub_scorers );

        if( p->nr_scorers < 2 )
        {
            s = LCN_ERR_SCORER_NOT_ENOUGH_SUBSCORERS;
            break;
        }

        if( minimum_nr_matchers == 0 )
        {
            LCNLOG_STR( "Minimum number of matchers must be more than 0", "" );
            s = LCN_ERR_INVALID_ARGUMENT;
            break;
        }

        p->minimum_nr_matchers = minimum_nr_matchers;
        p->sub_scorers         = sub_scorers;

        (*scorer)->pool        = pool;
        (*scorer)->doc         = lcn_ds_scorer_doc;
        (*scorer)->score_get   = lcn_ds_scorer_score_get;
        (*scorer)->skip_to     = lcn_ds_scorer_skip_to;
        (*scorer)->next        = lcn_ds_scorer_next;

        (*scorer)->type = "disjunction_sum_scorer";

        (*scorer)->priv = p;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_disjunction_ordered_scorer_create( lcn_scorer_t** scorer,
                                       lcn_list_t* sub_scorers,
                                       unsigned int minimum_nr_matchers,
                                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, NULL, pool ) );

        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        p->nr_scorers  = lcn_list_size( sub_scorers );

        if( p->nr_scorers < 2 )
        {
            s = LCN_ERR_SCORER_NOT_ENOUGH_SUBSCORERS;
            break;
        }

        if( minimum_nr_matchers == 0 )
        {
            LCNLOG_STR( "Minimum number of matchers must be more than 0", "" );
            s = LCN_ERR_INVALID_ARGUMENT;
            break;
        }

        p->minimum_nr_matchers = minimum_nr_matchers;
        p->sub_scorers         = sub_scorers;

        (*scorer)->pool        = pool;
        (*scorer)->doc         = lcn_ds_scorer_doc;
        (*scorer)->score_get   = lcn_ds_scorer_score_get;

        (*scorer)->skip_to     = lcn_dso_scorer_skip_to;
        (*scorer)->next        = lcn_dso_scorer_next;

        (*scorer)->type = "disjunction_ordered_scorer";

        (*scorer)->priv = p;
    }
    while( FALSE );

    return s;
}



