#include "test_all.h"
#include "lcn_util.h"

void
test_lucene_list(CuTest* tc)
{
    lcn_list_t *list;
    apr_pool_t *pool;
    int i[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    apr_pool_create( &pool, NULL );

    CuAssertIntEquals(tc, APR_SUCCESS, lcn_list_create( &list, 1, pool ));

    CuAssertTrue(tc, lcn_list_size( list ) == 0);
    CuAssertTrue(tc, lcn_list_get( list, 0 ) == NULL);
    CuAssertTrue(tc, lcn_list_get( list, 1 ) == NULL);

    lcn_list_add( list, &(i[0]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 1);
    CuAssertTrue(tc, lcn_list_get( list, 1 ) == NULL);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 1);
    i[0] = 100;
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 100);

    lcn_list_add( list, &(i[1]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 2);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 2);
    i[1] = 200;
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 200);

    lcn_list_add( list, &(i[2]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 3);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 100);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 200);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 3);

    lcn_list_add( list, &(i[3]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 4);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 100);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 200);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 3);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 4);

    lcn_list_add( list, &(i[4]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 100);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 200);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 3);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 4);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 4 )) == 5);

    lcn_list_set( list, 0, &(i[7]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 8);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 200);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 3);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 4);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 4 )) == 5);

    lcn_list_set( list, 1, &(i[6]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 8);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 7);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 3);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 4);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 4 )) == 5);

    lcn_list_set( list, 2, &(i[5]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 8);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 7);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 6);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 4);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 4 )) == 5);

    lcn_list_set( list, 3, &(i[4]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 8);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 7);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 6);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 4 )) == 5);

    lcn_list_set( list, 4, &(i[3]) );
    CuAssertTrue(tc, lcn_list_size( list ) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 8);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 7);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 6);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 5);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 4 )) == 4);

    lcn_list_remove( list, 3 );
    CuAssertTrue(tc, lcn_list_size( list ) == 4);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 8);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 7);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 6);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 3 )) == 4);

    lcn_list_remove( list, 3 );
    CuAssertTrue(tc, lcn_list_size( list ) == 3);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 8);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 7);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 2 )) == 6);

    lcn_list_remove( list, 3 );
    CuAssertTrue(tc, lcn_list_size( list ) == 3);
    lcn_list_remove( list, 0 );
    CuAssertTrue(tc, lcn_list_size( list ) == 2);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 0 )) == 7);
    CuAssertTrue(tc, *((int *) lcn_list_get( list, 1 )) == 6);

    apr_pool_destroy( pool );
}

void test_remove(CuTest* tc)
{
    lcn_list_t *list;
    apr_pool_t *pool;
    apr_pool_create( &pool, NULL );

    CuAssertIntEquals(tc, APR_SUCCESS, lcn_list_create( &list, 3, pool ));

    lcn_list_add( list, (void*)1 );
    lcn_list_add( list, (void*)2 );
    lcn_list_add( list, (void*)3 );
    lcn_list_add( list, (void*)4 );
    lcn_list_add( list, (void*)5 );
    lcn_list_remove( list, 1 );
    lcn_list_remove( list, 1 );
    lcn_list_add( list, (void*)10 );

    CuAssertIntEquals( tc, 4, lcn_list_size( list ));

    CuAssertIntEquals( tc, 1,  (unsigned long int)lcn_list_get( list, 0 ) );
    CuAssertIntEquals( tc, 4,  (unsigned long int) lcn_list_get( list, 1 ) );
    CuAssertIntEquals( tc, 5,  (unsigned long int) lcn_list_get( list, 2 ) );
    CuAssertIntEquals( tc, 10, (unsigned long int) lcn_list_get( list, 3 ) );

    lcn_list_add( list, (void*) 17 );

    CuAssertIntEquals( tc, 17, (unsigned long int) lcn_list_get( list, 4 ) );


    lcn_list_swap( list, 1, 3 );

    CuAssertIntEquals( tc, 10, (unsigned long int) lcn_list_get( list, 1 ) );
    CuAssertIntEquals( tc, 4,  (unsigned long int) lcn_list_get( list, 3 ) );

    lcn_list_swap( list, 4, 0 );

    CuAssertIntEquals( tc, 17, (unsigned long int) lcn_list_get( list, 0 ) );
    CuAssertIntEquals( tc, 1,  (unsigned long int) lcn_list_get( list, 4 ) );


    apr_pool_destroy( pool );
}

static int
test_cmp( const void* a, const void* b )
{
    return strcmp( *((const char**)a ),
                   *((const char**)b ) );
}

