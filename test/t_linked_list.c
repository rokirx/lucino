#include "test_all.h"
#include "lcn_util.h"
#include <time.h>

#define TEST_SIZE (1000)

static void
test_add_last( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_linked_list_t* l;
    unsigned int i;
    int vals[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

    int* p_vals = vals;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_linked_list_create( &l, pool ) );
    
    CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );
    for( i = 0; i < 10; i++ )
    {
        const lcn_linked_list_el_t* first, *last;

        LCN_TEST( lcn_linked_list_add_last( l, (p_vals + i ) ) );
        CuAssertIntEquals( tc, i+1, lcn_linked_list_size( l ) );
        
        first = lcn_linked_list_first( l );
        last  = lcn_linked_list_last( l );
        
        CuAssertIntEquals( tc,
                           vals[i],
                           *((int*)lcn_linked_list_content( last ) ) );
        CuAssertIntEquals( tc,
                           vals[0],
                           *((int*)lcn_linked_list_content( first ) ) );
    }

    i = 0;

    while( lcn_linked_list_size( l ) > 0 )
    { 
        const lcn_linked_list_el_t* last;
        last = lcn_linked_list_last( l );

        CuAssertIntEquals( tc, 
                           vals[9 - i++],
                           *((int*)lcn_linked_list_content( last ) ) );
        lcn_linked_list_remove_last( l );
    }    
    apr_pool_destroy( pool );
}

static void
test_add_first( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_linked_list_t* l;
    unsigned int i;
    int vals[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

    int* p_vals = vals;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_linked_list_create( &l, pool ) );
    
    CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );
    for( i = 0; i < 10; i++ )
    {
        const lcn_linked_list_el_t* first, *last;

        LCN_TEST( lcn_linked_list_add_first( l, (p_vals + i ) ) );
        CuAssertIntEquals( tc, i+1, lcn_linked_list_size( l ) );
        
        first = lcn_linked_list_first( l );
        last  = lcn_linked_list_last( l );
        
        CuAssertIntEquals( tc,
                           vals[i],
                           *((int*)lcn_linked_list_content( first ) ) );

        CuAssertIntEquals( tc,
                           vals[0],
                           *((int*)lcn_linked_list_content( last ) ) );
    }

    i = 0;

    while( lcn_linked_list_size( l ) > 0 )
    { 
        const lcn_linked_list_el_t* first;
        first = lcn_linked_list_first( l );
        CuAssertIntEquals( tc, 
                           vals[9 - i++],
                           *((int*)lcn_linked_list_content( first ) ) );
        lcn_linked_list_remove_first( l );

  
    }
    apr_pool_destroy( pool );
}

static void
test_huge( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_list_t* tl;
    lcn_linked_list_t* l;
    const lcn_linked_list_el_t* el;
    unsigned int i;

    int* vals, *p_vals;
    
    srand( time( NULL ) );


    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    vals = apr_palloc( pool, TEST_SIZE * sizeof( int ) );
    p_vals = vals;
    LCN_TEST( lcn_linked_list_create( &l, pool ) );
    LCN_TEST( lcn_list_create( &tl, TEST_SIZE, pool ) );

    for( i = 0; i < TEST_SIZE; i++ )
    {
        vals[i] = rand();
    }

    
    CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );
    for( i = 0; i < TEST_SIZE; i++ )
    {
        const lcn_linked_list_el_t* first, *last;

        LCN_TEST( lcn_linked_list_add_last( l, (p_vals + i ) ) );
        LCN_TEST( lcn_list_add( tl, (p_vals + i ) ) );

        CuAssertIntEquals( tc, i+1, lcn_linked_list_size( l ) );
        
        first = lcn_linked_list_first( l );
        last  = lcn_linked_list_last( l );
        
        CuAssertIntEquals( tc,
                           vals[i],
                           *((int*)lcn_linked_list_content( last ) ) );

        CuAssertIntEquals( tc,
                           vals[0],
                           *((int*)lcn_linked_list_content( first ) ) );
    }

    CuAssertIntEquals( tc, TEST_SIZE, lcn_linked_list_size( l ) );

    for( el = lcn_linked_list_first( l ), i = 0;
         el != NULL;
         el = lcn_linked_list_next( el ), i++ )
    {
        CuAssertIntEquals( tc, 
                           *((int*)lcn_list_get( tl, i ) ),
                           *((int*)lcn_linked_list_content( el ) ) );
    }
    
    apr_pool_destroy( pool );
}

static void
test_tiny( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_linked_list_t* l;
    unsigned int i;
    const lcn_linked_list_el_t* el;



    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_linked_list_create( &l, pool ) );

    for( i = 0; i <  TEST_SIZE; i++ )
    {
        int val = 42;

        LCN_TEST( lcn_linked_list_add_first( l, &val ) );

        CuAssertIntEquals( tc, 1, lcn_linked_list_size( l ) );

        el = lcn_linked_list_last( l );

        CuAssertIntEquals( tc, 42,*((int*)lcn_linked_list_content( el ) ) );

        lcn_linked_list_remove_last( l );
    
        CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );

        /**/
        val = 23;

        LCN_TEST( lcn_linked_list_add_last( l, &val ) );

        CuAssertIntEquals( tc, 1, lcn_linked_list_size( l ) );

        el = lcn_linked_list_first( l );

        CuAssertIntEquals( tc, 23,*((int*)lcn_linked_list_content( el ) ) );

        lcn_linked_list_remove_first( l );
    
        CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );

        /**/

        val = 12345;

        LCN_TEST( lcn_linked_list_add_first( l, &val ) );

        CuAssertIntEquals( tc, 1, lcn_linked_list_size( l ) );

        el = lcn_linked_list_last( l );

        CuAssertIntEquals( tc, 12345,*((int*)lcn_linked_list_content( el ) ) );

        lcn_linked_list_remove_last( l );
    
        CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );

        lcn_linked_list_clear( l );

        CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );
    }


    apr_pool_destroy( pool );
}

static void
test_remove_first( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_linked_list_t* l;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_linked_list_create( &l, pool ) );

    lcn_linked_list_add_last( l, "eins" );
    lcn_linked_list_add_last( l, "zwei" );
    lcn_linked_list_add_last( l, "drei" );

    CuAssertStrEquals( tc, "eins", lcn_linked_list_content( lcn_linked_list_remove_first( l ) ) );
    CuAssertStrEquals( tc, "zwei", lcn_linked_list_content( lcn_linked_list_remove_first( l ) ) );
    CuAssertStrEquals( tc, "drei", lcn_linked_list_content( lcn_linked_list_remove_first( l ) ) );
    CuAssertIntEquals( tc, 0, lcn_linked_list_size( l ) );
    
}

static void
test_to_array( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_linked_list_t* l;
    unsigned int i;
    int vals[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };
    lcn_ptr_array_t* array;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_linked_list_create( &l, pool ) );

    for( i = 0; i < 10; i++ )
    {
        lcn_linked_list_add_last( l, (vals + i ) );
    }

    LCN_TEST( lcn_linked_list_to_array( l, &array, pool ) );

    for( i = 0; i < 10; i++ )
    {
        CuAssertIntEquals( tc, 
                           *((int*)array->arr[i]),
                           vals[i] );
    }

    apr_pool_destroy( pool );
}

CuSuite* 
make_linked_list_suite( void )
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_add_last );
    SUITE_ADD_TEST( s, test_add_first );
    SUITE_ADD_TEST( s, test_huge );
    SUITE_ADD_TEST( s, test_tiny );
    SUITE_ADD_TEST( s, test_to_array );
    SUITE_ADD_TEST( s, test_remove_first );

    return s;
}

