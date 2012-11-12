#include "lcn_search.h"
#include "lcn_index.h"
#include "weight.h"
#include "query.h"
#include "scorer.h"
#include "lcn_bitvector.h"


/**
 * It is possible to make a filtered query from a term query
 * in the case that the term is a fixed sized field
 */
static apr_status_t
lcn_term_query_rewrite( lcn_query_t* query,
                        lcn_query_t** result,
                        lcn_index_reader_t* reader,
                        apr_pool_t* pool )
{
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_fs_field_t *fs_field;

        if ( NULL != reader &&
             APR_SUCCESS == lcn_index_reader_get_fs_field( reader, &fs_field,
                                                           lcn_term_field( query->term )))
        {
            lcn_bitvector_t *bv;
            lcn_query_t *q;

            LCNCE( lcn_bitvector_create_by_int_fs_field ( &bv,
                                                          fs_field,
                                                          atoi(lcn_term_text( query->term )),
                                                          pool ));
            LCNCE( lcn_filtered_query_create( &q, bv, pool ));

            *result = q;
        }
        else
        {
            *result = query;
        }
    }
    while(0);

    return s;
}

static float
lcn_term_weight_sum_of_squared_weights( lcn_weight_t* w )
{
    w->query_weight = w->idf * lcn_query_boost( w->query );
    return ( w->query_weight * w->query_weight );
}

static void
lcn_term_weight_normalize( lcn_weight_t* w, float n )
{
    w->query_norm = n;
    w->query_weight *= n;
    w->value = w->query_weight * w->idf;
}

static float
lcn_term_weight_value_get( lcn_weight_t* w )
{
    return w->value;
}

