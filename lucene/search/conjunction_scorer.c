#include "conjunction_scorer.h"
#include "stdlib.h"

static int
lcn_conjunction_scorer_compare( const void* ptr1, const void* ptr2 )
{
    return lcn_scorer_doc( (lcn_scorer_t*) *((void**)ptr1)) -
           lcn_scorer_doc( (lcn_scorer_t*) *((void**)ptr2));
}

static apr_status_t
lcn_conjunction_scorer_sort_scorers( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        unsigned int i;
        apr_pool_t* cp;
        lcn_ptr_array_t* scorers;

        LCNCE( apr_pool_create( &cp, scorer->pool ) );
        LCNCE( lcn_linked_list_to_array( p->scorers, &scorers, cp ) );

        qsort( scorers->arr,
               scorers->length,
               sizeof( lcn_scorer_t* ),
               lcn_conjunction_scorer_compare );

        lcn_linked_list_clear( p->scorers );

        for( i = 0; i < scorers->length; i++ )
        {
            LCNCE( lcn_linked_list_add_last( p->scorers,
                                             scorers->arr[i] ) );
        }
        LCNCE( s );

        apr_pool_destroy( cp );
    }
    while( FALSE );

    return s;
}


static apr_status_t
lcn_conjunction_scorer_init( lcn_scorer_t* scorer,
                             lcn_bool_t init_scorers )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p;
    unsigned int n_scorers;

    p = scorer->priv;
    n_scorers = lcn_linked_list_size( p->scorers );


    p->coord = lcn_similarity_coord( scorer->similarity,
                                          n_scorers, n_scorers );
    do
    {
        if( init_scorers )
        {
            const lcn_linked_list_el_t* el;

            for( el = lcn_linked_list_first( p->scorers );
                 NULL != el && p->more;
                 el = lcn_linked_list_next( el ) )
            {
                lcn_scorer_t* cur_scorer =
                    (lcn_scorer_t*)lcn_linked_list_content( el );
                p->more = ( APR_SUCCESS ==
                                 ( s = lcn_scorer_next( cur_scorer ) ) );
            }

            if( p->more == TRUE )
            {
                LCNCE( lcn_conjunction_scorer_sort_scorers( scorer ) );
            }

        }
        p->first_time = FALSE;
    }
    while( FALSE );

    return s;
}

static lcn_scorer_t*
lcn_cs_first( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;
    const lcn_linked_list_el_t* el = lcn_linked_list_first( p->scorers );

    if( NULL == el )
    {
        return NULL;
    }

    return (lcn_scorer_t*)lcn_linked_list_content( el );
}

static lcn_scorer_t*
lcn_cs_last( lcn_scorer_t* scorer )
{
    lcn_scorer_private_t* p = scorer->priv;
    const lcn_linked_list_el_t* el = lcn_linked_list_last( p->scorers );

    if( NULL == el )
    {
        return NULL;
    }

    return (lcn_scorer_t*)lcn_linked_list_content( el );
}

static apr_status_t
lcn_conjunction_scorer_do_next( lcn_scorer_t* scorer )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        while( p->more &&
               ( lcn_scorer_doc( lcn_cs_first( scorer ) ) <
                 lcn_scorer_doc( lcn_cs_last( scorer ) ) ) )
        {
            const lcn_linked_list_el_t* first;

            s = lcn_scorer_skip_to( lcn_cs_first( scorer ),
                                    lcn_scorer_doc( lcn_cs_last( scorer ) ) );
            p->more = ( s == APR_SUCCESS );

            s = ( s == LCN_ERR_NO_SUCH_DOC ) ? APR_SUCCESS : s;
            LCNCE( s );
            first = lcn_linked_list_remove_first( p->scorers );
            LCNCE( lcn_linked_list_add_last( p->scorers,
                                             lcn_linked_list_content( first )) );
        }

        if( !p->more )
        {
            s = ( APR_SUCCESS == s )
                ? LCN_ERR_NO_SUCH_DOC
                : s;
        }
    }
    while( FALSE );

    if( ( APR_SUCCESS != s ) &&
        ( LCN_ERR_NO_SUCH_DOC != s ) )
    {
        LCNCR( s );
    }

    return ( p->more ) ? APR_SUCCESS : LCN_ERR_NO_SUCH_DOC;
}

