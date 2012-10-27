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

    LCN_TEST( lcn_ptr_array_create( &array, 
                                    TEST_BUF_SIZE, 
                                    pool ) );
    
    CuAssertIntEquals( tc, TEST_BUF_SIZE, array->length );

    vals = apr_palloc( pool, TEST_BUF_SIZE * sizeof( int ) );

    for( i = 0; i < TEST_BUF_SIZE; i++ )
    {
        vals[i] = rand();
        array->arr[i] =  ( vals + i );
    }

    for( i = 0; i < TEST_BUF_SIZE; i++ )
    {
        CuAssertIntEquals( tc, 
                           vals[i],
                           *((int*)array->arr[i] ) );
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

    LCN_TEST( lcn_ptr_array_create( &array, 
                                    10, 
                                    pool ) );

    CuAssertIntEquals( tc, 10, array->length );

    for( i = 0; i < 10; i++ )
    {
        array->arr[i] = ( ptr_vals + i );
    }

    for( i = 0; i < 10; i++ )
    {
        CuAssertIntEquals( tc,
                           vals[i],
                           *((int*)array->arr[i] ) );
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
        CuAssertIntEquals( tc, 
                           ( 999 - i ), 
                           array->arr[i] );
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
        CuAssertIntEquals( tc, 
                           ( 99 - i ), 
                           array->arr[i] );
    }

    apr_pool_destroy( pool );
}

CuSuite*
make_array_suite( void )
{
    CuSuite* s = CuSuiteNew();
    
    SUITE_ADD_TEST( s, test_empty_array );
    SUITE_ADD_TEST( s, test_values );
    SUITE_ADD_TEST( s, test_big_array );
    SUITE_ADD_TEST( s, test_byte_array );
    SUITE_ADD_TEST( s, test_int_array );
    SUITE_ADD_TEST( s, test_float_array );
    SUITE_ADD_TEST( s, test_size_array );

    return s;
    
}