static apr_status_t
lcn_term_weight_scorer( lcn_weight_t* w,
                        lcn_scorer_t** scorer,
                        lcn_index_reader_t* r,
                        apr_pool_t* pool )
{
    apr_status_t s;
    lcn_byte_array_t* norms;
    lcn_term_docs_t* term_docs;

    if ( w->query->type == LCN_QUERY_TYPE_TERM_POS )
    {
        LCNCR( lcn_index_reader_term_positions_by_term( r,
                                                        &term_docs,
                                                        w->query->term,
                                                        pool ));
    }
    else
    {
        s = lcn_index_reader_term_docs_from( r,
                                             &term_docs,
                                             w->query->term,
                                             pool );

        /* let it fail while calling .._scorer_next  */
        /* to produce a hit list of length 0         */

        if ( LCN_ERR_TERM_INFOS_READER_CANNOT_SCAN_TO_TERM == s ||
             LCN_ERR_SCAN_ENUM_NO_MATCH  == s )
        {
            s = APR_SUCCESS;
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

    if( w->query->type == LCN_QUERY_TYPE_TERM_POS )
    {
	LCNCR( lcn_term_pos_scorer_create( scorer,
					   w,
					   term_docs,
					   w->similarity,
					   norms,
					   lcn_weight_query( w )->pos,
					   pool ) );
    }
    else
    {
	LCNCR( lcn_term_scorer_create( scorer,
				       w,
				       term_docs,
				       w->similarity,
				       norms,
				       pool ) );
    }
    return s;
}

apr_status_t
lcn_term_weight_create( lcn_weight_t** weight,
                        lcn_searcher_t* searcher,
                        lcn_query_t* term_query,
                        apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        unsigned int max_doc, doc_freq;

        LCNPV( *weight = apr_pcalloc( pool, sizeof( lcn_weight_t ) ), APR_ENOMEM );

        LCNCE( lcn_similarity_clone( lcn_searcher_similarity( searcher ),
                                     &((*weight)->similarity ),
                                     pool ) );

        max_doc = lcn_searcher_max_doc( searcher );

        LCNCE( lcn_searcher_doc_freq( searcher, term_query->term, &doc_freq ) );

        (*weight)->idf = 1.0;

        if( lcn_searcher_has_norms( searcher, lcn_term_field( term_query->term )))
        {
            (*weight)->idf = lcn_similarity_idf( (*weight)->similarity,
                                                 doc_freq,
                                                 max_doc );
        }

        (*weight)->sum_of_squared_weights = lcn_term_weight_sum_of_squared_weights;
        (*weight)->scorer    = lcn_term_weight_scorer;
        (*weight)->normalize = lcn_term_weight_normalize;
        (*weight)->value_get = lcn_term_weight_value_get;
        (*weight)->query     = term_query;
    }
    while( FALSE );

    return s;
}

static apr_status_t
lcn_term_query_extract_terms( lcn_query_t* query,
                              lcn_list_t* terms )
{
    apr_status_t s;
    lcn_term_t* clone;
    LCNCR( lcn_term_clone( query->term, &clone, lcn_list_pool( terms ) ) );
    lcn_list_add( terms, clone );
    return s;
}

static apr_status_t
lcn_term_query_to_string( lcn_query_t* query,
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
            LCNCE( lcn_string_buffer_append( sb, lcn_term_field( query->term ) ) );
            LCNCE( lcn_string_buffer_append( sb, ":" ) );
        }

        LCNCE( lcn_string_buffer_append( sb, lcn_term_text( query->term ) ) );
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

lcn_term_t*
lcn_term_query_term( lcn_query_t* query )
{
    return query->term;
}

apr_status_t
lcn_term_query_create_weight( lcn_query_t* query,
                              lcn_weight_t** weight,
                              lcn_searcher_t* searcher,
                              apr_pool_t* pool )
{
    return lcn_term_weight_create( weight, searcher, query, pool );
}

apr_status_t
lcn_term_query_clone( lcn_query_t* query,
                      lcn_query_t** clone,
                      apr_pool_t* pool )
{
    apr_status_t s;

    LCNCR( lcn_term_query_create( clone, query->term, pool ));
    (*clone)->boost     = query->boost;
    (*clone)->stop_term = query->stop_term;

    return s;
}

apr_status_t
lcn_term_query_create( lcn_query_t** query,
                       const lcn_term_t* term,
                       apr_pool_t* pool )
{
    apr_status_t s;

    do
    {
        LCNCE( lcn_query_create( query, pool ) );
        LCNCE( lcn_term_clone( term, &((*query)->term), pool ) );

        (*query)->type = LCN_QUERY_TYPE_TERM;
        (*query)->stop_term = LCN_FALSE;
        (*query)->extract_terms = lcn_term_query_extract_terms;
        (*query)->create_weight = lcn_term_query_create_weight;
        (*query)->to_string     = lcn_term_query_to_string;
        (*query)->rewrite       = lcn_term_query_rewrite;
        (*query)->clone         = lcn_term_query_clone;
        (*query)->boost         = 1.0f;
        (*query)->pool          = pool;
    }
    while( FALSE );

    return s;
}

lcn_bool_t
lcn_term_query_is_stop_term( lcn_query_t* query )
{
    if ( LCN_QUERY_TYPE_TERM == query->type &&
         LCN_TRUE == query->stop_term )
    {
        return LCN_TRUE;
    }

    return LCN_FALSE;
}


apr_status_t
lcn_stop_term_query_create( lcn_query_t** query,
                            const lcn_term_t* term,
                            apr_pool_t* pool )
{
    apr_status_t s = lcn_term_query_create( query, term, pool );

    (*query)->stop_term = LCN_TRUE;

    return s;
}

apr_status_t
lcn_term_query_create_by_chars( lcn_query_t** query,
                                const char* field,
                                const char* text,
                                apr_pool_t* pool )
{
    apr_status_t s;
    apr_pool_t* child_pool = NULL;

    do
    {
        lcn_term_t* term;

        LCNCE( apr_pool_create( &child_pool, pool ) );
        LCNCE( lcn_term_create( &term, field, text, LCN_TERM_TEXT_COPY,child_pool ) );
        LCNCE( lcn_term_query_create( query, term, pool ) );
    }
    while( FALSE );

    if ( NULL != child_pool )
    {
        apr_pool_destroy( child_pool );
    }

    return s;
}

