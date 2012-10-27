#include "test_all.h"
#include "lcn_search.h"
#include "search/query.h"

#define CHECK_DOC( I, ID );                                             \
    {                                                                   \
        lcn_document_t* doc;                                            \
        char* text;                                                     \
        apr_pool_t* my_cp;                                              \
                                                                        \
        LCN_TEST( apr_pool_create( &my_cp, pool ) );                    \
        LCN_TEST( lcn_hits_doc( hits, &doc, I, my_cp ) );               \
        /*LCN_TEST( lcn_document_get( doc, &text, "text", my_cp ) );    \
          printf( "<%s>\n", text );*/                                   \
        LCN_TEST( lcn_document_get( doc, &text, "xid",  my_cp ) );      \
        CuAssertStrEquals( tc, ID, text );                              \
    }

static void
test_opt_prohib( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    char* query_str;
    lcn_query_t* query, *tquery, *tquery2, *tquery3;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_compat_0",
                                                 pool ) );

    LCN_TEST( lcn_term_query_create_by_chars( &tquery, "volltext", "can", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tquery2,"volltext", "must", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tquery3,"volltext", "it", pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, cp ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery,
                                     LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery2,
                                     LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery3,
                                     LCN_BOOLEAN_CLAUSE_MUST_NOT ) );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc, "volltext:can volltext:must -volltext:it", query_str );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 8, lcn_hits_length( hits ) );

    CHECK_DOC( 0, "265" );
    CHECK_DOC( 1, "70" );
    CHECK_DOC( 2, "105" );
    CHECK_DOC( 3, "172" );
    CHECK_DOC( 4, "299" );
    CHECK_DOC( 5, "21" );
    CHECK_DOC( 6, "45" );
    CHECK_DOC( 7, "130" );

    apr_pool_destroy( pool );
}

static void
test_boolean_query( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    char* query_str;
    lcn_query_t* query, *tquery, *tquery2, *tquery3, *clone;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_compat_0",
                                                 pool ) );

    LCN_TEST( lcn_term_query_create_by_chars( &tquery, "volltext", "it", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tquery2,"volltext", "the", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tquery3,"volltext", "is", pool ) );
    LCN_TEST( lcn_boolean_query_create( &query, cp ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery,
                                     LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery2,
                                     LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery3,
                                     LCN_BOOLEAN_CLAUSE_MUST_NOT ) );
    LCN_TEST( lcn_query_clone( query, &clone, pool ) );


    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc, "+volltext:it +volltext:the -volltext:is", query_str );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 4, lcn_hits_length( hits ) );

    CHECK_DOC( 0, "324" );
    CHECK_DOC( 1, "69" );
    CHECK_DOC( 2, "97" );
    CHECK_DOC( 3, "157" );

    apr_pool_destroy( pool );
}

CuSuite* make_compatibility_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_boolean_query );
    SUITE_ADD_TEST( s, test_opt_prohib );

    return s;
}
