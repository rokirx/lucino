#include "test_all.h"
#include "lcn_util.h"


static void
test_escape( CuTest* tc )
{
    apr_pool_t* pool;
    char* str;
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_string_escape( &str,
                                 "\\,",
                                 ",",
                                 '\\',
                                 pool ) );
    CuAssertStrEquals( tc, "\\\\\\,", str );

    LCN_TEST( lcn_string_unescape( &str,
                                   "\\\\\\,",
                                   ",",
                                   '\\',
                                   pool ) );
    CuAssertStrEquals( tc, "\\,", str );

    LCN_TEST( lcn_string_escape( &str,
                                 "Text in \"parantheses\"",
                                 "\"",
                                 '-',
                                 pool ) );

    CuAssertStrEquals( tc, "Text in -\"parantheses-\"", str );
    
    LCN_TEST( lcn_string_unescape( &str,
                                   "Text in -\"parantheses-\"",
                                   "\"",
                                   '-',
                                   pool ) );

    CuAssertStrEquals( tc, "Text in \"parantheses\"",
                       str );
                       
    CuAssertPtrNotNull( tc, ( str = (char*)lcn_string_next_unescaped( 
                                  "blah\\\"1\",  \"blah\\\"2\"", '"', '\\' ) ) );
    CuAssertStrEquals( tc, "\",  \"blah\\\"2\"", str );
    CuAssertStrEquals( tc, "\"\"", 
                       lcn_string_next_unescaped( "\"\"", '"', '\\' ) );
    CuAssertStrEquals( tc, "\"", 
                       lcn_string_next_unescaped( "\\\"\"",
                                                  '"', '\\' ) );

    CuAssertStrEquals( tc, "\"",
                      lcn_string_next_unescaped( "\"", '"', '\\' ) );
    CuAssertStrEquals( tc, "\"",
                       lcn_string_next_unescaped( "title\"", '"', '\\' ) );

    CuAssertStrEquals( tc, "\"",
                       lcn_string_next_unescaped( "blah\\\\\"", '"', '\\' ) );
    apr_pool_destroy( pool );
}

static void
test_starts_with( CuTest* tc )
{
    CuAssert( tc, "", lcn_string_starts_with( "a", "a" ) );

    CuAssert( tc, "", lcn_string_starts_with( "abdefghijklmnopqrstuvwxyz", 
                                              "a" ) );

    CuAssert( tc, "", !lcn_string_starts_with( "abdefghijklmnopqrstuvwxyz", 
                                              "xyz" ) );

    CuAssert( tc, "", !lcn_string_starts_with( "ab",
                                              "abc" ) );
}

static void
test_purge_whitespaces( CuTest* tc )
{
    char buf[]="               blah blubb a b            ";

    CuAssertStrEquals( tc, "blahblubbab", lcn_string_purge_whitespaces( buf ) );
}

CuSuite* make_string_suite( )
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_starts_with );
    SUITE_ADD_TEST( s, test_escape );
    SUITE_ADD_TEST( s, test_purge_whitespaces );

    return s;
}
