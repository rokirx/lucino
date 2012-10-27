#include "test_all.h"
#include "lcn_util.h"

#define TEST_COUNT (1000)

static lcn_bool_t
test_less_than( lcn_priority_queue_t *pq, void* a, void* b )
{
    return (lcn_bool_t)( *((int*)a) < *((int*)b));
}

static char*
queue_to_string( lcn_priority_queue_t *queue, apr_pool_t *pool )
{
    int i;
    char *s = "";

    for( i = 1; i <= lcn_priority_queue_size( queue ); i++ )
    {
        int *v = lcn_priority_queue_element_at( queue, i );
        s = apr_psprintf( pool, "%s %d", s, *v );
    }

    return s;
}

static void
test_priority_queue( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_priority_queue_t* queue;

    int vals[ TEST_COUNT ];
    unsigned int i;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_priority_queue_create( &queue,
                                         test_less_than,
                                         pool ) );

    LCN_TEST( lcn_priority_queue_initialize( queue, TEST_COUNT ) );

    for( i = 0; i < TEST_COUNT; i++ )
    {
        vals[i]= ( TEST_COUNT - 1 ) - i;
        lcn_priority_queue_put( queue, (vals+ (  i ) ) );
    }

    CuAssertIntEquals( tc, TEST_COUNT, lcn_priority_queue_size( queue ) );

    for( i = 0; i < TEST_COUNT; i++ )
    {
        CuAssertIntEquals( tc,
                           i,
                           *((int*)lcn_priority_queue_pop( queue ) ) );
    }

    /* test random numbers */

#if 0

    LCN_TEST( lcn_priority_queue_create( &queue,
                                         test_less_than,
                                         pool ) );

    LCN_TEST( lcn_priority_queue_initialize( queue, 10 ) );

    for( i = 0; i < 100; i++ )
    {
        vals[i] = rand() % 100;
        lcn_priority_queue_insert( queue, (vals+ (  i ) ) );
        flog( stderr, "%s\n", queue_to_string( queue, pool  ) );
    }

    while( lcn_priority_queue_size( queue ) > 0 )
    {
        int *v = lcn_priority_queue_pop( queue );
        flog( stderr, "%d -> %s\n", *v, queue_to_string( queue, pool ) );
    }
#endif

    apr_pool_destroy( pool );
}

static void
test_clear( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_priority_queue_t* queue;

    int i1 = 1, i2 = 2, i3 = 3;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_priority_queue_create( &queue, test_less_than, pool ) );
    LCN_TEST( lcn_priority_queue_initialize( queue, 3 ) );

    lcn_priority_queue_put( queue, &i2 );
    lcn_priority_queue_put( queue, &i3 );
    lcn_priority_queue_put( queue, &i1 );
    CuAssertIntEquals( tc, lcn_priority_queue_size( queue ), 3 );
    lcn_priority_queue_clear( queue );
    CuAssertIntEquals( tc, lcn_priority_queue_size( queue ), 0 );

    apr_pool_destroy( pool );
}

CuSuite*
make_priority_queue_suite( void )
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_priority_queue );
    SUITE_ADD_TEST( s, test_clear );

    return s;
}
