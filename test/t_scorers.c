#include "test_all.h"
#include "lcn_search.h"
#include "scorer.h"

static lcn_searcher_t* searcher;
static apr_pool_t* test_pool;

static int it_result[] = { 10, 53, 67, 68, 69, 97, 127, 144, 157, 179, 183, 193, 197, 203, 255, 260, 317, 321, 322, 324, 329, };

static int the_result[] = { 7, 8, 9, 10, 13, 14, 22, 32, 49, 57, 60, 64, 69, 73, 74, 78, 86, 89, 94, 97, 104, 106, 107, 109, 110, 111, 113, 114, 115, 117, 119, 122, 125, 134, 139, 141, 147, 157, 162, 168, 173, 174, 175, 181, 184, 198, 199, 207, 208, 209, 225, 239, 240, 244, 258, 267, 282, 285, 288, 311, 312, 319, 323, 324, 328, 330, };

static int is_result[] = { 7, 9, 10, 13, 14, 18, 19, 35, 61, 74, 87, 110, 119, 123, 162, 164, 172, 175, 176, 180, 181, 183, 257, 288, 317, 318, 320, };

static int it_the_conj[] = { 10, 69, 97, 157, 324 };

static int it_the_disjunction[] = { 7, 8, 9, 10, 13, 14, 22, 32, 49, 53, 57, 60, 64, 67, 68, 69, 73, 74, 78, 86, 89, 94, 97, 104, 106, 107, 109, 110, 111, 113, 114, 115, 117, 119, 122, 125, 127, 134, 139, 141, 144, 147, 157, 162, 168, 173, 174, 175, 179, 181, 183, 184, 193, 197, 198, 199, 203, 207, 208, 209, 225, 239, 240, 244, 255, 258, 260, 267, 282, 285, 288, 311, 312, 317, 319, 321, 322, 323, 324, 328, 329, 330 };

static int the_req_it_excl[] = { 7, 8, 9, 13, 14, 22, 32, 49, 57, 60, 64, 73, 74, 78, 86, 89, 94, 104, 106, 107, 109, 110, 111, 113, 114, 115, 117, 119, 122, 125, 134, 139, 141, 147, 162, 168, 173, 174, 175, 181, 184, 198, 199, 207, 208, 209, 225, 239, 240, 244, 258, 267, 282, 285, 288, 311, 312, 319, 323, 328, 330 };

static int it_req_is_opt[] = { 10, 53, 67, 68, 69, 97, 127, 144, 157, 179, 183, 193, 197, 203, 255, 260, 317, 321, 322, 324, 329 };

static apr_status_t
term_scorer_by_chars( CuTest* tc,
                      const char* field,
                      const char* text,
                      lcn_scorer_t** result,
                      apr_pool_t* pool )
{
    apr_status_t s;
    lcn_term_t* term;
    lcn_weight_t* weight;
    lcn_query_t* query;

    do
    {
        LCNCE( lcn_term_create( &term, field, text, TRUE, pool ) );
        LCNCE( lcn_term_query_create( &query, term, pool ) );
        LCNCE( lcn_query_weight( query, &weight, searcher, pool ) );
        LCNCE( lcn_weight_scorer( weight,
                                  result,
                                  lcn_index_searcher_reader( searcher ),
                                  pool ) );
    }
    while( FALSE );

    return s;
}

static void
setup( CuTest* tc )
{
    LCN_TEST( apr_pool_create( &test_pool, main_pool ) );
    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_2",
                                                 test_pool ) );
}

static void
setup_multi( CuTest* tc )
{
    LCN_TEST( apr_pool_create( &test_pool, main_pool ) );
    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_1",
                                                 test_pool ) );
}

static void
tear_down( CuTest* tc )
{
    apr_pool_destroy( test_pool );
}

static void
test_conjunction_scorer( CuTest* tc )
{
    int i = 0;
    lcn_scorer_t* it_ts, *the_ts, *cs;
    lcn_similarity_t* sim;
    apr_pool_t* cp;

    apr_pool_create( &cp, test_pool );

    LCN_TEST( lcn_default_similarity_create( &sim, cp ) );
    LCN_TEST( lcn_conjunction_scorer_create( &cs, sim, cp ) );
    LCN_TEST( term_scorer_by_chars( tc, "text", "it",  &it_ts, cp ) );
    LCN_TEST( term_scorer_by_chars( tc, "text", "the", &the_ts, cp ) );

    LCN_TEST( lcn_conjunction_scorer_add( cs, it_ts ) );
    LCN_TEST( lcn_conjunction_scorer_add( cs, the_ts ) );

    while( APR_SUCCESS == lcn_scorer_next( cs ) )
    {
        CuAssertIntEquals( tc, it_the_conj[i++], lcn_scorer_doc( cs ) );
    }

    apr_pool_destroy( cp );
}

