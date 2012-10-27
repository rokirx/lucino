#include "test_all.h"
#include <compound_file_util.h>

static void
test_file_extensions( CuTest* tc )
{
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "_a4.aa.fnm" ) );
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "_a4.fnm" ) );
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "aa.frq" ) );
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "_a4.aa.prx" ) );
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "_a4.aa.fdx" ) );
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "_a4.aa.fdt" ) );
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "_a4.aa.tii" ) );
    CuAssertTrue( tc, lcn_compound_file_util_check_file_extension ( "_a4.aa.tis" ) );   
    
    CuAssertTrue( tc, !lcn_compound_file_util_check_file_extension ( "_a4.aa.fn" ) );
    CuAssertTrue( tc, !lcn_compound_file_util_check_file_extension ( "aa.fi" ) );
    CuAssertTrue( tc, !lcn_compound_file_util_check_file_extension ( "_fn" ) );
    CuAssertTrue( tc, !lcn_compound_file_util_check_file_extension ( "_a.f1e00" ) );
    CuAssertTrue( tc, !lcn_compound_file_util_check_file_extension ( "_a.f1e" ) );
}

CuSuite*
make_compound_file_util_suite (void)
{
    CuSuite* s = CuSuiteNew();
    
    SUITE_ADD_TEST(s, test_file_extensions);
    
    return s;
}
