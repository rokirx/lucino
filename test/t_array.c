#include "test_all.h"
#include "lcn_util.h"

#include <stdlib.h>
#include <time.h>

#define TEST_BUF_SIZE (10000)

static void
test_big_array( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_ptr_array_t* array;
    unsigned int i;
    int* vals;

    srand( time(NULL) );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_ptr_array_create( &array, TEST_BUF_SIZE, pool ) );

    CuAssertIntEquals( tc, TEST_BUF_SIZE, array->length );

    vals = apr_palloc( pool, TEST_BUF_SIZE * sizeof( int ) );

    for( i = 0; i < TEST_BUF_SIZE; i++ )
    {
        vals[i] = rand();
        array->arr[i] =  ( vals + i );
    }

    for( i = 0; i < TEST_BUF_SIZE; i++ )
    {
        CuAssertIntEquals( tc, vals[i], *((int*)array->arr[i] ) );
    }
}

static void
test_values( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_ptr_array_t* array;
    unsigned int i;
    int vals[] = { 1, 9, 2, 8, 3 ,7, 4, 6, 5, 0 };
    int* ptr_vals = vals;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_ptr_array_create( &array, 10, pool ) );

    CuAssertIntEquals( tc, 10, array->length );

    for( i = 0; i < 10; i++ )
    {
        array->arr[i] = ( ptr_vals + i );
    }

    for( i = 0; i < 10; i++ )
    {
        CuAssertIntEquals( tc, vals[i], *((int*)array->arr[i] ) );
    }

    apr_pool_destroy( pool );
}

static void
test_empty_array( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_ptr_array_t* array;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_ptr_array_create( &array, 0, pool ) );

    CuAssertIntEquals( tc, 0, array->length );

    apr_pool_destroy( pool );
}

static void
test_byte_array( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_byte_array_t* array;
    int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_byte_array_create( &array, 128, pool ) );

    CuAssertIntEquals( tc, 128, array->length );

    for( i = 0; i < 128; i++ )
    {
        array->arr[i] = ( 127 - i );
    }

    for( i = 0; i < 128; i++ )
    {
        CuAssertIntEquals( tc, ( 127 - i ), array->arr[i] );
    }

    apr_pool_destroy( pool );
}

static void
test_int_array( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_int_array_t* array;
    int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_int_array_create( &array, 1000, pool ) );

    CuAssertIntEquals( tc, 1000, array->length );

    for( i = 0; i < 1000; i++ )
    {
        array->arr[i] = ( 999 - i );
    }

    for( i = 0; i < 1000; i++ )
    {
        CuAssertIntEquals( tc, ( 999 - i ), array->arr[i] );
    }

    apr_pool_destroy( pool );
}

static void
test_float_array( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_float_array_t* array;
    int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_float_array_create( &array, 1000, pool ) );

    CuAssertIntEquals( tc, 1000, array->length );

    for( i = 0; i < 1000; i++ )
    {
        array->arr[i] =  (float)( 1 / (i+1) );
    }

    for( i = 0; i < 1000; i++ )
    {
        CuAssertDblEquals( tc,
                           (double)( 1 / (i+1) ),
                           (double)array->arr[i],
                           0 );
    }

    apr_pool_destroy( pool );
}

static char *
check_for_possible_date( const char *day,
                         const char *month,
                         const char *year,
                         apr_pool_t *pool )
{
    apr_int64_t i_day = 0, i_month = 0, i_year = 0;
    char *c_day = NULL;
    char *c_month = NULL;
    char *c_year = NULL;

    if ( NULL == day || strlen(day) > 2 || !LCN_IS_DIGIT(*day) )
    {
        return NULL;
    }

    if ( strlen(day) == 2 && !LCN_IS_DIGIT(*(day+1)))
    {
        return NULL;
    }

    i_day = apr_atoi64( day );

    if ( !(0 < i_day && i_day <= 31 ))
    {
        return NULL;
    }

    /* at this point we asserted a valid day value */

    if ( NULL != month )
    {
        if ( LCN_IS_DIGIT(*month))
        {
            if ( strlen(month) > 2 )
            {
                return NULL;
            }

            if ( strlen(month) == 2 && !LCN_IS_DIGIT(*(month+1)))
            {
                return NULL;
            }

            i_month = apr_atoi64(month);

            if ( !(0 < i_month && i_month <= 12 ))
            {
                return NULL;
            }
        }
        else
        {
            int i;
            int possible_month = 0;
            char *months[13] = {
                "januar",
                "februar",
                "m\344rz",
                "april",
                "mai",
                "juni",
                "juli",
                "august",
                "september",
                "oktober",
                "november",
                "dezember",
                NULL };

            for( i = 0; months[i] != NULL; i++ )
            {
                if ( 0 == strncmp( month, months[i], strlen(month) ))
                {
                    possible_month = i+1;
                    break;
                }
            }

            if ( possible_month == 0 )
            {
                return NULL;
            }

            if ( strlen(month) > 2 )
            {
                i_month = possible_month;
            }
        }

        /* now check if it's a year */

        if ( month > 0 && NULL != year && strlen( year ) > 0 )
        {
            if ( ! (1 <= strlen(year) && (strlen(year) <= 4) && ('1' == *year || '2' == *year )))
            {
                return NULL;
            }

            if ( 2 <= strlen(year ))
            {
                if (!(('1' == *year && ('9' == *(year+1) || '8' == *(year+1))) ||
                      ('2' == *year && ('0' == *(year+1) || '1' == *(year+1)))))
                {
                    return NULL;
                }
            }

            if ( 3 <= strlen(year))
            {
                if ( ! LCN_IS_DIGIT( *(year+2) ))
                {
                    return NULL;
                }
            }

            if ( 4 <= strlen(year))
            {
                if ( ! LCN_IS_DIGIT( *(year+3)))
                {
                    return NULL;
                }
            }

            i_year = apr_atoi64(year);
        }
    }

    c_day = NULL;

    if ( i_day > 0 )
    {
        c_day = apr_itoa( pool, i_day );
    }

    c_month = NULL;

    if ( i_month > 0 )
    {
        c_month = apr_itoa( pool, i_month );
    }

    c_year = NULL;

    if ( i_month > 0 && i_year > 0 )
    {
        c_year = apr_itoa( pool, i_year );
    }

    return apr_pstrcat( pool,
                        ".",
                        (NULL != c_day ? c_day : "" ),
                        (NULL != c_month ? "." : "" ),
                        (NULL != c_month ? c_month : "" ),
                        (NULL != c_month && NULL != c_year ? "." : "" ),
                        (NULL != c_month && NULL != c_year ? c_year : "" ),
                        NULL );
}


