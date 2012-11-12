#include "query.h"
#include "../query_parser/query_tokenizer.h"

BEGIN_C_DECLS

static apr_status_t
lcn_default_query_rewrite(  lcn_query_t* query,
                            lcn_query_t** result,
                            lcn_index_reader_t* reader,
                            apr_pool_t* pool )
{
    *result = query;
    return APR_SUCCESS;
}

static apr_status_t
lcn_default_query_clone( lcn_query_t* query,
                         lcn_query_t** clone,
                         apr_pool_t* pool )
{
    *clone = query;
    return APR_SUCCESS;
}

apr_status_t
lcn_query_extract_terms( lcn_query_t* query,
                         lcn_list_t* terms )
{
    return query->extract_terms( query, terms );
}

apr_status_t
lcn_query_weight( lcn_query_t* query,
                  lcn_weight_t** weight,
                  lcn_searcher_t* searcher,
                  apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_query_t* rewritten_query;
        float sum, norm;
        lcn_similarity_t* similarity;

#if 0
        if( APR_SUCCESS !=
            ( s = lcn_query_rewrite( query,
                                     &rewritten_query,
                                     lcn_index_searcher_reader( searcher ),
                                     pool ) ) )
#endif
        if( APR_SUCCESS !=
            ( s = lcn_index_searcher_rewrite( searcher,
                                              query,
                                              &rewritten_query,
                                              pool ) ) )
        {
            if( LCN_ERR_EMPTY_QUERY == s )
            {
                break;
            }
            LCNCE( s );
        }

        LCNCE( lcn_query_create_weight( rewritten_query,
                                        weight,
                                        searcher,
                                        pool ) );

        similarity = lcn_searcher_similarity( searcher );
        sum = lcn_weight_sum_of_squared_weights( *weight );
        norm = lcn_similarity_query_norm( similarity, sum );
        lcn_weight_normalize( *weight, norm );
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_query_create_weight( lcn_query_t* query,
                         lcn_weight_t** weight,
                         lcn_searcher_t* searcher,
                         apr_pool_t* pool )
{
    return query->create_weight( query, weight, searcher, pool );
}

float
lcn_query_boost( lcn_query_t* query )
{
    return query->boost;
}

void
lcn_query_boost_set( lcn_query_t* query, float boost )
{
    query->boost = boost;
}

apr_status_t
lcn_query_to_string( lcn_query_t* query,
                     char** result,
                     const char* field,
                     apr_pool_t* pool )
{
    return query->to_string( query, result, field, pool );
}


apr_status_t
lcn_query_rewrite( lcn_query_t* query,
                   lcn_query_t** result,
                   lcn_index_reader_t* reader,
                   apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;

    if ( APR_SUCCESS == (s = query->rewrite( query, result, reader, pool )) &&
         query != *result )
    {
        lcn_query_set_name( *result, query->name );
    }

    return s;
}

static
void lcn_query_base_clone( lcn_query_t* query,
                           lcn_query_t* clone,
                           apr_pool_t *pool )
{
    clone->pool = pool;

    if ( NULL != query->name )
    {
        clone->name = apr_pstrdup( clone->pool, query->name );
    }
}

void
lcn_query_set_name( lcn_query_t *query,
                    const char *name )
{
    if ( NULL == name )
    {
        query->name = NULL;
    }
    else
    {
        query->name = apr_pstrdup( query->pool, name );
    }
}

char *
lcn_query_name( lcn_query_t *query )
{
    return query->name;
}

apr_status_t
lcn_query_clone( lcn_query_t* query,
                 lcn_query_t** clone,
                 apr_pool_t* pool )
{
    lcn_query_t *result;
    apr_status_t s = query->clone( query, &result, pool );
    lcn_query_base_clone( query, result, pool );
    *clone = result;
    return s;
}

lcn_query_type_t
lcn_query_type( lcn_query_t* query )
{
    return query->type;
}

const char*
lcn_query_type_string( lcn_query_t* query )
{
    switch( query->type )
    {
    case LCN_QUERY_TYPE_MATCH_ALL_DOCS:
        return "match_all_docs_query";
    case LCN_QUERY_TYPE_BOOLEAN:
        return "boolean_query";
    case LCN_QUERY_TYPE_TERM:
        return "term_query";
    case LCN_QUERY_TYPE_MULTIPHRASE:
        return "multi_phrase_query";
    case LCN_QUERY_TYPE_PREFIX:
        return "prefix_query";
    case LCN_QUERY_TYPE_TERM_POS:
        return "term_pos_query";
    case LCN_QUERY_TYPE_ORDERED:
        return "ordered_query";
    case LCN_QUERY_TYPE_ONE_HIT:
        return "one_hit_query;";
    case LCN_QUERY_TYPE_DEFAULT:
        return "uninitialized default_query - DANGER!!";
    }

    return "";
}

apr_status_t
lcn_query_create( lcn_query_t** query, apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *query = apr_pcalloc( pool, sizeof( lcn_query_t ) ),
               APR_ENOMEM );

        (*query)->type    = LCN_QUERY_TYPE_DEFAULT;
        (*query)->rewrite = lcn_default_query_rewrite;
        (*query)->clone   = lcn_default_query_clone;
        (*query)->boost   = 1.0f;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_parse_query( lcn_query_t**query,
                 const char *qstring,
                 apr_pool_t *pool )
{
    apr_status_t s = APR_SUCCESS;
    apr_pool_t *cp = NULL;

    do
    {
        lcn_query_tokenizer_t *t;
        lcn_query_info_t *info;
        void *parser;
        int token_id;
        lcn_query_token_t token;

        *query = NULL;
        LCNASSERT( NULL != qstring && strlen(qstring) > 0, LCN_ERR_EMPTY_QUERY );
        LCNCE( apr_pool_create( &cp, pool ));

#if 0
        lcn_query_parser_trace( stderr, "TRACE: ");
#endif

        info = (lcn_query_info_t*) apr_pcalloc( cp, sizeof( lcn_query_info_t ));
        info->pool = pool;
        info->status = APR_SUCCESS;
        LCNCE( lcn_query_tokenizer_create( &t, apr_pstrdup(pool,qstring), pool ));
        parser = lcn_query_parser_alloc( malloc );

        while ( APR_SUCCESS == lcn_query_tokenizer_next_token( t, &token_id, &token ) )
        {
            lcn_query_token_t *t;

            if ( token_id == LCNQ_EOS )
            {
                s = LCN_ERR_QUERY_PARSER_SYNTAX_ERROR;
                break;
            }

            t = (lcn_query_token_t*) apr_pcalloc( pool, sizeof(lcn_query_token_t));
            t->token_id = token_id;
            t->text = apr_pstrdup( pool, token.text );
            lcn_query_parser_parse( parser, token_id, t, info );
            LCNCE( info->status );
        }

        LCNCE( s );
        lcn_query_parser_parse( parser, 0, &token, info );
        LCNCE( info->status );

        lcn_query_parser_free( parser, free );

        *query = info->query;
    }
    while(0);

    if ( APR_SUCCESS != s && NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

END_C_DECLS
