#include "test_all.h"
#include "lcn_bitvector.h"

static void
test_bitvector_disjunction( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* t_query1, *t_query2, *b_query;
    lcn_hits_t* hits;
    unsigned int hits_ref, hits_size;
    lcn_bitvector_t* bitvector, *and_bitvector, *result_bitvector;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher, "test_index_2", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &t_query1, "text", "should", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &t_query2, "text", "must", pool ) );
    LCN_TEST( lcn_boolean_query_create( &b_query, pool ) );

    lcn_boolean_query_add( b_query, t_query1, LCN_BOOLEAN_CLAUSE_SHOULD );
    lcn_boolean_query_add( b_query, t_query2, LCN_BOOLEAN_CLAUSE_SHOULD );

    lcn_searcher_search( searcher, &hits, b_query, NULL, pool );

    hits_ref = lcn_hits_length( hits );

    LCN_TEST( lcn_query_bitvector_create( &bitvector, t_query1, searcher, NULL, pool ) );
    LCN_TEST( lcn_query_bitvector_create( &and_bitvector, t_query2, searcher, NULL, pool ) );

    lcn_bitvector_or( &result_bitvector, bitvector, and_bitvector, pool );
    lcn_searcher_search( searcher, &hits, b_query, result_bitvector, pool );

    hits_size = lcn_hits_length( hits );

    CuAssertIntEquals( tc, hits_ref, hits_size );

    apr_pool_destroy( pool );
}
static void
test_bitvector_conjunction( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* t_query1, *t_query2, *b_query;
    lcn_hits_t* hits;
    unsigned int hits_ref, hits_size;
    lcn_bitvector_t* bitvector, *and_bitvector, *result_bitvector;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher, "test_index_2", pool ) );

    LCN_TEST( lcn_term_query_create_by_chars( &t_query1, "text", "can", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &t_query2, "text", "is", pool ) );
    LCN_TEST( lcn_boolean_query_create( &b_query, pool ) );

    lcn_boolean_query_add( b_query, t_query1, LCN_BOOLEAN_CLAUSE_MUST );
    lcn_boolean_query_add( b_query, t_query2, LCN_BOOLEAN_CLAUSE_MUST );

    lcn_searcher_search( searcher, &hits, b_query, NULL, pool );

    hits_ref = lcn_hits_length( hits );

    LCN_TEST( lcn_query_bitvector_create( &bitvector, t_query1, searcher, NULL, pool ) );
    LCN_TEST( lcn_query_bitvector_create( &and_bitvector, t_query2, searcher, NULL, pool ) );

    lcn_bitvector_and( &result_bitvector, bitvector, and_bitvector, pool );

    lcn_searcher_search( searcher, &hits, b_query, result_bitvector, pool );

    hits_size = lcn_hits_length( hits );

    CuAssertIntEquals( tc, hits_ref, hits_size );

    apr_pool_destroy( pool );
}

static void
test_query_bitvector( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* b_query1, *b_query2;
    lcn_hits_t* hits;
    lcn_bitvector_t* bitvector;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher, "test_index_2", pool ) );

    LCN_TEST( lcn_boolean_query_create( &b_query1, pool ) );
    LCN_TEST( lcn_boolean_query_create( &b_query2, pool ) );

    LCN_TEST( lcn_boolean_query_add_term( b_query1, "text", "is", LCN_BOOLEAN_CLAUSE_SHOULD  ) );
    LCN_TEST( lcn_boolean_query_add_term( b_query1, "text", "classes", LCN_BOOLEAN_CLAUSE_SHOULD  ) );


    LCN_TEST( lcn_boolean_query_add_term( b_query2, "text", "classes", LCN_BOOLEAN_CLAUSE_MUST_NOT  ) );
    LCN_TEST( lcn_boolean_query_add_term( b_query2, "text", "is", LCN_BOOLEAN_CLAUSE_SHOULD  ) );

    LCN_TEST( lcn_searcher_search( searcher, &hits, b_query1, NULL, pool ) );
    LCN_TEST( lcn_searcher_search( searcher, &hits, b_query2, NULL, pool ) );

    LCN_TEST( lcn_query_bitvector_create( &bitvector, b_query2, searcher, NULL, pool ) );
    LCN_TEST( lcn_searcher_search( searcher, &hits, b_query1, bitvector, pool ) );

    CuAssertIntEquals( tc, 27, lcn_hits_length( hits ) );

    apr_pool_destroy( pool );
}

CuSuite*
make_query_bitvector_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_query_bitvector );
    SUITE_ADD_TEST( s, test_bitvector_disjunction );
    SUITE_ADD_TEST( s, test_bitvector_conjunction );
    return s;
}
