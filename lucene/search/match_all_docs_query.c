#include "lcn_search.h"
#include "lcn_index.h"
#include "weight.h"
#include "query.h"
#include "scorer.h"

struct lcn_scorer_private_t
{
    lcn_index_reader_t* reader;
    unsigned int id;
    unsigned int next_id;
    unsigned int max_id;
    lcn_score_t score;
};

static apr_status_t
lcn_match_all_docs_query_extract_terms( lcn_query_t* query,
                                        lcn_list_t* terms )
{
    return APR_SUCCESS;
}

static apr_status_t
lcn_match_all_docs_query_to_string( lcn_query_t* query,
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

        LCNCE( lcn_string_buffer_append( sb, "MatchAllDocsQuery" ) );
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
lcn_match_all_docs_query_clone( lcn_query_t* query,
                                lcn_query_t** clone,
                                apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_match_all_docs_query_create( clone, pool ));
    (*clone)->boost = query->boost;
    return s;
}

static unsigned int
lcn_match_all_scorer_doc( lcn_scorer_t* scorer )
{
    return scorer->priv->id;
}

static apr_status_t
lcn_match_all_scorer_next( lcn_scorer_t* scorer )
{

    lcn_scorer_private_t *p = scorer->priv;

    while( p->id < p->max_id )
    {
        p->id = p->next_id++;

        if ( ! lcn_index_reader_is_deleted( p->reader, p->id ))
        {
            return APR_SUCCESS;
        }
    }

    return LCN_ERR_NO_SUCH_DOC;
}

static apr_status_t
lcn_match_all_scorer_skip_to( lcn_scorer_t* scorer,
                              unsigned int target )
{
    scorer->priv->id = ( target > 0 ? target - 1 : 0 );
    return lcn_match_all_scorer_next( scorer );
}

static apr_status_t
lcn_match_all_scorer_score_get( lcn_scorer_t* scorer, lcn_score_t* score )
{
    *score = scorer->priv->score;
    return APR_SUCCESS;
}

static apr_status_t
lcn_match_all_scorer( lcn_weight_t* w,
                      lcn_scorer_t** scorer,
                      lcn_index_reader_t* r,
                      apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        lcn_scorer_private_t* p;

        LCNCE( lcn_default_scorer_create( scorer, w->similarity, pool ) );

        LCNPV( p = apr_pcalloc( pool, sizeof( lcn_scorer_private_t ) ),
               APR_ENOMEM );

        p->reader = r;
        p->id      = 0;
        p->next_id = 0;
        p->max_id = lcn_index_reader_max_doc( r );

        if ( p->max_id > 0 )
        {
            p->max_id--;
        }

        p->score.float_val = lcn_weight_value( w );

        (*scorer)->doc          = lcn_match_all_scorer_doc;
        (*scorer)->next         = lcn_match_all_scorer_next;
        (*scorer)->score_get    = lcn_match_all_scorer_score_get;
	(*scorer)->skip_to      = lcn_match_all_scorer_skip_to;

	(*scorer)->type    = "match_all_docs_scorer";

        (*scorer)->priv         = p;
    }
    while(0);

    return s;
}

static float
lcn_match_all_docs_weight_sum_of_squared_weights( lcn_weight_t* w )
{
    w->query_weight = lcn_query_boost( w->query );
    w->value        = w->query_weight;
    return ( w->query_weight * w->query_weight );
}

static void
lcn_match_all_docs_weight_normalize( lcn_weight_t* w, float n )
{
    w->query_norm = n;
    w->query_weight *= n;
    w->value = w->query_weight;
}

static float
lcn_match_all_docs_weight_value_get( lcn_weight_t* w )
{
    return w->query_weight;
}


apr_status_t
lcn_match_all_docs_weight_create( lcn_weight_t** weight,
                                  lcn_searcher_t* searcher,
                                  lcn_query_t* match_all_docs_query,
                                  apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNPV( *weight = apr_pcalloc( pool, sizeof( lcn_weight_t ) ),
               APR_ENOMEM );

        LCNCE( lcn_similarity_clone( lcn_searcher_similarity( searcher ),
                                     &((*weight)->similarity ),
                                     pool ) );

        (*weight)->sum_of_squared_weights
            = lcn_match_all_docs_weight_sum_of_squared_weights;

        (*weight)->scorer    = lcn_match_all_scorer;
        (*weight)->value_get = lcn_match_all_docs_weight_value_get;
        (*weight)->normalize = lcn_match_all_docs_weight_normalize;
        (*weight)->query     = match_all_docs_query;
    }
    while( FALSE );

    return s;
}

apr_status_t
lcn_match_all_docs_query_create_weight( lcn_query_t* query,
                                        lcn_weight_t** weight,
                                        lcn_searcher_t* searcher,
                                        apr_pool_t* pool )
{
    return lcn_match_all_docs_weight_create( weight, searcher, query, pool );
}


apr_status_t
lcn_match_all_docs_query_create( lcn_query_t** query,
                                 apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_query_create( query, pool ) );

        (*query)->type = LCN_QUERY_TYPE_MATCH_ALL_DOCS;

        (*query)->extract_terms = lcn_match_all_docs_query_extract_terms;
        (*query)->create_weight = lcn_match_all_docs_query_create_weight;
        (*query)->to_string     = lcn_match_all_docs_query_to_string;
        (*query)->clone         = lcn_match_all_docs_query_clone;
        (*query)->boost         = 1.0f;
        (*query)->pool          = pool;
    }
    while( FALSE );

    return s;
}