static apr_status_t
lcn_conjunction_scorer_skip_to( lcn_scorer_t* scorer,
                                unsigned int target )
{
    apr_status_t s = APR_SUCCESS;
    lcn_scorer_private_t* p = scorer->priv;

    do
    {
        const lcn_linked_list_el_t* el;

        if( p->first_time )
        {
            LCNCE( lcn_conjunction_scorer_init( scorer, FALSE ) );
            p->first_time = FALSE;

        }

        for( el = lcn_linked_list_first( p->scorers );
             el != NULL && p->more;
             el = lcn_linked_list_next( el ) )
        {
            lcn_scorer_t* act_scorer = lcn_linked_list_content( el );

            p->more = ( APR_SUCCESS == ( s=lcn_scorer_skip_to( act_scorer,
                                                                    target ) ) );
            s = ( s == LCN_ERR_NO_SUCH_DOC ) ? APR_SUCCESS : s;
            LCNCE( s );
        }

        if( p->more )
        {
            LCNCE( lcn_conjunction_scorer_sort_scorers( scorer ) );
        }

        s = lcn_conjunction_scorer_do_next( scorer );
    }
    while( FALSE );

    return s;
}




static apr_status_t
lcn_conjunction_scorer_next( lcn_scorer_t* scorer )
{
    apr_status_t s;
    lcn_scorer_private_t* p = scorer->priv;

    if( p->first_time )
    {
        lcn_conjunction_scorer_init( scorer, TRUE );
    }
    else if( p->more )
    {
        lcn_scorer_t* last_scorer;
        last_scorer = lcn_cs_last( scorer );
        p->more = ( APR_SUCCESS == lcn_scorer_next( last_scorer ) );
    }

    s = lcn_conjunction_scorer_do_next( scorer );

    return s;
}

static unsigned int
lcn_conjunction_scorer_doc( lcn_scorer_t* scorer )
{
    lcn_scorer_t* first = lcn_cs_first( scorer );
    return lcn_scorer_doc( first );
}

apr_status_t
lcn_conjunction_scorer_add( lcn_scorer_t* scorer, lcn_scorer_t* to_add )
{
    lcn_scorer_private_t* p = scorer->priv;
    return lcn_linked_list_add_last( p->scorers, to_add );
}

apr_status_t
lcn_conjunction_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    apr_status_t s = APR_SUCCESS;
    const lcn_linked_list_el_t* el;
    lcn_scorer_private_t* p = scorer->priv;

    score->float_val = 0.0f;

    for( el = lcn_linked_list_first( p->scorers );
         el != NULL;
         el = lcn_linked_list_next( el ) )
    {
        lcn_scorer_t* act_scorer = lcn_linked_list_content( el );
        lcn_score_t act_score = {0};

        LCNCE( lcn_scorer_score_get( act_scorer, &act_score ) );

        score->float_val += act_score.float_val;
    }

    score->float_val *= p->coord;

    return s;
}

apr_status_t
lcn_conjunction_scorer_create( lcn_scorer_t** scorer,
                               lcn_similarity_t* similarity,
                               apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, similarity, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        LCNCE( lcn_linked_list_create( &(p->scorers ), pool ) );

        p->first_time = TRUE;
        p->more       = TRUE;

        (*scorer)->pool       = pool;
        (*scorer)->doc        = lcn_conjunction_scorer_doc;
        (*scorer)->score_get  = lcn_conjunction_scorer_score_get;
        (*scorer)->next       = lcn_conjunction_scorer_next;
        (*scorer)->skip_to    = lcn_conjunction_scorer_skip_to;
        (*scorer)->type       = "conjunction_scorer";
        (*scorer)->priv       = p;
    }
    while( FALSE );

    return s;
}

