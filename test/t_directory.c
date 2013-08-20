#include "test_all.h"
#include "lcn_util.h"
#include "io_context.h"

static int
check_in_list( lcn_list_t *list, const char *name )
{
    unsigned int i;

    for ( i = 0; i < lcn_list_size( list ); i++ )
    {
        char *s;
        s = lcn_list_get( list, i );

        if ( 0 == strcmp( s, name ))
        {
            return 1;
        }
    }

    return 0;
}



static void
do_tests( CuTest* tc, lcn_directory_t *t_dir, apr_pool_t *pool )
{
    apr_pool_t *child_pool;
    apr_pool_t *str_pool;
    lcn_index_output_t *out;
    lcn_index_input_t *in;
    char *str = NULL;
    unsigned int len;
    lcn_bool_t file_exists;
    int i;
    int lstrlen = 110000;
    char *long_string = (char*) apr_palloc( pool, lstrlen );
    lcn_list_t *file_list;

    for( i = 0; i < lstrlen; i++ )
    {
        long_string[i] = 'x';
    }

    long_string[lstrlen-1] = '\0';
    LCN_TEST( apr_pool_create( &str_pool, pool ) );

    LCN_TEST( lcn_directory_create_output( t_dir, &out, "segments", pool ));
    lcn_index_output_write_string( out, "TEST" );
    LCN_TEST( lcn_index_output_close( out ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "segments", LCN_IO_CONTEXT_READONCE, pool ) );
    LCN_TEST( lcn_index_input_read_string( in, &str, &len, pool ) );
    CuAssertStrEquals(tc, str, "TEST" );
    LCN_TEST( lcn_index_input_close( in ) );
    CuAssertTrue(tc, APR_SUCCESS != lcn_directory_open_input( t_dir, &in, "xxx", LCN_IO_CONTEXT_READONCE, pool ));

    LCN_TEST( lcn_directory_create_segment_file( t_dir, &out, "segments", ".abc", pool ));
    LCN_TEST( lcn_index_output_write_string( out, "TEST abc" ) );
    LCN_TEST( lcn_index_output_close( out ) );
    LCN_TEST( lcn_directory_open_segment_file( t_dir, &in, "segments", ".abc", pool ) );
    LCN_TEST( lcn_index_input_read_string( in, &str, &len, pool ) );
    CuAssertStrEquals(tc, str, "TEST abc" );
    LCN_TEST( lcn_index_input_close( in ) );

    LCN_TEST( lcn_directory_file_exists( t_dir, "segments.abc", &file_exists ) );
    CuAssertIntEquals(tc, file_exists, LCN_TRUE );

    LCN_TEST( lcn_directory_file_exists( t_dir, "segments.abd", &file_exists ) );
    CuAssertIntEquals(tc, file_exists, LCN_FALSE );

    LCN_TEST( lcn_directory_list( t_dir, &file_list, pool ) );
    CuAssertIntEquals(tc, 2, lcn_list_size( file_list ) );

    CuAssertIntEquals(tc, 1, check_in_list( file_list, "segments" ));
    CuAssertIntEquals(tc, 1, check_in_list( file_list, "segments.abc" ));

    LCN_TEST( lcn_directory_delete_file( t_dir, "segments" ) );
    CuAssertTrue(tc, APR_SUCCESS != lcn_directory_open_input( t_dir, &in, "segments", LCN_IO_CONTEXT_READONCE, pool ));

    /* tests memory management: first write data into dir using a pool */
    LCN_TEST( apr_pool_create(&child_pool, pool ) );
    LCN_TEST( lcn_directory_create_output( t_dir, &out, "afile", child_pool ));
    LCN_TEST( lcn_index_output_write_string( out, "some test data" ) );
    LCN_TEST( lcn_index_output_close( out ) );
    apr_pool_destroy( child_pool );

    /* now check whether the data is still available */
    LCN_TEST( apr_pool_create(&child_pool, pool ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "afile", LCN_IO_CONTEXT_READONCE, child_pool ));
    LCN_TEST( lcn_index_input_read_string( in, &str, &len, str_pool ) );
    LCN_TEST( lcn_index_input_close( in ) );
    apr_pool_destroy( child_pool );
    CuAssertStrEquals(tc, "some test data", str );

    LCN_TEST( lcn_directory_rename_file( t_dir, "afile", "thefile" ) );

    LCN_TEST( lcn_directory_file_exists( t_dir, "afile", &file_exists ) );
    CuAssertIntEquals(tc, file_exists, LCN_FALSE );

    LCN_TEST( lcn_directory_file_exists( t_dir, "thefile", &file_exists ) );
    CuAssertIntEquals(tc, file_exists, LCN_TRUE );

    /* try writing/reading a big file */

    LCN_TEST( apr_pool_create( &child_pool, pool ) );
    LCN_TEST( lcn_directory_create_output( t_dir, &out, "thefile", child_pool ) );

    for( i = 0; i<60; i++ )
    {
        LCN_TEST( lcn_index_output_write_vint( out, i ) );
        LCN_TEST( lcn_index_output_write_string( out, long_string ) );
    }

    LCN_TEST( lcn_index_output_close( out ) );
    apr_pool_destroy( child_pool );

    LCN_TEST( apr_pool_create( &child_pool, pool ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "thefile", LCN_IO_CONTEXT_READONCE, child_pool ));

    for( i = 0; i < 60; i++ )
    {
        unsigned int j;
        LCN_TEST( lcn_index_input_read_vint( in,
                                         (unsigned int*)&j ) );
        CuAssertIntEquals(tc, i, j );
        LCN_TEST( lcn_index_input_read_string( in, &str, &len, child_pool ) );
        CuAssertStrEquals(tc, str, long_string );
    }

    LCN_TEST( lcn_directory_delete_file( t_dir, "thefile" ) );

    LCN_TEST( lcn_index_input_close( in ) );
    apr_pool_destroy( child_pool );

    LCN_TEST( lcn_directory_close( t_dir ) );
    apr_pool_destroy( str_pool );
}

