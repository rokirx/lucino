#include "test_all.h"
#include "lcn_util.h"
#include <time.h>

static void
test_appends( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_string_buffer_t* sb;
    char* test_str;

    float f        = 0.7564;
    unsigned int u = 4095000; 
    int i          = 1024 * 1024 * 1024;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb, pool ) );

    CuAssertIntEquals( tc, 0, lcn_string_buffer_length( sb ) );
    LCN_TEST( lcn_string_buffer_append_float( sb, f ) );
    CuAssertIntEquals( tc, 8, lcn_string_buffer_length( sb ) );
    LCN_TEST( lcn_string_buffer_append( sb, " " ) );
    CuAssertIntEquals( tc, 9, lcn_string_buffer_length( sb ) );
    LCN_TEST( lcn_string_buffer_append_int( sb, i ) );
    CuAssertIntEquals( tc, 19, lcn_string_buffer_length( sb ) );
    LCN_TEST( lcn_string_buffer_append( sb, " " ) );
    LCN_TEST( lcn_string_buffer_append_uint( sb, u ) );
    LCN_TEST( lcn_string_buffer_append( sb, " " ) );
    LCN_TEST( lcn_string_buffer_append( sb, "blah" ) );

    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );
    CuAssertStrEquals( tc, "0.756400 1073741824 4095000 blah", test_str );

    LCN_TEST( lcn_string_buffer_append_buffer( sb, sb ) );
    LCN_TEST( lcn_string_buffer_append( sb, "" ) );
    LCN_TEST( lcn_string_buffer_nappend( sb, "123456789", 4 ) );
    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );

    CuAssertStrEquals( tc, 
                       "0.756400 1073741824 4095000 blah"
                       "0.756400 1073741824 4095000 blah1234",
                       test_str );
    CuAssertIntEquals( tc, strlen( test_str ), lcn_string_buffer_length( sb ) );
    apr_pool_destroy( pool );
}

#define TEST_LENGTH ( 2 * 1024 * 1024 )

static void
test_append_many_chars( CuTest* tc )
{
    apr_pool_t* pool;
    unsigned int i;
    lcn_string_buffer_t* sb;
    char* test_str;
    char buf[2];

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb, pool ) );

    test_str = apr_palloc( pool, TEST_LENGTH );
    srand( time( NULL ) );
    
    buf[1] = 0;

    for( i = 0; i < TEST_LENGTH; i++ )
    {
        buf[0] = ( ( rand() % 26 ) + 97 );
        lcn_string_buffer_append( sb, buf );
    }

    apr_pool_destroy( pool );
}

static void
test_append_format( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_string_buffer_t* sb;
    char* test_str;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb, pool ) );
    LCN_TEST( lcn_string_buffer_append_format( sb, "%d : %c : %s",
                                               10, 'b', "blah" ) );
    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );

    CuAssertStrEquals( tc, test_str, "10 : b : blah" );
    CuAssertIntEquals( tc, strlen( test_str ), lcn_string_buffer_length( sb ) );
    apr_pool_destroy( pool );
}

static void
test_fill_huge_buffer( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_string_buffer_t* sb1, *sb2;
    char* test_str1, *test_str2;
    unsigned int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb1, pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb2, pool ) );

    for( i = 0; i < 10000; i++ )
    {
        char act_buf[10];

        apr_snprintf( act_buf, 10, "%u", i );
        LCN_TEST( lcn_string_buffer_append( sb1, act_buf ) );
        
    }

    for( i = 0; i < 10000; i++ )
    {
        char act_buf[10];

        apr_snprintf( act_buf, 10, "%u", i );
        LCN_TEST( lcn_string_buffer_append( sb2, act_buf ) );
        
    }
    
    LCN_TEST( lcn_string_buffer_to_string( sb1, &test_str1, pool ) );
    LCN_TEST( lcn_string_buffer_to_string( sb2, &test_str2, pool ) );

    CuAssertTrue( tc, ( strcmp( test_str1, test_str2 ) == 0 ) );

    apr_pool_destroy( pool );
}

static void
test_fill_string_buffer( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_string_buffer_t* sb;
    char* test_str;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb, pool ) );

    LCN_TEST( lcn_string_buffer_append( sb, "eins" ) );
    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );

    CuAssertStrEquals( tc, "eins", test_str );

    LCN_TEST( lcn_string_buffer_append( sb, "zwei" ) );
    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );

    CuAssertStrEquals( tc, "einszwei", test_str );

    LCN_TEST( lcn_string_buffer_append( sb, "drei " ) );
    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );

    CuAssertStrEquals( tc, "einszweidrei ", test_str );

    LCN_TEST( lcn_string_buffer_append( sb, "vier" ) );
    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );

    CuAssertStrEquals( tc, "einszweidrei vier", test_str );

    apr_pool_destroy( pool );
}

