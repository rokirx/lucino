#include "query.h"
#include "weight.h"
#include "scorer.h"
#include "boolean_clause.h"

struct lcn_weight_private_t
{
    lcn_list_t* weights;
};

struct lcn_query_private_t
{
    lcn_list_t* clauses;
    unsigned int min_nr_should_match;
    apr_pool_t* rewrite_pool;
    lcn_bool_t disable_coord;
};

lcn_list_t*
lcn_boolean_query_clauses( lcn_query_t* boolean_query )
{
    return boolean_query->priv->clauses;
}

static float
lcn_boolean_weight_sum_of_squared_weights( lcn_weight_t* w )
{
    apr_status_t s;
    unsigned int i;
    float sum = 0.0f;


    for( i = 0; i < lcn_list_size( w->priv->weights ); i++ )
    {
        lcn_boolean_clause_t* c;
        lcn_weight_t* act_w;

        LCNPV( c = lcn_list_get( w->query->priv->clauses, i ), LCN_ERR_NULL_PTR );
        LCNPV( act_w = lcn_list_get( w->priv->weights, i ), LCN_ERR_NULL_PTR );

        if( !lcn_boolean_clause_is_prohibited( c ) )
        {
            sum += lcn_weight_sum_of_squared_weights( act_w );
        }
    }

    sum *= ( lcn_query_boost( w->query ) * lcn_query_boost( w->query ) );

    return sum;
}

static void
lcn_boolean_weight_normalize( lcn_weight_t* w, float n )
{
    apr_status_t s;
    unsigned int i;

    n *= lcn_query_boost( w->query );

    for( i = 0; i < lcn_list_size( w->priv->weights ); i++ )
    {
        lcn_boolean_clause_t* c;
        lcn_weight_t* act_w;

        LCNPV( c = lcn_list_get( w->query->priv->clauses, i ), LCN_ERR_NULL_PTR );
        LCNPV( act_w = lcn_list_get( w->priv->weights, i ), LCN_ERR_NULL_PTR );

        if( !lcn_boolean_clause_is_prohibited( c ) )
        {
            lcn_weight_normalize( act_w, n );
        }
    }
}

static float
lcn_boolean_weight_value_get( lcn_weight_t* w )
{
    return lcn_query_boost( w->query );
}

