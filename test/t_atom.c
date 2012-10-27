#include "test_all.h"
#include "lucene.h"


void TestCuAtom(CuTest* tc)
{
    const char *s;

    char t1[] = "test";
    char t2[] = "test";

    s = lcn_atom_get_str ( "test" );
    CuAssertStrEquals( tc, s, lcn_atom_get_str( "test" ) );
    CuAssertStrEquals( tc, s, lcn_atom_get_str( t1 ) );
    CuAssertStrEquals( tc, s, lcn_atom_get_str( t2 ) );
}

CuSuite *make_atom_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s,TestCuAtom);

    return s;
}
