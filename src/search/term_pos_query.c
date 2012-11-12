#include "lcn_search.h"
#include "lcn_index.h"
#include "weight.h"
#include "query.h"
#include "scorer.h"

static float
lcn_term_pos_weight_sum_of_squared_weights( lcn_weight_t* w )
{
    w->query_weight = w->idf * lcn_query_boost( w->query );
    return ( w->query_weight * w->query_weight );
}

static void
lcn_term_pos_weight_normalize( lcn_weight_t* w, float n )
{
    w->query_norm = n;
    w->query_weight *= n;
    w->value = w->query_weight * w->idf;
}

static float
lcn_term_pos_weight_value_get( lcn_weight_t* w )
{
    return w->value;
}

static apr_status_t
lcn_term_pos_weight_scorer( lcn_weight_t* w,
                            lcn_scorer_t** scorer,
                            lcn_index_reader_t* r,
                            apr_pool_t* pool )
{
    apr_status_t s;
    lcn_byte_array_t* norms;
    lcn_term_docs_t* term_docs;

    *scorer = NULL;

    if( ( s = lcn_index_reader_term_positions_by_term( r,
                                                       &term_docs,
                                                       w->query->term,
                                                       pool ) ) )
    {
        if( LCN_ERR_SCAN_ENUM_NO_MATCH == s )
        {
            lcn_scorer_t* result;
            LCNCR( lcn_non_matching_scorer_create( &result, pool ) );
            *scorer = result;
            return APR_SUCCESS;
        }
        LCNCR( s );
    }

    s = lcn_index_reader_norms( r, &norms, lcn_term_field( w->query->term ) );

    if ( LCN_ERR_NORMS_NOT_FOUND == s )
    {
        norms = NULL;
    }
    else if ( APR_SUCCESS != s )
    {
        return s;
    }

    LCNCR( lcn_term_pos_scorer_create( scorer,
                                       w,
                                       term_docs,
                                       w->similarity,
                                       norms,
                                       lcn_weight_query( w )->pos,
                                       pool ) );
    return s;
}

apr_status_t
lcn_term_pos_weight_create( lcn_weight_t** weight,
                        lcn_searcher_t* searcher,
                        lcn_query_t* term_pos_query,
                        apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int max_doc, doc_freq;

        LCNPV( *weight = apr_pcalloc( pool, sizeof( lcn_weight_t ) ),
               APR_ENOMEM );

        LCNCE( lcn_similarity_clone( lcn_searcher_similarity( searcher ),
                                     &((*weight)->similarity ),
                                     pool ) );

        max_doc = lcn_searcher_max_doc( searcher );

        LCNCE( lcn_searcher_doc_freq( searcher, term_pos_query->term, &doc_freq ) );

        (*weight)->idf = lcn_similarity_idf( (*weight)->similarity,
                                             doc_freq,
                                             max_doc );

        (*weight)->sum_of_squared_weights
            = lcn_term_pos_weight_sum_of_squared_weights;
        (*weight)->scorer    = lcn_term_pos_weight_scorer;
        (*weight)->normalize = lcn_term_pos_weight_normalize;
        (*weight)->value_get = lcn_term_pos_weight_value_get;
        (*weight)->query     = term_pos_query;
    }
    while( FALSE );

    return s;
}


static apr_status_t
lcn_term_pos_query_to_string( lcn_query_t* query,
                              char** result,
                              const char* field,
                              apr_pool_t* pool )
{
    apr_status_t s;
    apr_pool_t* cp = NULL;

    do
    {
        char boost_buf[20];
        lcn_string_buffer_t* sb;

        lcn_to_string_boost( boost_buf, 20, lcn_query_boost( query ) );

        LCNCE( apr_pool_create( &cp, pool ) );
        LCNCE( lcn_string_buffer_create( &sb, cp ) );

        if( strcmp( lcn_term_field( query->term ), field ) != 0 )
        {
            LCNCE( lcn_string_buffer_append( sb,
                                             lcn_term_field( query->term ) ) );
            LCNCE( lcn_string_buffer_append( sb, ":" ) );
        }

        LCNCE( lcn_string_buffer_append( sb, lcn_term_text( query->term ) ) );
        LCNCE( lcn_string_buffer_append( sb, "[" ));
        LCNCE( lcn_string_buffer_append_int( sb, query->pos ));
        LCNCE( lcn_string_buffer_append( sb, "]" ));
        LCNCE( lcn_string_buffer_append( sb, boost_buf ) );
        LCNCE( lcn_string_buffer_to_string( sb, result, pool ) );
    }
    while( FALSE );

    if( NULL != cp )
    {
        apr_pool_destroy( cp );
    }

    return s;
}

apr_status_t
lcn_term_pos_query_create_weight( lcn_query_t* query,
                              lcn_weight_t** weight,
                              lcn_searcher_t* searcher,
                              apr_pool_t* pool )
{
    return lcn_term_pos_weight_create( weight, searcher, query, pool );
}

apr_status_t
lcn_term_pos_query_clone( lcn_query_t* query,
                          lcn_query_t** clone,
                          apr_pool_t* pool )
{
    return lcn_term_pos_query_create( clone, query->term, query->pos, pool );
}


apr_status_t
lcn_term_pos_query_create( lcn_query_t** query,
                           const lcn_term_t* term,
                           unsigned int pos,
                           apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_t* result;

    LCNCR( lcn_term_query_create( &result, term, pool ) );

    result->type          = LCN_QUERY_TYPE_TERM_POS;
    result->to_string     = lcn_term_pos_query_to_string;
    result->create_weight = lcn_term_pos_query_create_weight;
    result->to_string     = lcn_term_pos_query_to_string;
    result->clone         = lcn_term_pos_query_clone;
    result->pos           = pos;

    *query = result;

    return s;
}


apr_status_t
lcn_term_pos_query_create_by_chars( lcn_query_t** query,
                                    const char* field,
                                    const char* text,
                                    unsigned int pos,
                                    apr_pool_t* pool )
{
    apr_status_t s;
    lcn_query_t* result;
    lcn_term_t* term;

    LCNCR( lcn_term_create( &term, field, text, LCN_TRUE, pool ) );
    LCNCR( lcn_term_pos_query_create( &result, term, pos, pool ) );

    *query = result;

    return s;
}
