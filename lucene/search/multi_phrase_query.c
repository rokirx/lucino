#include "query.h"
#include "lcn_util.h"
#include "weight.h"
#include "phrase_scorer.h"

struct lcn_query_private_t
{
    char* field;
    lcn_list_t* term_arrays;
    lcn_list_t* positions;

    unsigned int min_slop;
    unsigned int slop;
    unsigned int preserve_term_order;
};

static void
lcn_mpq_weight_normalize( lcn_weight_t* weight, float norm )
{
    weight->query_norm = norm;
    weight->query_weight *= weight->query_norm;
    weight->value = weight->query_weight * weight->idf;
}

static float
lcn_mpq_weight_sum_of_squared_weights( lcn_weight_t* weight )
{
    weight->query_weight = weight->query->boost * weight->idf;

    return weight->query_weight * weight->query_weight;
}

static apr_status_t
lcn_mpq_weight_scorer( lcn_weight_t* weight,
                       lcn_scorer_t** scorer,
                       lcn_index_reader_t* index_reader,
                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_size_array_t* positions;
        lcn_byte_array_t* norms = NULL;

        /* TermPositions */

        lcn_ptr_array_t* tps;
        unsigned int i;
        lcn_query_private_t* p = weight->query->priv;

        if( lcn_list_size( p->term_arrays ) == 0 )
        {
            LCNCM( LCN_ERR_EMPTY_QUERY, "Call for scorer on empty term_arrays-List" );
        }

        LCNCE( lcn_ptr_array_create( &tps,
                                     lcn_list_size( p->term_arrays ),
                                     pool ) );

        for( i = 0; i < lcn_list_size( p->term_arrays ); i++ )
        {
            lcn_list_t* terms;
            lcn_term_docs_t* positions;

            LCNPV( terms = lcn_list_get( p->term_arrays, i ), LCN_ERR_NULL_PTR );

            if( lcn_list_size( terms ) > 1 )
            {
                LCNCE( lcn_multiple_term_positions_create( &positions,
                                                           index_reader,
                                                           terms,
                                                           pool ) );
            }
            else
            {
                lcn_term_t* first_term = lcn_list_get( terms, 0 );

                s = lcn_index_reader_term_positions_by_term( index_reader,
                                                             &positions,
                                                             first_term,
                                                             pool );

                s = ( LCN_ERR_SCAN_ENUM_NO_MATCH == s ) ? APR_SUCCESS : s;
            }

            tps->arr[i] = positions;
        }

        if ( s )
        {
            break;
        }

        LCNCE( lcn_multi_phrase_query_positions( weight->query, &positions, pool ) );

        if ( LCN_ERR_NORMS_NOT_FOUND == ( s = lcn_index_reader_norms( index_reader, &norms, p->field ) ))
        {
            norms = NULL;
            s = APR_SUCCESS;
        }

        LCNCM( s, p->field );

        if( 0 == p->slop )
        {
            LCNCE( lcn_exact_phrase_scorer_create( scorer,
                                                   weight,
                                                   tps,
                                                   positions,
                                                   weight->similarity,
                                                   norms,
                                                   pool ) );
        }
        else
        {
            if ( p->preserve_term_order )
            {
                LCNCE( lcn_sloppy_ordered_phrase_scorer_create( scorer,
                                                                weight,
                                                                tps,
                                                                positions,
                                                                weight->similarity,
                                                                p->slop,
                                                                norms,
                                                                pool ) );
            }
            else
            {
                LCNCE( lcn_sloppy_phrase_scorer_create( scorer,
                                                        weight,
                                                        tps,
                                                        positions,
                                                        weight->similarity,
                                                        p->slop,
                                                        norms,
                                                        pool ) );
            }
        }

    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_mpq_weight_create( lcn_weight_t** weight,
                       lcn_searcher_t* searcher,
                       lcn_query_t* mp_query,
                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int i;
        lcn_weight_t* result;
        lcn_query_private_t* p = mp_query->priv;

        if ( LCN_QUERY_TYPE_MULTIPHRASE != mp_query->type )
        {
            return LCN_ERR_UNSUPPORTED_OPERATION;
        }

        LCNPV( result = apr_pcalloc( pool, sizeof( lcn_weight_t ) ),
               APR_ENOMEM );

        result->query     = mp_query;
        result->normalize = lcn_mpq_weight_normalize;
        result->scorer    = lcn_mpq_weight_scorer;
        result->sum_of_squared_weights = lcn_mpq_weight_sum_of_squared_weights;

        result->similarity = lcn_searcher_similarity( searcher );

        for( i = 0; i < lcn_list_size( p->term_arrays ); i++ )
        {
            unsigned int j;
            lcn_list_t* terms;

            LCNPV( terms = lcn_list_get( p->term_arrays, i ),
                   LCN_ERR_NULL_PTR );

            for( j = 0; j < lcn_list_size( terms ); j++ )
            {
                unsigned int doc_freq, max_doc;
                lcn_term_t* term;

                LCNPV( term = lcn_list_get( terms, j ), LCN_ERR_NULL_PTR );

                max_doc = lcn_searcher_max_doc( searcher );
                LCNCE( lcn_searcher_doc_freq( searcher,
                                              term,
                                              &doc_freq ) );

                result->idf += lcn_similarity_idf( result->similarity,
                                                   doc_freq, max_doc );
            }
            LCNCE( s );
        }

        *weight = result;
    }
    while( 0 );

    return s;
}

