#include "test_all.h"
#include "query.h"
#include "lcn_index.h"

static const char* test_index_name = "test_index_2";

void
test_single_prefix_char( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* query;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );


    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_prefix_query_create_by_chars( &query, "text", "a",
                                                pool ) );

    LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );
    CuAssertIntEquals( tc, 184, lcn_hits_length( hits ) );


    apr_pool_destroy( pool );
}

void
test_prefix_query( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* query;
    lcn_hits_t* hits;
    char* query_str;

    apr_pool_create( &pool, main_pool );


    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_prefix_query_create_by_chars( &query, "text", "clas",
                                                pool ) );

    LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );
    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );
    CuAssertStrEquals( tc, "text:clas*", query_str );
    CuAssertIntEquals( tc, 11, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "text",
               "9. Added class RemoteSearchable, "
               "providing support for remote" );
    CHECK_HIT( hits, 1, "text", "test class TestPhrasePrefixQuery provides "
               "the usage example." );
    CHECK_HIT( hits, 2,  "id", "KW211" );
    CHECK_HIT( hits, 3,  "id", "KW104" );
    CHECK_HIT( hits, 4,  "id", "KW132" );
    CHECK_HIT( hits, 5,  "id", "KW67" );
    CHECK_HIT( hits, 6,  "id", "KW100" );
    CHECK_HIT( hits, 7,  "id", "KW150" );
    CHECK_HIT( hits, 8,  "id", "KW168" );
    CHECK_HIT( hits, 9,  "id", "KW184" );
    CHECK_HIT( hits, 10, "id", "KW314" );
    CHECK_HIT( hits, 10, "text", "complex classes of terms, "
               "including numbers, acronyms, email" );
    apr_pool_destroy( pool );
}

void
test_empty_prefix( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* query;
    lcn_hits_t* hits;
    char* query_str;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_prefix_query_create_by_chars( &query, "id", "", pool ) );

    LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );
    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );
    CuAssertStrEquals( tc, "id:*", query_str );
    CuAssertIntEquals( tc, 331, lcn_hits_length( hits ) );
    apr_pool_destroy( pool );
}

CuSuite* make_prefix_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_prefix_query );
    SUITE_ADD_TEST( s, test_single_prefix_char );
    SUITE_ADD_TEST( s, test_empty_prefix );

    return s;
}
