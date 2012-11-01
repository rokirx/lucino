#include "test_all.h"
#include "segment_infos.h"
#include "lcn_search.h"
#include "lcn_analysis.h"

#define SETUP() 
    {           \
        dir = newDirectory();    \
    DocHelper.setupDoc(testDoc); \
    SegmentInfoPerCommit info = DocHelper.writeDoc(random(), dir, testDoc); \
    reader = new SegmentReader(info, DirectoryReader.DEFAULT_TERMS_INDEX_DIVISOR, IOContext.READ);
    }

static void
test(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    SETUP();
    
#if 0

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_ram_directory_create( &dir, pool ) );

    {
        apr_pool_t *si_pool;
        lcn_segment_infos_t *infos;
        char *name;

        LCN_TEST( apr_pool_create( &si_pool, pool ) );
        LCN_TEST( lcn_segment_infos_create( &infos, si_pool ) );
        CuAssertIntEquals(tc, 0, lcn_segment_infos_size( infos ) );

        /**
         * According to APR-Coding guidelines, input values to functions
         * are not checked
         * LCN_ERR( lcn_segment_infos_get( infos, &info, 0 ), LCN_ERR_INDEX_OUT_OF_RANGE );
         */

        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_0", name );
        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_1", name );
        CuAssertIntEquals(tc, 0, lcn_segment_infos_version( infos ) );
        LCN_TEST( lcn_segment_infos_write( infos, dir ) );
        CuAssertIntEquals(tc, 1, lcn_segment_infos_version( infos ) );

        apr_pool_destroy( si_pool );
    }

    {
        apr_pool_t *si_pool;
        lcn_segment_infos_t *infos;
        lcn_segment_info_t *info;
        char *name;

        LCN_TEST( apr_pool_create( &si_pool, pool ) );
        LCN_TEST( lcn_segment_infos_create( &infos, si_pool ) );
        LCN_TEST( lcn_segment_infos_read_directory( infos, dir ) );
        CuAssertIntEquals(tc, 0, lcn_segment_infos_size( infos ) );
        CuAssertIntEquals(tc, 1, lcn_segment_infos_version( infos ) );
        /**
         * LCN_ERR( lcn_segment_infos_get( infos, &info, 0 ), LCN_ERR_INDEX_OUT_OF_RANGE );
         */
        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_2", name );
        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_3", name );
        LCN_TEST( lcn_segment_infos_add_info( infos, dir, name, 23 ));
        CuAssertIntEquals(tc, 1, lcn_segment_infos_size( infos ) );
        LCN_TEST( lcn_segment_infos_get( infos, &info, 0 ) );
        CuAssertStrEquals(tc, "_3", lcn_segment_info_name( info ) );
        LCN_TEST( lcn_segment_infos_write( infos, dir ) );
        CuAssertIntEquals(tc, 2, lcn_segment_infos_version( infos ) );

        apr_pool_destroy( si_pool );
    }

    {
        apr_pool_t *si_pool;
        lcn_segment_infos_t *infos;
        lcn_segment_info_t *info;
        char *name;

        LCN_TEST( apr_pool_create( &si_pool, pool ) );
        LCN_TEST( lcn_segment_infos_create( &infos, si_pool ) );
        LCN_TEST( lcn_segment_infos_read_directory( infos, dir ) );
        CuAssertIntEquals(tc, 1, lcn_segment_infos_size( infos ) );
        CuAssertIntEquals(tc, 2, lcn_segment_infos_version( infos ) );
        /*
          LCN_ERR( lcn_segment_infos_get( infos, &info, 1 ), LCN_ERR_INDEX_OUT_OF_RANGE );
        */
        LCN_TEST( lcn_segment_infos_get( infos, &info, 0 ) );
        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_4", name );
        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_5", name );
        LCN_TEST( lcn_segment_infos_add_info( infos, dir, name, 12 ));
        CuAssertIntEquals(tc, 2, lcn_segment_infos_size( infos ) );
        LCN_TEST( lcn_segment_infos_get( infos, &info, 1 ) );
        CuAssertStrEquals(tc, "_5", lcn_segment_info_name( info ) );
        LCN_TEST( lcn_segment_infos_write( infos, dir ) );
        CuAssertIntEquals(tc, 3, lcn_segment_infos_version( infos ) );

        apr_pool_destroy( si_pool );
    }
#endif

}


CuSuite *
make_segment_reader_suite(void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, test );

    return s;
}