static void
test_disjunction_sum_scorer( CuTest* tc )
{
    int i = 0;
    lcn_scorer_t* it_ts, *the_ts, *cs;
    lcn_list_t* s_list;
    lcn_similarity_t* sim;
    apr_pool_t* cp;

    apr_pool_create( &cp, test_pool );

    LCN_TEST( lcn_list_create( &s_list, 10, cp ) );
    LCN_TEST( lcn_default_similarity_create( &sim, cp ) );

    LCN_TEST( term_scorer_by_chars( tc, "text", "it",  &it_ts, cp ) );
    LCN_TEST( term_scorer_by_chars( tc, "text", "the", &the_ts, cp ) );
    LCN_TEST( lcn_list_add( s_list, it_ts ) );
    LCN_TEST( lcn_list_add( s_list, the_ts ) );

    LCN_TEST( lcn_disjunction_sum_scorer_create( &cs, s_list, 1, cp ) );

    while( APR_SUCCESS == lcn_scorer_next( cs ) )
    {
        CuAssertIntEquals( tc, it_the_disjunction[i++], lcn_scorer_doc( cs ) );
    }

    apr_pool_destroy( cp );
}

static void
test_req_excl_scorer( CuTest* tc )
{
    int i = 0;
    lcn_scorer_t* it_ts, *the_ts, *res;
    lcn_list_t* s_list;
    lcn_similarity_t* sim;
    apr_pool_t* cp;

    apr_pool_create( &cp, test_pool );

    LCN_TEST( lcn_list_create( &s_list, 10, cp ) );
    LCN_TEST( lcn_default_similarity_create( &sim, cp ) );


    LCN_TEST( term_scorer_by_chars( tc, "text", "it",  &it_ts, cp ) );
    LCN_TEST( term_scorer_by_chars( tc, "text", "the", &the_ts, cp ) );

    LCN_TEST( lcn_req_excl_scorer_create( &res, the_ts, it_ts,  cp ) );

    while( APR_SUCCESS == lcn_scorer_next( res ) )
    {
        CuAssertIntEquals( tc, the_req_it_excl[i++], lcn_scorer_doc( res ) );
    }

    apr_pool_destroy( cp );
}

static void
test_req_opt_scorer( CuTest* tc )
{
    int i = 0;
    lcn_scorer_t* it_ts, *is_ts, *ros;
    lcn_list_t* s_list;
    apr_pool_t* cp;

    apr_pool_create( &cp, test_pool );

    LCN_TEST( lcn_list_create( &s_list, 10, cp ) );

    LCN_TEST( term_scorer_by_chars( tc, "text", "it",  &it_ts, cp ) );
    LCN_TEST( term_scorer_by_chars( tc, "text", "is", &is_ts, cp ) );

    LCN_TEST( lcn_req_opt_sum_scorer_create( &ros, it_ts, is_ts,  cp ) );

    while( APR_SUCCESS == lcn_scorer_next( ros ) )
    {
        CuAssertIntEquals( tc, it_req_is_opt[i++], lcn_scorer_doc( ros ) );
    }

    apr_pool_destroy( cp );
}

static void
test_term_scorer( CuTest* tc )
{
    int i = 0;
    apr_status_t s;
    lcn_scorer_t* ts1, *ts2, *ts3, *ts4;

    apr_pool_t* cp;

    apr_pool_create( &cp, test_pool );
    LCN_TEST( term_scorer_by_chars( tc, "text", "it",  &ts1, cp ) );
    LCN_TEST( term_scorer_by_chars( tc, "text", "the", &ts2, cp ) );
    LCN_TEST( term_scorer_by_chars( tc, "text", "is",  &ts3, cp ) );

    while( APR_SUCCESS == ( s = lcn_scorer_next( ts1 ) ) )
    {
        CuAssertIntEquals( tc, it_result[i++], lcn_scorer_doc( ts1 ) );
    }

    i = 0;
    while( APR_SUCCESS == ( s = lcn_scorer_next( ts2 ) ) )
    {

        CuAssertIntEquals( tc, the_result[i++], lcn_scorer_doc( ts2 ) );
    }

    i = 0;
    while( APR_SUCCESS == ( s = lcn_scorer_next( ts3 ) ) )
    {
        CuAssertIntEquals( tc, is_result[i++], lcn_scorer_doc( ts3 ) );
    }

    LCN_TEST( term_scorer_by_chars( tc, "text", "it",  &ts4, cp ) );

    LCN_TEST( lcn_scorer_skip_to( ts4, 324 ) );
    CuAssertIntEquals( tc, 324, lcn_scorer_doc( ts4 ) );
    LCN_TEST( lcn_scorer_skip_to( ts4, 329 ) );
    CuAssertIntEquals( tc, 329, lcn_scorer_doc( ts4 ) );

    apr_pool_destroy( cp );

}

CuSuite*
make_scorer_suite( )
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, setup );
    SUITE_ADD_TEST( s, test_term_scorer );
    SUITE_ADD_TEST( s, test_disjunction_sum_scorer );
    SUITE_ADD_TEST( s, test_conjunction_scorer );
    SUITE_ADD_TEST( s, test_req_excl_scorer );
    SUITE_ADD_TEST( s, test_req_opt_scorer );
    SUITE_ADD_TEST( s, tear_down );

    SUITE_ADD_TEST( s, setup_multi );
    SUITE_ADD_TEST( s, test_disjunction_sum_scorer );
    SUITE_ADD_TEST( s, test_term_scorer );
    SUITE_ADD_TEST( s, test_conjunction_scorer );
    SUITE_ADD_TEST( s, test_req_excl_scorer );
    SUITE_ADD_TEST( s, test_req_opt_scorer );
    SUITE_ADD_TEST( s, tear_down );

    return s;
}
