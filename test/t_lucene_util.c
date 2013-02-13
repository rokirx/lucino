#include "test_all.h"
#include "lucene.h"
#include "lucene_util.h"
#include "lcn_util.h"
#include "CuTest.h"

static void
test_cu_to_lower(CuTest* tc)
{
    apr_pool_t *pool;
    (void) apr_pool_create( &pool, NULL );
    char *s = apr_pstrdup( pool, "ABCDEFH \304\326\334\301");
    lcn_string_to_lower( s );
    CuAssertTrue(tc, strcmp("abcdefh \344\366\374\341",s) == 0);
}

static void
test_cu_string_difference(CuTest* tc)
{
    CuAssertIntEquals(tc, 0, lcn_string_difference( "a", "b" ) );
    CuAssertIntEquals(tc, 1, lcn_string_difference( "ax", "ab" ) );
    CuAssertIntEquals(tc, 1, lcn_string_difference( "a", "ab" ) );
    CuAssertIntEquals(tc, 2, lcn_string_difference( "abc", "ab" ) );
    CuAssertIntEquals(tc, 2, lcn_string_difference( "abc", "abxy" ) );
    CuAssertIntEquals(tc, 3, lcn_string_difference( "abc", "abc" ) );
}

#define CHECK36( n, s )                                 \
{                                                       \
    lcn_itoa36( n, buf );                               \
    CuAssertStrEquals(tc, s, buf );                     \
    CuAssertIntEquals(tc, n, lcn_atoi36( buf ) );       \
}


static void
test_cu_base_36(CuTest* tc)
{
    char buf[10];

    CHECK36( 0, "0" );
    CHECK36( 1, "1" );
    CHECK36( 2, "2" );
    CHECK36( 10, "a" );
    CHECK36( 35, "z" );
    CHECK36( 36, "10" );
    CHECK36( 37, "11" );
    CHECK36( 40, "14" );
    CHECK36( (10 * 36*36 + 15), "a0f" );
}

CuSuite *make_lucene_util_suite (void)
{
    CuSuite *s=CuSuiteNew();
    SUITE_ADD_TEST(s,test_cu_to_lower);
    SUITE_ADD_TEST(s,test_cu_string_difference);
    SUITE_ADD_TEST(s,test_cu_base_36);
    return s;
}
