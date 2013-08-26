#include "test_all.h"
#include "util/crc32.h"

static void
test_crc32( CuTest* tc )
{
    apr_pool_t *pool = NULL;
    
    do
    {
        lcn_crc32_t* crc;
        
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_crc32_create( &crc, pool ) );
        lcn_crc32_update( crc, "hallo", strlen("hallo") );
        CuAssertIntEquals( tc, 3111268817, crc->crc);
        lcn_crc32_reset( crc );
        CuAssertIntEquals( tc, crc->crc, 0 );
        lcn_crc32_update( crc, "\366\344\374\326\304\334\337", strlen("\366\344\374\326\304\334\337"));
        CuAssertIntEquals( tc, 883665370, crc->crc);
        lcn_crc32_reset( crc );
        CuAssertIntEquals( tc, crc->crc, 0 );
        lcn_crc32_update( crc, "!123456789_:;*\366\344\374\326\304\334\337", strlen("!123456789_:;*\366\344\374\326\304\334\337") );
        CuAssertIntEquals( tc, 3269394715, crc->crc );
    }
    while(0);
    
    if ( pool != NULL )
    {
        apr_pool_destroy( pool );
    }
}

CuSuite *
make_50_crc32_suite (void)
{
    CuSuite *s= CuSuiteNew();

#if 1
    SUITE_ADD_TEST(s, test_crc32);
#endif

    return s;
}