static apr_status_t
lcn_boolean_weight_scorer( lcn_weight_t* w,
                           lcn_scorer_t** scorer,
                           lcn_index_reader_t* r,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        lcn_weight_private_t* p = w->priv;

        if ( w->query->type == LCN_QUERY_TYPE_ORDERED )
        {
            LCNCE( lcn_ordered_scorer_create( scorer,
                                              w->similarity,
                                              w->query->priv->min_nr_should_match,
                                              w->query->type,
                                              pool ) );

            (*scorer)->type = "ordering_scorer";
        }
        else
        {
            LCNCE( lcn_boolean_scorer_create( scorer,
                                              w->similarity,
                                              w->query->priv->min_nr_should_match,
                                              w->query->type,
                                              pool ) );

            if ( LCN_QUERY_TYPE_ONE_HIT == w->query->type )
            {
                (*scorer)->type = "one_hit_scorer";
            }
        }

        for( i = 0; i < lcn_list_size( p->weights );  i++ )
        {
            lcn_weight_t* act_w;
            lcn_scorer_t* sub_scorer;
            lcn_boolean_clause_t* c;

            LCNPV( c = lcn_list_get( w->query->priv->clauses, i ), LCN_ERR_INDEX_OUT_OF_RANGE );

            LCNPV( act_w = lcn_list_get( p->weights, i ), LCN_ERR_INDEX_OUT_OF_RANGE );

            LCNCE( lcn_weight_scorer( act_w, &sub_scorer, r, pool ) );

            if ( w->query->type == LCN_QUERY_TYPE_ORDERED )
            {
                sub_scorer->order = i+1;
                LCNCE( lcn_ordered_scorer_add( *scorer, sub_scorer ));
            }
            else
            {
                LCNCE( lcn_boolean_scorer_add( *scorer,
                                               sub_scorer,
                                               lcn_boolean_clause_is_required( c ),
                                               lcn_boolean_clause_is_prohibited( c )) );
            }
        }
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_boolean_weight_create( lcn_weight_t** bweight,
                           lcn_searcher_t* searcher,
                           lcn_query_t* boolean_query,
                           apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        lcn_weight_private_t* p;

        LCNPV( *bweight = apr_pcalloc( pool, sizeof( lcn_weight_t ) ), APR_ENOMEM );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_weight_private_t ) ), APR_ENOMEM );

        LCNCE( lcn_similarity_clone( lcn_searcher_similarity( searcher ),
                                     &((*bweight)->similarity ),
                                     pool ) );

        LCNCE( lcn_list_create( &(p->weights), 10, pool ) );

        (*bweight)->query     = boolean_query;
        (*bweight)->scorer    = lcn_boolean_weight_scorer;
        (*bweight)->value_get = lcn_boolean_weight_value_get;
        (*bweight)->normalize = lcn_boolean_weight_normalize;
        (*bweight)->priv      = p;
        (*bweight)->sum_of_squared_weights = lcn_boolean_weight_sum_of_squared_weights;

        for( i = 0; i < lcn_list_size( boolean_query->priv->clauses ); i++ )
        {
            lcn_boolean_clause_t* c;
            lcn_weight_t* w;
            lcn_query_t* query;

            LCNPV( c = lcn_list_get( boolean_query->priv->clauses, i ), LCN_ERR_INDEX_OUT_OF_RANGE );
            LCNPV( query = lcn_boolean_clause_query( c ), LCN_ERR_NULL_PTR );

            if( APR_SUCCESS !=
                ( s = lcn_query_create_weight( query, &w, searcher, pool ) ) )
            {
                if( LCN_ERR_EMPTY_QUERY == s )
                {
                    s = APR_SUCCESS;
                    continue;
                }
            }
            LCNCE( lcn_list_add( p->weights, w ) );
        }
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_boolean_query_create_weight( lcn_query_t* q,
                                 lcn_weight_t** weight,
                                 lcn_searcher_t* searcher,
                                 apr_pool_t* pool )
{
    return lcn_boolean_weight_create( weight, searcher, q, pool );
}

