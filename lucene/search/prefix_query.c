#include "query.h"
#include "lcn_search.h"
#include "lcn_util.h"

#define LNC_PREFIX_QUERY_MAX_HITS (500000)
#define LCN_PREFIX_QUERY_MAX_TERMS (1000)

struct lcn_query_private_t
{
    lcn_term_t* prefix;
    unsigned int max_terms;
};

static apr_status_t
lcn_prefix_query_clone( lcn_query_t* prefix_query,
                        lcn_query_t** clone,
                        apr_pool_t* pool )
{
    apr_status_t s;

    lcn_query_private_t* p = prefix_query->priv;
    lcn_query_t* result;

    LCNCR( lcn_prefix_query_create ( &result, p->prefix, pool ) );
    result->boost = prefix_query->boost;

    *clone = result;

    return s;
}

static apr_status_t
lcn_prefix_query_rewrite( lcn_query_t* query,
                          lcn_query_t** rewritten_query,
                          lcn_index_reader_t* reader,
                          apr_pool_t* pool )
{
    apr_status_t s;
    apr_pool_t* cp = NULL;

    do
    {
        lcn_list_t* terms;
        unsigned int i;

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNCE( lcn_boolean_query_create( rewritten_query, pool ) );

        (*rewritten_query)->boost = query->boost;

        LCNCE( lcn_prefix_query_terms( query, &terms, reader, cp ) );

        if ( 0 == lcn_list_size( terms ) )
        {
            lcn_query_private_t* p = query->priv;

            LCNCE( lcn_term_query_create( rewritten_query, p->prefix, pool ) );
        }
        else
        {
            for( i = 0; i < lcn_list_size( terms ); i++ )
            {
                lcn_query_t* term_query;

                const lcn_term_t* cur_term = (const lcn_term_t* )lcn_list_get( terms, i );
                LCNCE( lcn_term_query_create( &term_query, cur_term, cp ) );
                LCNCE( lcn_boolean_query_add( *rewritten_query, term_query, LCN_BOOLEAN_CLAUSE_SHOULD ) );
            }
        }
    }
    while( 0 );

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

lcn_term_t*
lcn_prefix_query_prefix( lcn_query_t* query )
{
    return query->priv->prefix;
}

void
lcn_prefix_query_max_terms_set( lcn_query_t* query,
                                unsigned int max )
{
    query->priv->max_terms = max;
}

unsigned int
lcn_prefix_query_max_terms( const lcn_query_t* query )
{
    return query->priv->max_terms;
}

apr_status_t
lcn_prefix_query_terms( lcn_query_t* query,
                        lcn_list_t** terms,
                        lcn_index_reader_t* reader,
                        apr_pool_t* pool )
{
    apr_status_t s;
    const char* field;
    const char* text;
    unsigned int n_terms = 0;

    do
    {
        lcn_term_enum_t* te;
        lcn_query_private_t* p = query->priv;
        unsigned int sum = 0;

        LCNCE( lcn_list_create( terms, 100, pool ) );

        LCNPV( field = lcn_term_field( p->prefix ), LCN_ERR_NULL_PTR );
        LCNPV( text  = lcn_term_text( p->prefix ), LCN_ERR_NULL_PTR );

        LCNCE( lcn_index_reader_terms_from( reader, &te, p->prefix, pool ) );

        if( NULL == te )
        {
            break;
        }

        while( APR_SUCCESS == ( lcn_term_enum_next( te ) ) &&
               ++n_terms < p->max_terms )
        {
            const char* cur_text;
            lcn_term_t* cur_term;

            LCNCE( lcn_term_clone( (lcn_term_t*)lcn_term_enum_term( te ),
                                   &cur_term,
                                   pool ) );

            LCNPV( cur_text = lcn_term_text( cur_term ), LCN_ERR_NULL_PTR );

            if(  strcmp( field, lcn_term_field( cur_term ) ) != 0 ||
                 ( *text != '\0' && !lcn_string_starts_with( cur_text, text )) )
            {
                break;
            }

            LCNCE( lcn_list_add( *terms, cur_term ) );

            if ( (sum += lcn_term_enum_doc_freq( te )) >= LNC_PREFIX_QUERY_MAX_HITS )
            {
                break;
            }
        }

    }
    while( 0 );

    return s;
}



static apr_status_t
lcn_prefix_query_to_string( lcn_query_t* query,
                            char** result, const char* field,
                            apr_pool_t* pool )
{
    apr_pool_t* cp;
    apr_status_t s;
    const char* q_field;
    lcn_string_buffer_t* sb;
    lcn_query_private_t* p = query->priv;

    LCNCR( apr_pool_create( &cp, pool ) );
    LCNCR( lcn_string_buffer_create( &sb, cp ) );

    if( strcmp( q_field = lcn_term_field( p->prefix ), field ) != 0 )
    {
        lcn_string_buffer_append( sb, q_field );
        lcn_string_buffer_append( sb, ":" );
    }

    LCNCR( lcn_string_buffer_append( sb, lcn_term_text( p->prefix ) ) );
    LCNCR( lcn_string_buffer_append( sb, "*" ) );

    LCNCR( lcn_string_buffer_to_string( sb, result, pool ) );

    apr_pool_destroy( cp );

    return s;
}

apr_status_t
lcn_prefix_query_create( lcn_query_t** query,
                         const lcn_term_t* prefix,
                         apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_query_private_t* p;

        LCNCE( lcn_query_create( query, pool ) );
        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_query_private_t ) ),
               APR_ENOMEM );

        LCNCE( lcn_term_clone( prefix, &(p->prefix), pool ) );

        p->max_terms = LCN_PREFIX_QUERY_MAX_TERMS;

        (*query)->type      = LCN_QUERY_TYPE_PREFIX;
        (*query)->rewrite   = lcn_prefix_query_rewrite;
        (*query)->to_string = lcn_prefix_query_to_string;
        (*query)->clone     = lcn_prefix_query_clone;
        (*query)->priv      = p;
        (*query)->pool      = pool;
    }
    while( 0 );

    return s;
}

apr_status_t
lcn_prefix_query_create_by_chars( lcn_query_t** query,
                                  const char* field,
                                  const char* prefix,
                                  apr_pool_t* pool )
{
    apr_status_t s;
    lcn_term_t* prefix_term;
    LCNCR( lcn_term_create( &prefix_term, field, prefix, LCN_TRUE, pool ) );
    return lcn_prefix_query_create( query, prefix_term, pool );
}
