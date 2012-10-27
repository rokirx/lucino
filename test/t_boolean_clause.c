#include "test_all.h"
#include "lcn_search.h"

static void
test_boolean_clause( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_boolean_clause_t* clause;

    char* clause_str;

    apr_pool_create( &pool, main_pool );
    LCN_TEST( lcn_boolean_clause_create( &clause,
                                         LCN_BOOLEAN_CLAUSE_MUST,
                                         NULL,
                                         pool ) );
    
    LCN_TEST( lcn_boolean_clause_to_string( clause,
                                            &clause_str,
                                            pool ) );

    CuAssertIntEquals( tc, 
                       lcn_boolean_clause_occur( clause ),
                       LCN_BOOLEAN_CLAUSE_MUST );
						     
    CuAssertStrEquals( tc, "+", clause_str );

    CuAssertTrue( tc, lcn_boolean_clause_is_required( clause ) == TRUE );
    CuAssertTrue( tc, lcn_boolean_clause_is_prohibited( clause ) == FALSE );

    lcn_boolean_clause_set_occur( clause, LCN_BOOLEAN_CLAUSE_MUST_NOT );

    CuAssertIntEquals( tc, 
                       lcn_boolean_clause_occur( clause ),
                       LCN_BOOLEAN_CLAUSE_MUST_NOT );

    LCN_TEST( lcn_boolean_clause_to_string( clause,
                                            &clause_str,
                                            pool ) );


    CuAssertStrEquals( tc, "-", clause_str );

    CuAssertTrue( tc, lcn_boolean_clause_is_required( clause ) == FALSE );
    CuAssertTrue( tc, lcn_boolean_clause_is_prohibited( clause ) == TRUE);

    lcn_boolean_clause_set_occur( clause, LCN_BOOLEAN_CLAUSE_SHOULD );

    CuAssertIntEquals( tc, 
                       lcn_boolean_clause_occur( clause ),
                       LCN_BOOLEAN_CLAUSE_SHOULD );

    LCN_TEST( lcn_boolean_clause_to_string( clause,
                                            &clause_str,
                                            pool ) );


    CuAssertStrEquals( tc, "", clause_str );

    CuAssertTrue( tc, lcn_boolean_clause_is_required( clause ) == FALSE );
    CuAssertTrue( tc, lcn_boolean_clause_is_prohibited( clause ) == FALSE );

    apr_pool_destroy( pool );
}

CuSuite* 
make_boolean_clause_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_boolean_clause );
    return s;
}