static apr_status_t
lcn_mpq_create_weight( lcn_query_t* query,
                              lcn_weight_t** weight,
                              lcn_searcher_t* searcher,
                              apr_pool_t* pool )
{
    return lcn_mpq_weight_create( weight, searcher, query, pool );
}


static apr_status_t
lcn_mpq_to_string( lcn_query_t* query,
                   char** result,
                   const char* field,
                   apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_private_t* p = query->priv;

    do
    {
        unsigned int i;
        apr_pool_t* cp;
        lcn_string_buffer_t* sb;

        if( lcn_list_size( p->term_arrays ) == 0 )
        {
            LCNPV( *result = apr_pstrdup( pool, "" ), APR_ENOMEM );
            break;
        }

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNCE( lcn_string_buffer_create( &sb, cp ) );


        if( 0 != strcmp( p->field, field ) )
        {
            LCNCE( lcn_string_buffer_append( sb, p->field ) );
            LCNCE( lcn_string_buffer_append( sb, ":" ) );
        }
        LCNCE( lcn_string_buffer_append( sb, "\"" ) );
        for( i = 0; i < lcn_list_size( p->term_arrays ); i++ )
        {
            unsigned int j;
            lcn_list_t* terms;

            LCNPV( terms = lcn_list_get( p->term_arrays, i ),
                   LCN_ERR_NULL_PTR );
            if( lcn_list_size( terms ) > 1 )
            {
                LCNCE( lcn_string_buffer_append( sb, "(" ) );

                for( j = 0; j < lcn_list_size( terms ); j++ )
                {
                    lcn_term_t* act_term;
                    const char* act_term_text;

                    LCNPV( act_term = lcn_list_get( terms, j ),
                           LCN_ERR_NULL_PTR );
                    act_term_text = lcn_term_text( act_term );
                    LCNCE( lcn_string_buffer_append( sb,
                                                     act_term_text ) );

                    if( j < ( lcn_list_size( terms ) - 1 ) )
                    {
                        LCNCE( lcn_string_buffer_append( sb, " " ) );
                    }
                }
                LCNCE( s );
                LCNCE( lcn_string_buffer_append( sb, ")" ) );
            }
            else
            {
                  lcn_term_t* act_term;
                  const char* act_term_text;

                  LCNPV( act_term = lcn_list_get( terms, 0 ), LCN_ERR_NULL_PTR );
                  act_term_text = lcn_term_text( act_term );

                  LCNCE( lcn_string_buffer_append( sb, act_term_text ) );
            }
            if( i < ( lcn_list_size( p->term_arrays ) - 1 ) )
            {
                LCNCE( lcn_string_buffer_append( sb, " " ) );
            }
        }

        LCNCE( lcn_string_buffer_append( sb, "\"" ) );

        if( 0 != p->slop )
        {
            LCNCE( lcn_string_buffer_append( sb, "~" ) );
            LCNCE( lcn_string_buffer_append_uint( sb, p->slop ) );
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
    while( 0 );

    return s;
}

static apr_status_t
lcn_mpq_clone( lcn_query_t* query,
               lcn_query_t** clone,
               apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_query_private_t* p = query->priv;

    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    do
    {
        unsigned int i, j;

        LCNCE( lcn_multi_phrase_query_create( clone, pool ) );

        (*clone)->boost = query->boost;

        for( i = 0; i < lcn_list_size( p->term_arrays ); i++ )
        {
            lcn_list_t* cur_term_list;
            lcn_list_t* list_copy;

            LCNPV( cur_term_list = lcn_list_get( p->term_arrays, i ),
                   LCN_ERR_NULL_PTR );

            LCNCE( lcn_list_create( &list_copy,
                                    lcn_list_size( cur_term_list ),
                                    pool ) );

            for( j = 0; j < lcn_list_size( cur_term_list ); j++ )
            {
                lcn_term_t* cur_clone;

                LCNCE( lcn_term_clone( lcn_list_get( cur_term_list, j ), &cur_clone, pool ) );
                LCNCE( lcn_list_add( list_copy, cur_clone ) );
            }
            LCNCE( s );

            LCNCE( lcn_multi_phrase_query_add_terms( *clone, list_copy ) );
            lcn_multi_phrase_query_slop_set( *clone, query->priv->slop );
            lcn_multi_phrase_query_preserve_term_order_set( *clone, query->priv->preserve_term_order );
            lcn_multi_phrase_query_min_slop_set( *clone, query->priv->min_slop );
        }

    }
    while( 0 );

    return s;
}

static apr_status_t
lcn_mpq_rewrite( lcn_query_t* query,
                 lcn_query_t** new_query,
                 lcn_index_reader_t* reader,
                 apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;
    lcn_query_private_t* p = query->priv;

    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    do
    {
        if( lcn_list_size( p->term_arrays ) == 1 )
        {
            unsigned int i;
            lcn_query_t* bq;
            lcn_list_t* terms;

            LCNPV( terms = lcn_list_get( p->term_arrays, 0 ),
                   LCN_ERR_NULL_PTR );
            LCNCE( lcn_boolean_query_create_no_coord( &bq, pool ) );

            for( i = 0; i < lcn_list_size( terms ); i++ )
            {
                lcn_query_t* term_query;

                LCNCE( lcn_term_query_create( &term_query,
                                              (lcn_term_t*)lcn_list_get( terms,
                                                                         i ),
                                              pool ) );
                LCNCE( lcn_boolean_query_add( bq, term_query,
                                              LCN_BOOLEAN_CLAUSE_SHOULD ) );
            }
            *new_query = bq;
            break;
        }
        else
        {
            *new_query = query;
        }
    }
    while( 0 );

    return s;
}


apr_status_t
lcn_multi_phrase_query_add_terms_at( lcn_query_t* query,
                                     const lcn_list_t* terms,
                                     unsigned int position )
{
    apr_status_t s = APR_SUCCESS;
    lcn_query_private_t* p = query->priv;
    unsigned int i;

    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    do
    {
        unsigned int* new_pos;

        if ( lcn_list_size( terms ) == 0 )
        {
            LCNCM( LCN_ERR_INVALID_ARGUMENT,
                   "Adding empty terms list to multi phrase query" );
        }

        if( lcn_list_size( p->term_arrays ) == 0 )
        {
            lcn_term_t* first_term = (lcn_term_t*) lcn_list_get( terms, 0 );
            p->field = apr_pstrdup( query->pool, lcn_term_field( first_term ) );
            LCNPV( p->field, APR_ENOMEM );
        }

        for( i = 0; i < lcn_list_size( terms ); i++ )
        {
            lcn_term_t* cur_term = (lcn_term_t*) lcn_list_get( terms, i );

            if( strcmp( lcn_term_field( cur_term ), p->field ) != 0 )
            {
                LCNLOG_STR( "All phrase terms must be in the same field", p->field );
                LCNLOG_STR( "Erroneous field", lcn_term_field( cur_term ) );

                s = LCN_ERR_INVALID_ARGUMENT;
                break;
            }
        }

        LCNCE( s );

        /* clone lists */

        {
            lcn_list_t *new_list;
            unsigned int i;

            LCNCE( lcn_list_create( &new_list, 10, query->pool ));

            for( i = 0; i < lcn_list_size( terms ); i++ )
            {
                lcn_term_t *clone;

                LCNCE( lcn_term_clone( (lcn_term_t*) lcn_list_get( terms, i ), &clone, query->pool ));
                LCNCE( lcn_list_add( new_list, clone ));
            }
            LCNCE( s );
            LCNCE( lcn_list_add( p->term_arrays, (void*) new_list ) );
        }

        LCNPV( new_pos = apr_pcalloc( lcn_list_pool( p->positions ),
                                      sizeof( unsigned int ) ), APR_ENOMEM );
        *new_pos = position;
        LCNCE( lcn_list_add( p->positions, new_pos ) );
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_multi_phrase_query_add_terms( lcn_query_t* query,
                                  const lcn_list_t* terms )
{
    unsigned int position = 0;
    lcn_query_private_t* p = query->priv;

    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    if( lcn_list_size( p->positions ) > 0 )
    {
        unsigned int last_pos = lcn_list_size( p->positions ) - 1;
        position = *((unsigned int*)lcn_list_get( p->positions, last_pos ) ) + 1;
    }

    return lcn_multi_phrase_query_add_terms_at( query, terms, position );
}

apr_status_t
lcn_multi_phrase_query_positions( lcn_query_t* query,
                                  lcn_size_array_t** positions,
                                  apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_private_t* p = query->priv;
    unsigned int i;

    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    LCNCR( lcn_size_array_create( positions,
                                  lcn_list_size( p->positions ),
                                  pool ) );

    for( i = 0; i < lcn_list_size( p->positions ); i++ )
    {
        (*positions)->arr[i] = *((unsigned int*)lcn_list_get( p->positions, i ) );
    }

    return s;
}

apr_status_t
lcn_multi_phrase_query_add_term( lcn_query_t* query,
                                 const lcn_term_t* term )
{
    apr_status_t s;
    lcn_list_t* terms;

    LCNCR( lcn_list_create( &terms, 1, query->pool ) );
    LCNCR( lcn_list_add( terms, (void*) term ) );
    LCNCR( lcn_multi_phrase_query_add_terms( query, terms ) );

    return s;
}

apr_status_t
lcn_multi_phrase_query_min_slop_set( lcn_query_t* query, unsigned int min_slop )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    query->priv->min_slop = min_slop;

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_phrase_query_min_slop( lcn_query_t* query, unsigned int *min_slop  )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    *min_slop = query->priv->min_slop;

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_phrase_query_slop_set( lcn_query_t* query, unsigned int slop )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    query->priv->slop = slop;

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_phrase_query_slop( lcn_query_t* query, unsigned int *slop  )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    *slop = query->priv->slop;

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_phrase_query_preserve_term_order_set( lcn_query_t* query, lcn_bool_t do_preserve )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    query->priv->preserve_term_order = do_preserve;

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_phrase_query_preserve_term_order( lcn_query_t* query, lcn_bool_t *preserve_term_order )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    *preserve_term_order = query->priv->preserve_term_order;

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_phrase_query_term_arrays( lcn_query_t* query, lcn_list_t **term_arrays )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    *term_arrays = query->priv->term_arrays;

    return APR_SUCCESS;
}

apr_status_t
lcn_multi_phrase_query_get_field_name( lcn_query_t* query, const char **field_name )
{
    if ( LCN_QUERY_TYPE_MULTIPHRASE != query->type )
    {
        return LCN_ERR_UNSUPPORTED_OPERATION;
    }

    *field_name = query->priv->field;

    return APR_SUCCESS;
}


apr_status_t
lcn_multi_phrase_query_create( lcn_query_t** query,
                               apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_query_private_t* p;

        LCNCE( lcn_query_create( query, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_query_private_t ) ),
               APR_ENOMEM );

        LCNCE( lcn_list_create( &(p->term_arrays), 10, pool ) );
        LCNCE( lcn_list_create( &(p->positions), 10, pool ) );

        (*query)->pool          = pool;
        (*query)->priv          = p;
        (*query)->to_string     = lcn_mpq_to_string;
        (*query)->create_weight = lcn_mpq_create_weight;
        (*query)->rewrite       = lcn_mpq_rewrite;
        (*query)->clone         = lcn_mpq_clone;

        (*query)->type          = LCN_QUERY_TYPE_MULTIPHRASE;

        (*query)->priv->preserve_term_order   = LCN_FALSE;
        (*query)->priv->min_slop              = 0;
    }
    while( FALSE );

    return s;
}
