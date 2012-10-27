#include "test_all.h"
#include "lcn_search.h"
#include "lcn_bitvector.h"


static void
test_query( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_hits_t *hits;
    lcn_query_t *query, *one_hit_query;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_2",
                                                 pool ) );

    LCN_TEST( lcn_term_query_create_by_chars( &query, "text", "phrasequery", pool ));
    LCN_TEST( lcn_one_hit_query_create( &one_hit_query,  query, pool ));
    LCN_TEST( lcn_searcher_search( searcher, &hits, one_hit_query, NULL, pool ) );
    CuAssertIntEquals( tc, 1, lcn_hits_length( hits ));
    
    LCN_TEST( lcn_term_query_create_by_chars( &query, "text", "phrasequeryx", pool ));
    LCN_TEST( lcn_one_hit_query_create( &one_hit_query,  query, pool ));
    LCN_TEST( lcn_searcher_search( searcher, &hits, one_hit_query, NULL, pool ) );
    CuAssertIntEquals( tc, 0, lcn_hits_length( hits ));

    LCN_TEST( lcn_index_searcher_close( searcher ));

    apr_pool_destroy( pool );
}

static void
test_clone_type( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* o_query, *clone, *query;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_term_query_create_by_chars( &query, "text", "phrasequery", pool ));

    LCN_TEST( lcn_one_hit_query_create( &o_query, query, pool ));
    CuAssertIntEquals( tc, LCN_QUERY_TYPE_ONE_HIT, lcn_query_type( o_query ));
    LCN_TEST( lcn_query_clone( o_query, &clone, pool ));
    CuAssertIntEquals( tc, LCN_QUERY_TYPE_ONE_HIT, lcn_query_type( clone ));

    apr_pool_destroy( pool );
}

CuSuite*
make_one_hit_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_query );
    SUITE_ADD_TEST( s, test_clone_type );
    
    return s;
}