static apr_status_t
lcn_boolean_query_clone( lcn_query_t* q,
                         lcn_query_t** result,
                         apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;

        LCNCE( lcn_boolean_query_create( result, pool ) );

        (*result)->boost = q->boost;

        for( i = 0; i < lcn_list_size( q->priv->clauses ); i++ )
        {
            lcn_boolean_clause_t *clause_clone;
            lcn_query_t* sub_query;

            lcn_boolean_clause_t* act_clause = lcn_list_get( q->priv->clauses, i );

            LCNCE( lcn_query_clone( lcn_boolean_clause_query( act_clause ), &sub_query, pool ) );

            LCNCE( lcn_boolean_clause_create( &clause_clone,
                                              lcn_boolean_clause_occur( act_clause ),
                                              sub_query,
                                              pool ) );

            LCNCE( lcn_list_add( (*result)->priv->clauses, clause_clone ) );
        }

        (*result)->type = q->type;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_boolean_query_to_string( lcn_query_t* query,
                             char** result,
                             const char* field,
                             apr_pool_t* pool )
{
    apr_status_t s;
    lcn_bool_t need_parens = ( ( lcn_query_boost( query ) != 1.0f ) ||
                               ( query->priv->min_nr_should_match > 0 ) );

    do
    {
        unsigned int i;
        apr_pool_t* cp, *lp;
        lcn_string_buffer_t* sb;
        unsigned int min_nr;

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNCE( apr_pool_create( &lp, cp ) );
        LCNCE( lcn_string_buffer_create( &sb, cp ) );

        if( need_parens )
        {
            LCNCE( lcn_string_buffer_append( sb, "(" ) );
        }

        for( i = 0; i < lcn_list_size( query->priv->clauses ); i++ )
        {
            lcn_boolean_clause_t* c;
            lcn_query_t* sub_query;
            char* substr;

            apr_pool_clear( lp );

            c = lcn_list_get( query->priv->clauses, i );

            if( lcn_boolean_clause_is_prohibited( c ) )
            {
                LCNCE( lcn_string_buffer_append( sb, "-" ) );
            }
            else if( lcn_boolean_clause_is_required( c ) )
            {
                LCNCE( lcn_string_buffer_append( sb, "+" ) );
            }

            sub_query = lcn_boolean_clause_query( c );

            LCNCE( lcn_query_to_string( sub_query, &substr, field, lp ) );

            if( lcn_query_type( sub_query ) == LCN_QUERY_TYPE_BOOLEAN )
            {
                LCNCE( lcn_string_buffer_append( sb, "(" ) );
                LCNCE( lcn_string_buffer_append( sb, substr ) );
                LCNCE( lcn_string_buffer_append( sb, ")" ) );
            }
            else
            {
                LCNCE( lcn_string_buffer_append( sb, substr ) );
            }

            if( i != ( lcn_list_size( query->priv->clauses ) - 1 ))
            {
                LCNCE( lcn_string_buffer_append( sb, " "  ) );
            }
        }

        LCNCE( s );

        if( need_parens )
        {
            LCNCE( lcn_string_buffer_append( sb, ")" ) );
        }

        if( (min_nr=query->priv->min_nr_should_match) > 0 )
        {
            LCNCE( lcn_string_buffer_append( sb, "~" ) );
            LCNCE( lcn_string_buffer_append_int( sb, min_nr ) );
        }

        if( lcn_query_boost( query ) != 1.0f )
        {
            char buf[20];

            lcn_to_string_boost( buf, 20, lcn_query_boost( query ) );
            LCNCE( lcn_string_buffer_append( sb, buf ) );
        }

        LCNCE( lcn_string_buffer_to_string( sb, result, pool ) );

        apr_pool_destroy( cp );
    }
    while( FALSE );

    return s;
}

static lcn_bool_t
has_reducible_match_all_docs_query( lcn_query_t* query, lcn_boolean_clause_t **not_clause )
{
    lcn_boolean_clause_t *mad_cl, *not_cl, *save_cl;
    lcn_query_private_t* p = query->priv;

    if ( LCN_QUERY_TYPE_BOOLEAN != lcn_query_type( query ) )
    {
        return LCN_FALSE;
    }

    if ( 2 != lcn_list_size( p->clauses ) )
    {
        return LCN_FALSE;
    }

    if ( NULL == ( mad_cl = lcn_list_get( p->clauses, 0 ) ))
    {
        return LCN_FALSE;
    }

    if ( NULL == ( not_cl = lcn_list_get( p->clauses, 1 ) ))
    {
        return LCN_FALSE;
    }

    if ( LCN_BOOLEAN_CLAUSE_MUST_NOT != lcn_boolean_clause_occur( not_cl ))
    {
        save_cl = not_cl;
        not_cl  = mad_cl;
        mad_cl  = save_cl;
    }

    if ( LCN_BOOLEAN_CLAUSE_MUST_NOT != lcn_boolean_clause_occur( not_cl ) )
    {
        return LCN_FALSE;
    }

    if ( LCN_QUERY_TYPE_MATCH_ALL_DOCS != lcn_query_type( lcn_boolean_clause_query( mad_cl ) ))
    {
        return LCN_FALSE;
    }

    *not_clause = not_cl;

    return LCN_TRUE;
}

static apr_status_t
lcn_boolean_query_rewrite( lcn_query_t* query,
                           lcn_query_t** result,
                           lcn_index_reader_t* reader,
                           apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t* cp = NULL;
    lcn_query_private_t* p = query->priv;

    *result = NULL;

    do
    {
        unsigned int i;

        /* Flag, indicating, that any change occured
         * in the process of rewriting
         */
        lcn_bool_t use_clone = LCN_FALSE;

        /* Flag indicating whether a MUST-clause exists */
        lcn_bool_t has_must_clause = LCN_FALSE;

        /* Flag indicating whether a MUST-clause exists,
         * which is not a match_all_docs_query
         */
        lcn_bool_t has_nontrivial_must_clause = LCN_FALSE;

        /* Indicating an offset of clauses, if f.e. rewriting
         * a clause indicates, that the clause is empty ->removed
         */
        unsigned int clone_offset = 0;
        lcn_query_t* clone = NULL;

        lcn_bool_t has_nonempty_query = LCN_FALSE;

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNCE( lcn_query_clone( query, &clone, cp ) );

        if( lcn_list_size( p->clauses ) == 0 )
        {
            s = LCN_ERR_EMPTY_QUERY;
            break;
        }
        else if( lcn_list_size( p->clauses ) == 1  &&
                 LCN_QUERY_TYPE_ONE_HIT != query->type )
        {
            lcn_boolean_clause_t* c;

            LCNPV( c = lcn_list_get( p->clauses, 0 ), LCN_ERR_NULL_PTR );

            if( ! lcn_boolean_clause_is_prohibited( c ) )
            {
                lcn_query_t* sub_query;

                if( APR_SUCCESS ==
                    ( s = lcn_query_rewrite( lcn_boolean_clause_query( c ),
                                             &sub_query,
                                             reader,
                                             pool ) ) )
                {
                    if( query->boost != 1.0f )
                    {
                        lcn_query_boost_set( sub_query,
                                             query->boost *
                                             lcn_query_boost( sub_query ) );
                    }

                    *result = sub_query;
                }
                else if( LCN_ERR_EMPTY_QUERY == s )
                {
                    break;
                }

                LCNCE( s );

                break;
            }
        }

        for( i = 0; i < lcn_list_size( p->clauses ); i++ )
        {
            lcn_boolean_clause_t* c, *c_clone;
            lcn_query_t* sub_query;

            LCNPV( c = lcn_list_get( p->clauses, i ), LCN_ERR_NULL_PTR );

            if( APR_SUCCESS !=
                ( s = lcn_query_rewrite( lcn_boolean_clause_query( c ),
                                         &sub_query,
                                         reader,
                                         pool ) ) )
            {
                if( LCN_ERR_EMPTY_QUERY == s )
                {
                    s = APR_SUCCESS;
                    lcn_list_remove( clone->priv->clauses, i );
                    clone_offset++;
                    use_clone = LCN_TRUE;
                    continue;
                }
                LCNCE( s );
            }

            if ( ( lcn_term_query_is_stop_term( lcn_boolean_clause_query( c )) ||
                   lcn_term_query_is_stop_term( sub_query ) )
                 &&
                 ( has_nonempty_query || i < lcn_list_size( p->clauses )-1 ))
            {
                lcn_list_remove( clone->priv->clauses, i );
                clone_offset++;
                use_clone = LCN_TRUE;
                continue;
            }

            if( sub_query != lcn_boolean_clause_query( c ) )
            {
                use_clone = LCN_TRUE;

                LCNCE( lcn_boolean_clause_create( &c_clone,
                                                  c->occur,
                                                  sub_query,
                                                  pool ) );

                lcn_list_set( clone->priv->clauses, i - clone_offset, c_clone );
            }

            /* look at the current clause of the query being rewritten */
            /* optimize the case ( +(()) ... + (()) )                  */

            while(1)
            {
                lcn_boolean_clause_t* cq;
                int do_break = 1;

                LCNPV( cq = lcn_list_get( clone->priv->clauses, 0 ), LCN_ERR_NULL_PTR );

                if ( lcn_boolean_clause_is_required( cq ) )
                {
                    /* check if sublevel query looks like   +(())         */

                    lcn_query_t *sublevel_1 = lcn_boolean_clause_query( cq );

                    /* assert that it is boolean */

                    if ( LCN_QUERY_TYPE_BOOLEAN == lcn_query_type( sublevel_1 ) ||
                         LCN_QUERY_TYPE_ORDERED == lcn_query_type( sublevel_1 ))
                    {
                        lcn_query_private_t* p1 = sublevel_1->priv;

                        /* assert that is has only one clause */

                        if ( 1 == lcn_list_size( p1->clauses ))
                        {
                            lcn_boolean_clause_t* c1;
                            lcn_query_t *sublevel_2;

                            LCNPV( c1 = lcn_list_get( p1->clauses, 0 ), LCN_ERR_NULL_PTR );

                            /* assert that this clause is not prohibited */

                            if ( ! lcn_boolean_clause_is_prohibited( c1 ) )
                            {
                                lcn_boolean_clause_t *c_clone;

                                sublevel_2 = lcn_boolean_clause_query( c1 );

                                /* then replace sublevel_1 with sublevel_2 */

                                LCNCE( lcn_boolean_clause_create( &c_clone,
                                                                  cq->occur,
                                                                  sublevel_2,
                                                                  pool ) );

                                lcn_list_set( clone->priv->clauses, i - clone_offset, c_clone );

                                use_clone = LCN_TRUE;
                                do_break = 0;
                            }
                        }
                    }
                }

                if ( do_break )
                {
                    break;
                }
            }
#if 0
            else if ( ! lcn_boolean_clause_is_required( c ) && ! lcn_boolean_clause_is_prohibited( c ) )
            {
                lcn_query_t *sublevel_1 = lcn_boolean_clause_query( c );

                LCNLOG_QUERY( "?SUBLEVEL", sublevel_1 );

                if ( LCN_QUERY_TYPE_BOOLEAN == lcn_query_type( sublevel_1 ))
                {
                    lcn_query_private_t* p1 = sublevel_1->priv;

                    flog( stderr, "?Clauses : %d\n", lcn_list_size( p1->clauses ) );

                    if ( 1 == lcn_list_size( p1->clauses ))
                    {
                        flog( stderr, "?SHOULD OPTIMIZE\n" );
                    }
                }
            }
#endif


            has_nonempty_query = LCN_TRUE;
        }

        LCNCE( s );

        if ( ! use_clone )
        {
            /* check whether a MUST clause exists */

            for( i = 0; i < lcn_list_size( clone->priv->clauses ); i++ )
            {
                lcn_boolean_clause_t* c;
                lcn_query_t* sub_query;
                lcn_boolean_clause_occur_t occur;

                LCNPV( c = lcn_list_get( clone->priv->clauses, i ), LCN_ERR_NULL_PTR );

                sub_query = lcn_boolean_clause_query( c );
                occur = lcn_boolean_clause_occur( c );

                if ( LCN_BOOLEAN_CLAUSE_MUST == occur )
                {
                    if ( LCN_QUERY_TYPE_MATCH_ALL_DOCS == lcn_query_type( sub_query ) )
                    {
                        has_must_clause = LCN_TRUE;
                    }
                    else
                    {
                        has_nontrivial_must_clause = LCN_TRUE;
                    }
                }

                if ( has_must_clause && has_nontrivial_must_clause )
                {
                    break;
                }
            }

            LCNCE( s );

            for( i = 0; i < lcn_list_size( clone->priv->clauses ); i++ )
            {
                lcn_boolean_clause_t* c;
                lcn_query_t* sub_query;

                LCNPV( c = lcn_list_get( clone->priv->clauses, i ), LCN_ERR_NULL_PTR );
                sub_query = lcn_boolean_clause_query( c );

                if ( has_nontrivial_must_clause &&
                     LCN_QUERY_TYPE_MATCH_ALL_DOCS == lcn_query_type( sub_query ))
                {
                    lcn_list_remove( clone->priv->clauses, i );
                    use_clone = LCN_TRUE;
                    break;
                }

                if ( has_nontrivial_must_clause &&
                     LCN_QUERY_TYPE_BOOLEAN == lcn_query_type( sub_query ) )
                {
                    lcn_boolean_clause_t *not_cl = NULL;

                    if ( has_reducible_match_all_docs_query( sub_query, &not_cl ) )
                    {
                        lcn_list_set( clone->priv->clauses,  i, not_cl );
                        use_clone = LCN_TRUE;
                        break;
                    }
                }
            }
        }

        if( use_clone )
        {
            lcn_query_t* res;
            LCNCE( lcn_query_clone( clone, &res, pool ) );
            *result = res;
        }
        else
        {
            *result = query;
        }
    }
    while( FALSE );

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

unsigned int
lcn_boolean_query_minimum_nr_should_match( lcn_query_t* query )
{
    return query->priv->min_nr_should_match;
}

apr_status_t
lcn_boolean_query_add( lcn_query_t* bq,
                       lcn_query_t* q,
                       lcn_boolean_clause_occur_t occur )
{
    apr_status_t s;

    do
    {
        lcn_query_t* clone;
        lcn_boolean_clause_t* clause;

        if( bq->type != LCN_QUERY_TYPE_BOOLEAN  &&
            !( bq->type == LCN_QUERY_TYPE_ORDERED && occur == LCN_BOOLEAN_CLAUSE_SHOULD ))
        {
            LCNCM( LCN_ERR_INVALID_ARGUMENT, "Trying to add something to a non-boolean-query" );
        }

        LCNCE( lcn_query_clone( q, &clone, bq->pool ) );

        LCNCE( lcn_boolean_clause_create( &clause,  occur, clone, bq->pool ) );
        LCNCE( lcn_list_add( bq->priv->clauses, clause ) );

    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_boolean_query_add_term( lcn_query_t* boolean_query,
                            const char* field,
                            const char* text,
                            lcn_boolean_clause_occur_t occur )
{
    apr_status_t s;

    do
    {
        lcn_query_t* term_query;

        LCNCE( lcn_term_query_create_by_chars( &term_query,
                                               field, text,
                                               boolean_query->pool ) );
        LCNCE( lcn_boolean_query_add( boolean_query, term_query, occur ) );
    }
    while( FALSE );

    return s;
}

void
lcn_boolean_query_minimum_nr_should_match_set( lcn_query_t* query,
                                               unsigned int min )
{
    query->priv->min_nr_should_match = min;
}

apr_status_t
lcn_boolean_query_create( lcn_query_t** query,
                          apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_query_private_t* p;

        LCNCE( lcn_query_create( query, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_query_private_t ) ),
               APR_ENOMEM );

        (*query)->type  = LCN_QUERY_TYPE_BOOLEAN;
        (*query)->pool  = pool;
        (*query)->boost = 1.0f;

        (*query)->create_weight = lcn_boolean_query_create_weight;
        (*query)->to_string     = lcn_boolean_query_to_string;
        (*query)->clone         = lcn_boolean_query_clone;
        (*query)->rewrite       = lcn_boolean_query_rewrite;

        LCNCE( lcn_list_create( &(p->clauses), 10, pool ) );
        (*query)->priv  = p;

    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_boolean_query_create_no_coord( lcn_query_t** boolean_query,
                                   apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_private_t* p;

    do
    {
        LCNCE( lcn_boolean_query_create( boolean_query, pool ) );

        p = (*boolean_query)->priv;

        p->disable_coord = LCN_TRUE;
    }
    while( 0 );

    return s;
}






/* ORDERED QUERY */

apr_status_t
lcn_ordered_query_create( lcn_query_t** query,
                          apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_t *q;

    do
    {
        LCNCE( lcn_boolean_query_create( &q, pool ) );
        q->type = LCN_QUERY_TYPE_ORDERED;

        *query = q;
    }
    while(0);

    return s;
}

apr_status_t
lcn_ordered_query_add( lcn_query_t* oq,
                       lcn_query_t* q )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_boolean_query_add( oq, q, LCN_BOOLEAN_CLAUSE_SHOULD ));
    }
    while(0);

    return s;
}

apr_status_t
lcn_ordered_query_name_list( lcn_query_t *query,
                             lcn_list_t **names_list,
                             apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_query_private_t* p = query->priv;
    lcn_list_t *new_list = NULL;

    do
    {
        unsigned int i;

        LCNASSERT( LCN_QUERY_TYPE_ORDERED == query->type, LCN_ERR_UNSUPPORTED_OPERATION );

        if( lcn_list_size( p->clauses ) == 0 )
        {
            s = LCN_ERR_EMPTY_QUERY;
            break;
        }

        LCNCE( lcn_list_create( &new_list, 10, pool ));

        for( i = 0; i < lcn_list_size( p->clauses ); i++ )
        {
            lcn_query_t *sub_query;
            lcn_boolean_clause_t* c;
            char *name;

            LCNPV( c = lcn_list_get( p->clauses, i ), LCN_ERR_NULL_PTR );
            sub_query = lcn_boolean_clause_query( c );
            name = (char*) lcn_query_name( sub_query );
            if ( NULL == name )
            {
                name = "";
            }

            LCNCE( lcn_list_add( new_list, apr_pstrdup( pool, name )));
        }

        LCNCE( s );

        *names_list = new_list;
    }
    while(0);

    return s;
}

/* ONE HIT QUERY */

apr_status_t
lcn_one_hit_query_create( lcn_query_t** query,
                          lcn_query_t *base_query,
                          apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_t *q;

    do
    {
        LCNCE( lcn_boolean_query_create( &q, pool ) );
        LCNCE( lcn_boolean_query_add( q, base_query, LCN_BOOLEAN_CLAUSE_MUST ));
        q->type = LCN_QUERY_TYPE_ONE_HIT;

        *query = q;
    }
    while(0);

    return s;
}
