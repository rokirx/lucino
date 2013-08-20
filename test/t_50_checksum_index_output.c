#include "test_all.h"
#include "../include/lucene.h"

static void
test_create( CuTest* tc )
{
    lcn_ostream_t *os = NULL;
    lcn_ostream_t *cio = NULL;
    lcn_directory_t *dir = NULL;
    apr_pool_t *pool = NULL;
    
    do
    {
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_directory_create( &dir, "05_checksum_index_output_test", LCN_TRUE, pool ) );
        LCN_TEST( lcn_directory_create_output( dir, &os, "test_create", pool ))
        LCN_TEST( lcn_checksum_index_output_create( &cio, os, pool ) );
        CuAssertIntEquals( tc, 0, lcn_checksum_index_output_get_checksum( cio ) );
    }
    while(0);
    
    if ( pool != NULL )
    {
        apr_pool_destroy( pool );
    }
}

CuSuite *
make_50_checksum_index_output_suite (void)
{
    CuSuite *s= CuSuiteNew();

#if 1
    SUITE_ADD_TEST(s, test_create);
#endif

    return s;
}

