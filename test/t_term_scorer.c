#include "test_all.h"
#include "lcn_search.h"
#include "scorer.h"

static void
test_term_scorer( CuTest* tc )
{
    
}

CuSuite*
make_term_scorer_suite( )
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_term_scorer );
    return s;
}