static void
test_ram_create(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_ram_directory_create( &dir, pool ) );

    do_tests( tc, dir, pool );

    apr_pool_destroy( pool );
}

static void
test_create(CuTest* tc)
{
    apr_pool_t *pool;
    apr_status_t s;
    lcn_directory_t *test_dir;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    s = lcn_fs_directory_create( &test_dir, "there_is_no_such_directory", LCN_FALSE, pool );
    CuAssertTrue(tc, s != APR_SUCCESS );

    CuAssertIntEquals( tc, LCN_ERR_CANNOT_OPEN_DIR,
                       lcn_fs_directory_create( &test_dir, "Makefile", LCN_FALSE, pool ));

    CuAssertIntEquals( tc, LCN_ERR_CANNOT_OPEN_DIR,
                       lcn_fs_directory_create( &test_dir, "Makefile", LCN_TRUE, pool ));

    CuAssertIntEquals( tc, APR_SUCCESS,
                       lcn_fs_directory_create( &test_dir, "new_dir", LCN_TRUE, pool ));

    CuAssertTrue( tc, lcn_directory_is_open( test_dir ) );
    CuAssertStrEquals(tc, test_dir->name, "new_dir" );
    LCN_TEST( lcn_directory_close( test_dir ));

    apr_dir_remove( "new_dir", pool );
    apr_pool_destroy( pool );
}

static void
test_create_file(CuTest* tc)
{
    lcn_directory_t *t_dir;
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    delete_files( tc, "new_dir" );
    LCN_TEST( lcn_fs_directory_create( &t_dir, "new_dir", LCN_TRUE, pool ) );

    do_tests( tc, t_dir, pool );

    apr_dir_remove( "new_dir", pool );
    apr_pool_destroy( pool );
}


CuSuite *make_directory_suite (void)
{
    CuSuite *s = CuSuiteNew();

    SUITE_ADD_TEST(s, test_create);
    SUITE_ADD_TEST(s, test_create_file);
    SUITE_ADD_TEST(s, test_ram_create);

    return s;
}