static void
test_empty_string_buffer( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_string_buffer_t* sb;
    char* test_str;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb, pool ) );
    LCN_TEST( lcn_string_buffer_to_string( sb, &test_str, pool ) );

    CuAssertStrEquals( tc, "", test_str );

    apr_pool_destroy( pool );
}

static unsigned int
sum_over_n( unsigned int n )
{
    unsigned int result = 0, i;

    for( i = 1; i <= n; i++ )
    {
        result += i;
    }

    return result;
}

static void
test_nappend2( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_string_buffer_t *sb;
    char *result;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb, pool ) );

    LCN_TEST( lcn_string_buffer_append( sb, "abc" ));

    LCN_TEST( lcn_string_buffer_to_string( sb, &result, pool ) );
    CuAssertStrEquals( tc, "abc", result );

    LCN_TEST( lcn_string_buffer_nappend( sb, "xyz", 10 ));
    LCN_TEST( lcn_string_buffer_to_string( sb, &result, pool ) );
    CuAssertStrEquals( tc, "abcxyz", result );

    apr_pool_destroy( pool );
}


static void
test_nappend( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_string_buffer_t* sb;
    char* test_str = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    char* result;
    unsigned int i, len;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_string_buffer_create( &sb, pool ) );

    len = strlen( test_str );

    for( i = 0; i < len; i++ )
    {
        LCN_TEST( lcn_string_buffer_nappend( sb, test_str, i ) );
        CuAssertIntEquals( tc, sum_over_n( i ), lcn_string_buffer_length( sb ) );
    }

    LCN_TEST( lcn_string_buffer_to_string( sb, &result, pool ) );
    
    CuAssertIntEquals( tc, 0, strcmp( 
                           "aababcabcdabcdeabcdefabcdefgabcdefghabcdefghiabcd"
                           "efghijabcdefghijkabcdefghijklabcdefghijklmabcdefg"
                           "hijklmnabcdefghijklmnoabcdefghijklmnopabcdefghijk"
                           "lmnopqabcdefghijklmnopqrabcdefghijklmnopqrsabcdef"
                           "ghijklmnopqrstabcdefghijklmnopqrstuabcdefghijklmn"
                           "opqrstuvabcdefghijklmnopqrstuvwabcdefghijklmnopqr"
                           "stuvwxabcdefghijklmnopqrstuvwxyabcdefghijklmnopqr"
                           "stuvwxyzabcdefghijklmnopqrstuvwxyzaabcdefghijklmn"
                           "opqrstuvwxyzababcdefghijklmnopqrstuvwxyzabcabcdef"
                           "ghijklmnopqrstuvwxyzabcdabcdefghijklmnopqrstuvwxy"
                           "zabcdeabcdefghijklmnopqrstuvwxyzabcdefabcdefghijk"
                           "lmnopqrstuvwxyzabcdefgabcdefghijklmnopqrstuvwxyza"
                           "bcdefghabcdefghijklmnopqrstuvwxyzabcdefghiabcdefg"
                           "hijklmnopqrstuvwxyzabcdefghijabcdefghijklmnopqrst"
                           "uvwxyzabcdefghijkabcdefghijklmnopqrstuvwxyzabcdef"
                           "ghijklabcdefghijklmnopqrstuvwxyzabcdefghijklmabcd"
                           "efghijklmnopqrstuvwxyzabcdefghijklmnabcdefghijklm"
                           "nopqrstuvwxyzabcdefghijklmnoabcdefghijklmnopqrstu"
                           "vwxyzabcdefghijklmnopabcdefghijklmnopqrstuvwxyzab"
                           "cdefghijklmnopqabcdefghijklmnopqrstuvwxyzabcdefgh"
                           "ijklmnopqrabcdefghijklmnopqrstuvwxyzabcdefghijklm"
                           "nopqrsabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrst"
                           "uabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv"
                           "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvw"
                           "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvw"
                           "xabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv"
                           "wxyabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrst"
                           "uvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzaabcdefghijklmnopqrstuvwxyzabcdefghijklm"
                           "nopqrstuvwxyzababcdefghijklmnopqrstuvwxyzabcdefgh"
                           "ijklmnopqrstuvwxyzabcabcdefghijklmnopqrstuvwxyzab"
                           "cdefghijklmnopqrstuvwxyzabcdabcdefghijklmnopqrstu"
                           "vwxyzabcdefghijklmnopqrstuvwxyzabcdeabcdefghijklm"
                           "nopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefabcd"
                           "efghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyza"
                           "bcdefgabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghabcdefghijklmnopqrstuvwxyzabcdef"
                           "ghijklmnopqrstuvwxyzabcdefghiabcdefghijklmnopqrst"
                           "uvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijabcdefg"
                           "hijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcd"
                           "efghijkabcdefghijklmnopqrstuvwxyzabcdefghijklmnop"
                           "qrstuvwxyzabcdefghijklabcdefghijklmnopqrstuvwxyza"
                           "bcdefghijklmnopqrstuvwxyzabcdefghijklmabcdefghijk"
                           "lmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefgh"
                           "ijklmnabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghijklmnoabcdefghijklmnopqrstuvwxy"
                           "zabcdefghijklmnopqrstuvwxyzabcdefghijklmnopabcdef"
                           "ghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabc"
                           "defghijklmnopqabcdefghijklmnopqrstuvwxyzabcdefghi"
                           "jklmnopqrstuvwxyzabcdefghijklmnopqrabcdefghijklmn"
                           "opqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk"
                           "lmnopqrsabcdefghijklmnopqrstuvwxyzabcdefghijklmno"
                           "pqrstuvwxyzabcdefghijklmnopqrstabcdefghijklmnopqr"
                           "stuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmno"
                           "pqrstuabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghijklmnopqrstuvabcdefghijklmnopqr"
                           "stuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmno"
                           "pqrstuvwabcdefghijklmnopqrstuvwxyzabcdefghijklmno"
                           "pqrstuvwxyzabcdefghijklmnopqrstuvwxabcdefghijklmn"
                           "opqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk"
                           "lmnopqrstuvwxyabcdefghijklmnopqrstuvwxyzabcdefghi"
                           "jklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef"
                           "ghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabc"
                           "defghijklmnopqrstuvwxyzaabcdefghijklmnopqrstuvwxy"
                           "zabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv"
                           "wxyzababcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghijklmnopqrstuvwxyzabcabcdefghijk"
                           "lmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefgh"
                           "ijklmnopqrstuvwxyzabcdabcdefghijklmnopqrstuvwxyza"
                           "bcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx"
                           "yzabcdeabcdefghijklmnopqrstuvwxyzabcdefghijklmnop"
                           "qrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefabcdefg"
                           "hijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcd"
                           "efghijklmnopqrstuvwxyzabcdefgabcdefghijklmnopqrst"
                           "uvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghabcdefghijklmnopqrstuvwxyzabcdef"
                           "ghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabc"
                           "defghiabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijabcd"
                           "efghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyza"
                           "bcdefghijklmnopqrstuvwxyzabcdefghijkabcdefghijklm"
                           "nopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghij"
                           "klmnopqrstuvwxyzabcdefghijklabcdefghijklmnopqrstu"
                           "vwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqr"
                           "stuvwxyzabcdefghijklmabcdefghijklmnopqrstuvwxyzab"
                           "cdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxy"
                           "zabcdefghijklmnabcdefghijklmnopqrstuvwxyzabcdefgh"
                           "ijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcde"
                           "fghijklmnoabcdefghijklmnopqrstuvwxyzabcdefghijklm"
                           "nopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghij"
                           "klmnopabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmn"
                           "opqabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrst"
                           "uvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv"
                           "wxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs"
                           "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvw"
                           "xyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrst"
                           "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvw"
                           "xyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrst"
                           "uabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv"
                           "wxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs"
                           "tuvabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrst"
                           "uvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwabcdefghijklmnopqrstuvwxyzabcdefghijklmnopq"
                           "rstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmn"
                           "opqrstuvwxabcdefghijklmnopqrstuvwxyzabcdefghijklm"
                           "nopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghij"
                           "klmnopqrstuvwxy", result ) );

    CuAssertIntEquals( tc, 5356, lcn_string_buffer_length( sb ) );

    apr_pool_destroy( pool );
}

CuSuite* make_string_buffer_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_appends );
    SUITE_ADD_TEST( s, test_nappend );
    SUITE_ADD_TEST( s, test_nappend2 );
    SUITE_ADD_TEST( s, test_append_many_chars );
    SUITE_ADD_TEST( s, test_empty_string_buffer );
    SUITE_ADD_TEST( s, test_fill_string_buffer );
    SUITE_ADD_TEST( s, test_fill_huge_buffer );
    SUITE_ADD_TEST( s, test_append_format );

    return s;
}
