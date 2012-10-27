#include "test_all.h"
#include "lcn_search.h"
#include "lcn_index.h"
#include "lcn_analysis.h"
#include "lcn_util.h"
#include "lcn_bitvector.h"

apr_status_t
custom_counter( void* custom_data, unsigned int doc )
{
    int *a;

    a = custom_data;

    if ( doc < 10 )
    {
        (*a)++;
    }

    return APR_SUCCESS;
}

static void
test_range_counting( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t *query;
    int custom_data = 0;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher, "test_index_1", pool ) );
    LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );
    LCN_TEST( lcn_searcher_set_custom_counter( searcher, custom_counter, (void*) (&custom_data) ));

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 10, custom_data );

    apr_pool_destroy( pool );
}

CuSuite* make_range_counting_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_range_counting );

    return s;
}
