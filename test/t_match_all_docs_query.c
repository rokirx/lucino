#include "test_all.h"
#include "lcn_search.h"
#include "lcn_index.h"
#include "lcn_analysis.h"

static void
test_match_all_docs_query_impl( CuTest* tc, const char *index_dir )
{
    apr_pool_t* pool;
    unsigned int i;
    lcn_searcher_t* searcher;
    lcn_query_t* query;
    lcn_hits_t* hits;
    char* query_str;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );

    CuAssertIntEquals(tc, LCN_QUERY_TYPE_MATCH_ALL_DOCS, lcn_query_type( query ));
    CuAssertStrEquals(tc, "match_all_docs_query", lcn_query_type_string( query ));

    /* second param is not used, it is noop function for this query */
    LCN_TEST( lcn_query_extract_terms( query, NULL ));
    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals(tc, "MatchAllDocsQuery", query_str );

    lcn_query_boost_set( query, 1.5f );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );
    CuAssertStrEquals(tc, "MatchAllDocsQuery^1.500000", query_str );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 index_dir,
                                                 pool ) );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits, 
                                   query, 
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 331,  lcn_searcher_max_doc( searcher ) );
    CuAssertIntEquals( tc, 331,  lcn_hits_length( hits ));

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t* doc;
        char* id;

        LCN_TEST( lcn_hits_doc( hits, &doc, i, pool ) );
        LCN_TEST( lcn_document_get( doc, &id, "id", pool ) );

        CuAssertStrEquals(tc, apr_pstrcat(pool, "KW",apr_itoa( pool, i), NULL ), id );
    }

    LCN_TEST( lcn_index_searcher_close( searcher ));

    apr_pool_destroy( pool );
}

static void
test_match_all_docs_query( CuTest* tc )
{
    test_match_all_docs_query_impl( tc, "test_index_2" );
    test_match_all_docs_query_impl( tc, "test_index_1" );
}

static void
test_rewrite( CuTest* tc )
{
    apr_pool_t* pool;
    char* query_str;

    apr_pool_create( &pool, main_pool );

    {
        lcn_query_t *query, *tqy, *bq, *rewritten;

        LCN_TEST( lcn_boolean_query_create( &bq, pool ));

        LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );
        LCN_TEST( lcn_term_query_create_by_chars( &tqy, "f", "y", pool ));

        LCN_TEST( lcn_boolean_query_add( bq, query, LCN_BOOLEAN_CLAUSE_MUST ));
        LCN_TEST( lcn_boolean_query_add( bq, tqy,   LCN_BOOLEAN_CLAUSE_MUST ));

        LCN_TEST( lcn_query_to_string( bq, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "+MatchAllDocsQuery +f:y", query_str );

        LCN_TEST( lcn_query_rewrite( bq, &rewritten, NULL, pool ));

        LCN_TEST( lcn_query_to_string( rewritten, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "+f:y", query_str );
    }

    {
        lcn_query_t *query, *tqy, *bq, *rewritten;

        LCN_TEST( lcn_boolean_query_create( &bq, pool ));

        LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );
        LCN_TEST( lcn_term_query_create_by_chars( &tqy, "f", "y", pool ));

        LCN_TEST( lcn_boolean_query_add( bq, query, LCN_BOOLEAN_CLAUSE_MUST ));
        LCN_TEST( lcn_boolean_query_add( bq, tqy,   LCN_BOOLEAN_CLAUSE_MUST_NOT ));

        LCN_TEST( lcn_query_to_string( bq, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "+MatchAllDocsQuery -f:y", query_str );

        LCN_TEST( lcn_query_rewrite( bq, &rewritten, NULL, pool ));

        LCN_TEST( lcn_query_to_string( rewritten, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "+MatchAllDocsQuery -f:y", query_str );
    }

    {
        lcn_query_t* query, *tqx, *tqy, *bq, *bq1, *rewritten;

        LCN_TEST( lcn_boolean_query_create( &bq1, pool ));

        LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );
        LCN_TEST( lcn_term_query_create_by_chars( &tqy, "f", "y", pool ));

        LCN_TEST( lcn_boolean_query_add( bq1, query, LCN_BOOLEAN_CLAUSE_MUST     ));
        LCN_TEST( lcn_boolean_query_add( bq1, tqy,   LCN_BOOLEAN_CLAUSE_MUST_NOT ));


        LCN_TEST( lcn_boolean_query_create( &bq, pool ));

        LCN_TEST( lcn_term_query_create_by_chars( &tqx, "f", "x", pool ));

        LCN_TEST( lcn_boolean_query_add( bq, tqx, LCN_BOOLEAN_CLAUSE_MUST ));
        LCN_TEST( lcn_boolean_query_add( bq, bq1, LCN_BOOLEAN_CLAUSE_MUST_NOT ));

        LCN_TEST( lcn_query_to_string( bq, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "+f:x -(+MatchAllDocsQuery -f:y)", query_str );

        LCN_TEST( lcn_query_rewrite( bq, &rewritten, NULL, pool ));

        LCN_TEST( lcn_query_to_string( rewritten, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "+f:x -f:y", query_str );
    }

{
        lcn_query_t* query, *tqx, *tqy, *bq, *bq1, *rewritten;

        LCN_TEST( lcn_boolean_query_create( &bq1, pool ));

        LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );
        LCN_TEST( lcn_term_query_create_by_chars( &tqy, "f", "y", pool ));

        LCN_TEST( lcn_boolean_query_add( bq1, tqy,   LCN_BOOLEAN_CLAUSE_MUST_NOT ));
        LCN_TEST( lcn_boolean_query_add( bq1, query, LCN_BOOLEAN_CLAUSE_MUST     ));

        LCN_TEST( lcn_boolean_query_create( &bq, pool ));

        LCN_TEST( lcn_term_query_create_by_chars( &tqx, "f", "x", pool ));

        LCN_TEST( lcn_boolean_query_add( bq, bq1, LCN_BOOLEAN_CLAUSE_MUST_NOT ));
        LCN_TEST( lcn_boolean_query_add( bq, tqx, LCN_BOOLEAN_CLAUSE_MUST ));

        LCN_TEST( lcn_query_to_string( bq, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "-(-f:y +MatchAllDocsQuery) +f:x", query_str );

        LCN_TEST( lcn_query_rewrite( bq, &rewritten, NULL, pool ));

        LCN_TEST( lcn_query_to_string( rewritten, &query_str, "", pool ) );

        CuAssertStrEquals(tc, "-f:y +f:x", query_str );
    }

    apr_pool_destroy( pool );
}


CuSuite* make_match_all_docs_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_match_all_docs_query );
    SUITE_ADD_TEST( s, test_rewrite );

    return s;
}