static void
test_datum_tokens( CuTest* tc )
{
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ));

    CuAssertTrue( tc, NULL == check_for_possible_date( NULL, NULL, NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "011", NULL, NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "a", NULL, NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1b", NULL, NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "0", NULL, NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "32", NULL, NULL, pool ));

    CuAssertTrue( tc, NULL == check_for_possible_date( NULL, "11111", NULL, pool ));

    CuAssertTrue( tc, NULL == check_for_possible_date( "01", "11111", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "01", "1a", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "01", "0", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "01", "13", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "jarnuar", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "je", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "32", "05", NULL, pool ));

    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "1", "a", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "1", "0", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "1", "3", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "1", "17", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "1", "22", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "1", "20a", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "1", "1", "200a", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "32", "05", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "10", "13", NULL, pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "10", "12", "a", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "10", "12", "0", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "10", "12", "3", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "10", "12", "17", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "10", "12", "22", pool ));
    CuAssertTrue( tc, NULL == check_for_possible_date( "10", "12", "2000a", pool ));



    CuAssertStrEquals( tc, ".1", check_for_possible_date( "1", NULL, NULL, pool ));
    CuAssertStrEquals( tc, ".1", check_for_possible_date( "01", NULL, NULL, pool ));
    CuAssertStrEquals( tc, ".10", check_for_possible_date( "10", NULL, NULL, pool ));
    CuAssertStrEquals( tc, ".10.1", check_for_possible_date( "10", "1", NULL, pool ));
    CuAssertStrEquals( tc, ".10", check_for_possible_date( "10", "j", NULL, pool ));
    CuAssertStrEquals( tc, ".10", check_for_possible_date( "10", "ja", NULL, pool ));
    CuAssertStrEquals( tc, ".10.1", check_for_possible_date( "10", "jan", NULL, pool ));
    CuAssertStrEquals( tc, ".10.1", check_for_possible_date( "10", "janua", NULL, pool ));
    CuAssertStrEquals( tc, ".10.1", check_for_possible_date( "10", "januar", NULL, pool ));

    CuAssertStrEquals( tc, ".10.1", check_for_possible_date( "10", "01", NULL, pool ));
    CuAssertStrEquals( tc, ".10.12", check_for_possible_date( "10", "12", NULL, pool ));
    CuAssertStrEquals( tc, ".10.12.1", check_for_possible_date( "10", "12", "1", pool ));
    CuAssertStrEquals( tc, ".10.12.2", check_for_possible_date( "10", "12", "2", pool ));
    CuAssertStrEquals( tc, ".10.12.18", check_for_possible_date( "10", "12", "18", pool ));
    CuAssertStrEquals( tc, ".10.12.19", check_for_possible_date( "10", "12", "19", pool ));
    CuAssertStrEquals( tc, ".10.12.20", check_for_possible_date( "10", "12", "20", pool ));
    CuAssertStrEquals( tc, ".10.12.21", check_for_possible_date( "10", "12", "21", pool ));
    CuAssertStrEquals( tc, ".10.12.201", check_for_possible_date( "10", "12", "201", pool ));
    CuAssertStrEquals( tc, ".10.12.2013", check_for_possible_date( "10", "12", "2013", pool ));

    apr_pool_destroy( pool );
}


static void
test_size_array( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_size_array_t* array;
    unsigned int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_size_array_create( &array, 100, pool ) );

    CuAssertIntEquals( tc, 100, array->length );

    for( i = 0; i < 100; i++ )
    {
        array->arr[i]= ( 99 - i );
    }

    for( i = 0; i < 100; i++ )
    {
        CuAssertIntEquals( tc, ( 99 - i ), array->arr[i] );
    }

    apr_pool_destroy( pool );
}

CuSuite*
make_array_suite( void )
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_datum_tokens );
    SUITE_ADD_TEST( s, test_empty_array );
    SUITE_ADD_TEST( s, test_values );
    SUITE_ADD_TEST( s, test_big_array );
    SUITE_ADD_TEST( s, test_byte_array );
    SUITE_ADD_TEST( s, test_int_array );
    SUITE_ADD_TEST( s, test_float_array );
    SUITE_ADD_TEST( s, test_size_array );

    return s;
}