static void
test_sort( CuTest* tc )
{
    unsigned int i;
    apr_pool_t* pool;
    lcn_list_t* list;

    char* words[10] = {
        "xx",
        "qq",
        "aa",
        "zz",
        "yy",
        "aaa",
        "a",
        "abzc",
        "10000",
        "010000"
    };
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &list, 10, pool ) );

    for( i = 0; i < 10; i++ )
    {
        LCN_TEST( lcn_list_add( list, words[i] ) );
    }

    LCN_TEST( lcn_list_sort( list, test_cmp ) );
}

static void
test_insert( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_list_t* list;
    apr_ssize_t i;
    int int_list[] = { 0, 1, 3, 4, 5 };
    int to_add[] = { 2, 6, 1000 };
    int* p_int_list, *p_to_add;

    p_int_list = int_list;
    p_to_add   = to_add;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &list, 10, pool ) );

    for( i = 0; i < 5; i++ )
    {
        LCN_TEST( lcn_list_add( list, (p_int_list+i) ) );
    }
    LCN_TEST( lcn_list_insert( list, 2, p_to_add ) );

    CuAssertIntEquals( tc, 0, *((int*)lcn_list_get( list, 0 ) ) );
    CuAssertIntEquals( tc, 1, *((int*)lcn_list_get( list, 1 ) ) );
    CuAssertIntEquals( tc, 2, *((int*)lcn_list_get( list, 2 ) ) );
    CuAssertIntEquals( tc, 3, *((int*)lcn_list_get( list, 3 ) ) );
    CuAssertIntEquals( tc, 4, *((int*)lcn_list_get( list, 4 ) ) );
    CuAssertIntEquals( tc, 5, *((int*)lcn_list_get( list, 5 ) ) );

    LCN_TEST( lcn_list_insert( list, 6, ( p_to_add + 1 ) ));

    CuAssertIntEquals( tc, 6, *((int*)lcn_list_get( list, 6 ) ) );

    LCN_TEST( lcn_list_insert( list, 5, ( p_to_add + 2 ) ));

    CuAssertIntEquals( tc, 1000, *((int*)lcn_list_get( list, 5 ) ) );

    lcn_list_clear( list );

    CuAssertIntEquals( tc, 0, lcn_list_size( list ));

    for( i = 4; i >= 0; i-- )
    {
        LCN_TEST( lcn_list_add( list, (p_int_list+i) ) );
    }

    LCN_TEST( lcn_list_insert( list, 2, p_to_add ) );

    CuAssertIntEquals( tc, 5, *((int*)lcn_list_get( list, 0 ) ) );
    CuAssertIntEquals( tc, 4, *((int*)lcn_list_get( list, 1 ) ) );
    CuAssertIntEquals( tc, 2, *((int*)lcn_list_get( list, 2 ) ) );
    CuAssertIntEquals( tc, 3, *((int*)lcn_list_get( list, 3 ) ) );
    CuAssertIntEquals( tc, 1, *((int*)lcn_list_get( list, 4 ) ) );
    CuAssertIntEquals( tc, 0, *((int*)lcn_list_get( list, 5 ) ) );

    LCN_TEST( lcn_list_insert( list, 6, ( p_to_add + 1 ) ));

    CuAssertIntEquals( tc, 6, *((int*)lcn_list_get( list, 6 ) ) );

    LCN_TEST( lcn_list_insert( list, 5, ( p_to_add + 2 ) ));

    CuAssertIntEquals( tc, 1000, *((int*)lcn_list_get( list, 5 ) ) );

    apr_pool_destroy( pool );
}

static void
test_create_with_0( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_list_t* list;
    unsigned int i;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &list, 0, pool ) );

    for( i = 0; i < 100000; i++ )
    {
        LCN_TEST( lcn_list_add( list, list ));
    }

    apr_pool_destroy( pool );
}


static void
test_uniquify( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_list_t* list;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &list, 0, pool ) );

    LCN_TEST( lcn_list_add( list, "TEST1" ) );
    LCN_TEST( lcn_list_add( list, "TEST3" ) );
    LCN_TEST( lcn_list_add( list, "TEST2" ) );
    LCN_TEST( lcn_list_add( list, "TEST1" ) );
    LCN_TEST( lcn_list_add( list, "TEST1" ) );
    LCN_TEST( lcn_list_add( list, "TEST3" ) );

    LCN_TEST( lcn_list_uniquify( &list, list, pool ) );

    CuAssertIntEquals( tc, 3, lcn_list_size( list ) );

    apr_pool_destroy( pool );
}


CuSuite *make_lucene_list_suite (void)
{
    CuSuite *s= CuSuiteNew();
    SUITE_ADD_TEST(s, test_lucene_list);
    SUITE_ADD_TEST(s, test_remove );
    SUITE_ADD_TEST(s, test_sort );
    SUITE_ADD_TEST(s, test_insert );
    SUITE_ADD_TEST(s, test_create_with_0 );
    SUITE_ADD_TEST(s, test_uniquify );
    return s;
}
