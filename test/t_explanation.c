#include "test_all.h"
#include "lcn_search.h"

static char test_result[] = "0.000000 = (null)\n"
    "  0.347200 = blah1\n"
    "    874.875427 = test1\n"
    "    874.875427 = test1\n"
    "    874.875427 = test1\n"
    "    874.875427 = test1\n"
    "    874.875427 = test1\n"
    "    874.875427 = test1\n"
    "  9.872360 = blah2\n"
    "    7783.873047 = test2\n"
    "    7783.873047 = test2\n"
    "    7783.873047 = test2\n"
    "    7783.873047 = test2\n"
    "    7783.873047 = test2\n";

#define ADD_DETAIL( EXPL, VALUE, DESCR )                                \
{                                                                       \
    lcn_explanation_t* act_expl;                                        \
    LCN_TEST( lcn_explanation_create_values( &act_expl,        \
                                                       VALUE,           \
                                                       DESCR,           \
                                                       pool ) );        \
                                                                        \
    LCN_TEST( lcn_explanation_detail_add( EXPL,               \
                                                    act_expl ) );       \
                                                                        \
}

static void
test_sub_explanations( CuTest* tc )
{
    apr_pool_t* pool;
    char* test;
    lcn_explanation_t* explanation1, *explanation2, *explanation3;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );


    LCN_TEST( lcn_explanation_create( &explanation1, pool ) );
    LCN_TEST( lcn_explanation_create_values( &explanation2, 
                                                       0.3472f,
                                                       "blah1",
                                                       pool ) );

    ADD_DETAIL( explanation2, 874.8754f, "test1" );
    ADD_DETAIL( explanation2, 874.8754f, "test1" );
    ADD_DETAIL( explanation2, 874.8754f, "test1" );
    ADD_DETAIL( explanation2, 874.8754f, "test1" );
    ADD_DETAIL( explanation2, 874.8754f, "test1" );
    ADD_DETAIL( explanation2, 874.8754f, "test1" );

    LCN_TEST( lcn_explanation_create_values( &explanation3, 
                                                       9.87236f,
                                                       "blah2",
                                                       pool ) );

    ADD_DETAIL( explanation3, 7783.873f, "test2" );
    ADD_DETAIL( explanation3, 7783.873f, "test2" );
    ADD_DETAIL( explanation3, 7783.873f, "test2" );
    ADD_DETAIL( explanation3, 7783.873f, "test2" );
    ADD_DETAIL( explanation3, 7783.873f, "test2" );

    lcn_explanation_detail_add( explanation1, explanation2 );
    lcn_explanation_detail_add( explanation1, explanation3 );

    


    LCN_TEST( lcn_explanation_to_string( explanation1,
                                                   &test,
                                                   pool ) );


    CuAssertStrEquals( tc, test_result, test );

    apr_pool_destroy( pool );
}

static void
test_empty_explanation( CuTest* tc )
{
    apr_pool_t* pool;

    lcn_explanation_t* explanation;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );


    LCN_TEST( lcn_explanation_create( &explanation,
                                                pool ) );

    CuAssertStrEquals( tc,
                       "(null)",
                       lcn_explanation_description_get( explanation ) );

    CuAssertTrue( tc,
                  lcn_explanation_value_get( explanation ) == 0.0f );

    apr_pool_destroy( pool );
}

CuSuite*
make_explanation_suite()
{
    CuSuite* s = CuSuiteNew();


    SUITE_ADD_TEST( s, test_empty_explanation );
    SUITE_ADD_TEST( s, test_sub_explanations );
    return s;
}
