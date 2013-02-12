#include "test_all.h"
#include "directory.h"
#include "compound_file_writer.h"
#include "compound_file_reader.h"
#include "lucene.h"
#include "stdlib.h"
#include "time.h"
#include "lcn_util.h"
#include "io_context.h"

const char *dir_test = "cf_test_dir";
const char *cf_name = "testfile.cfs";
const char *cf_static = "cf_test_dir/static";

static apr_status_t
create_sequence_file( lcn_directory_t *dir,
                      char *file_name,
                      int start,
                      int size,
                      apr_pool_t *pool )
{
    int i = 0;
    apr_status_t s = APR_SUCCESS;

    do
    {
        lcn_ostream_t *os;

        LCNCE( lcn_directory_create_output( dir, &os, file_name, pool ));

        for( i = 0; i <= size; i++ )
        {
            lcn_ostream_write_int( os, start);
            start++;
        }

        lcn_ostream_close( os );
    }
    while(0);

    return s;
}

static void
assert_same_stream ( CuTest* tc,
                     lcn_index_input_t* expected,
                     lcn_index_input_t* actual )
{
    CuAssertPtrNotNull(tc, expected);
    CuAssertPtrNotNull(tc, actual);

    unsigned int expected_size = lcn_index_input_size ( expected );
    unsigned int test_size = lcn_index_input_size ( actual );
    CuAssertIntEquals ( tc, expected_size, test_size );

    unsigned int expected_pos = lcn_index_input_file_pointer ( expected );
    unsigned int test_pos = lcn_index_input_file_pointer( actual );

    CuAssertIntEquals ( tc, expected_pos, test_pos );
}

static void
assert_same_streams ( CuTest* tc,
                      lcn_index_input_t* expected,
                      lcn_index_input_t* actual,
                      long seek_to )
{
    CuAssertPtrNotNull(tc, expected);
    CuAssertPtrNotNull(tc, actual);

    unsigned int expected_size = lcn_index_input_size ( expected );
    if ( seek_to >= 0 && seek_to < expected_size )
    {
        LCN_TEST ( lcn_index_input_seek( expected, seek_to ) );
        LCN_TEST ( lcn_index_input_seek( actual, seek_to ) );
        assert_same_stream ( tc, expected, actual );
    }
}

static void
assert_same_seek_behavior ( CuTest* tc,
                           lcn_index_input_t *expected,
                           lcn_index_input_t *actual )
{
    // seek to 0
    long point = 0;
    assert_same_streams ( tc, expected, actual, point );

    // seek to middle
    point = lcn_index_input_size ( expected ) / 2;
    assert_same_streams ( tc, expected, actual, point );

    // seek to end - 2
    point = lcn_index_input_size ( expected ) - 2;
    assert_same_streams ( tc, expected, actual, point );

    // seek to end - 1
    point = lcn_index_input_size ( expected ) - 1;
    assert_same_streams ( tc, expected, actual, point );

    // seek to the end
    point = lcn_index_input_size ( expected );
    assert_same_streams ( tc, expected, actual, point );

    // seek past end
    point = lcn_index_input_size ( expected ) + 1;
    assert_same_streams ( tc, expected, actual, point );
}

static apr_status_t
create_random_file ( CuTest *tc,
                     lcn_directory_t* dir,
                     char* name,
                     int size,
                     apr_pool_t* pool)
{
    apr_status_t s = APR_SUCCESS;
    int i = 0;
    lcn_ostream_t* os;

    srand( ( unsigned int ) time(NULL) );

    do
    {
        LCN_TEST( lcn_directory_create_output( dir, &os, name, pool ) );

        for ( i = 0; i < size; i++ )
        {
            unsigned int data = ( unsigned int ) rand();
            CuAssertTrue( tc, data > 0 );
            lcn_ostream_write_long( os, data );
        }

        lcn_ostream_close ( os );
    }
    while(0);

    return s;
}

static void
test_single_file(CuTest* tc)
{
    char data[] = {0, 1, 10, 100};
    int i = 0, data_size = 4;
    unsigned int size = 0;

    apr_pool_t *pool;
    apr_pool_create( &pool, NULL );

    for(i = 0; i < data_size; i++ )
    {
        lcn_directory_t *dir;
        char *seq_file = "seq_file";

        LCN_TEST( lcn_fs_directory_create( &dir, dir_test, LCN_TRUE, pool ) );
        LCN_TEST( create_sequence_file(dir, seq_file, 0, data[i], pool ) );
        // add seq_file to cf writer.
        lcn_compound_file_writer_t *cfw;

        LCN_TEST( lcn_compound_file_writer_create( &cfw, dir, cf_name, pool ) );

        LCN_TEST( lcn_compound_file_writer_add_file( cfw, seq_file ) );
        LCN_TEST( lcn_compound_file_writer_add_file( cfw, seq_file ) );

        size = lcn_compound_file_writer_entries_size( cfw );
        CuAssertIntEquals(tc, 1, size );

        LCN_TEST( lcn_compound_file_writer_close( cfw ) );

        lcn_compound_file_reader_t *cfr = NULL;
        LCN_TEST( lcn_compound_file_reader_create( &cfr, dir, cf_name, pool ) );

        size = lcn_compound_file_reader_entries_size ( cfr );
        CuAssertIntEquals( tc, 1, size );

        lcn_index_input_t *expected = NULL;
        lcn_index_input_t *actual = NULL;

        LCN_TEST( lcn_directory_open_input( dir, &expected, seq_file, LCN_IO_CONTEXT_READONCE, pool ) );
        LCN_TEST( lcn_compound_file_reader_open_input( cfr, &actual, seq_file ) );

        assert_same_stream ( tc, expected, actual );
        assert_same_seek_behavior ( tc, expected, actual );

        LCN_TEST ( lcn_index_input_close( expected ) );
        LCN_TEST ( lcn_index_input_close( actual ) );
        LCN_TEST ( lcn_compound_file_reader_close ( cfr ) );

        CuAssertTrue ( tc, lcn_compound_file_reader_is_open ( cfr ) == LCN_FALSE );

        delete_files( tc, dir_test );
    }
}

static void
test_two_files ( CuTest* tc )
{
    lcn_directory_t *dir;

    apr_pool_t *pool;
    apr_pool_create( &pool, NULL );

    LCN_TEST( lcn_fs_directory_create( &dir, dir_test, LCN_TRUE, pool ) );
    LCN_TEST( create_sequence_file(dir, "seq_file1", 0, 15, pool ) );
    LCN_TEST( create_sequence_file(dir, "seq_file2", 0, 114, pool ) );

    lcn_compound_file_writer_t *cfw;
    LCN_TEST( lcn_compound_file_writer_create( &cfw, dir, "test_two_files.csf", pool ) );
    LCN_TEST( lcn_compound_file_writer_add_file( cfw, "seq_file1" ) );
    LCN_TEST( lcn_compound_file_writer_add_file( cfw, "seq_file2" ) );
    LCN_TEST( lcn_compound_file_writer_close( cfw ) );

    lcn_compound_file_reader_t *cfr;
    LCN_TEST( lcn_compound_file_reader_create( &cfr, dir, "test_two_files.csf", pool ) );

    /**
     * Open seq_file1 and test it.
     */
    lcn_index_input_t *expected = NULL;
    lcn_index_input_t *actual = NULL;

    LCN_TEST( lcn_directory_open_input( dir, &expected, "seq_file1", LCN_IO_CONTEXT_READONCE, pool ) );
    LCN_TEST( lcn_compound_file_reader_open_input( cfr, &actual, "seq_file1" ) );

    assert_same_stream ( tc, expected, actual);
    assert_same_seek_behavior ( tc, expected, actual );

    LCN_TEST ( lcn_index_input_close( expected ) );
    LCN_TEST ( lcn_index_input_close( actual ) );

    /**
     * Open seq_file2 and test it.
     */
    LCN_TEST( lcn_directory_open_input( dir, &expected, "seq_file2", LCN_IO_CONTEXT_READONCE, pool ) );
    LCN_TEST( lcn_compound_file_reader_open_input( cfr, &actual, "seq_file2" ) );

    assert_same_stream ( tc, expected, actual);
    assert_same_seek_behavior ( tc, expected, actual );

    LCN_TEST ( lcn_index_input_close( expected ) );
    LCN_TEST ( lcn_index_input_close( actual ) );

    LCN_TEST ( lcn_compound_file_reader_close ( cfr ) );

    delete_files( tc, dir_test );
}

static void
test_random_files( CuTest* tc )
{
    // Setup the test segment
    char* segment = "seq_file";
    int chunk = 1024, i = 0; // internal buffer size used by the stream
    lcn_directory_t *dir;
    char *data[] = {
            ".zero", ".one", ".ten", ".hundred", ".big1", ".big2", ".big3",
            ".big4", ".big5", ".big6", ".big7"
    };
    int data_size = 11;
    lcn_compound_file_writer_t *cfw;
    lcn_compound_file_reader_t *cfr;

    apr_pool_t *pool;
    apr_pool_create( &pool, NULL );

    LCN_TEST( lcn_fs_directory_create( &dir, dir_test, LCN_TRUE, pool ) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".zero", NULL), 0, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".one", NULL), 1, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".ten", NULL), 10, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".hundred", NULL), 100, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".big1", NULL), chunk, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".big2", NULL), chunk - 1, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".big3", NULL), chunk + 1, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".big4", NULL), 3 * chunk, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".big5", NULL), 3 * chunk - 1, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".big6", NULL), 3 * chunk + 1, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".big7", NULL), 1000 * chunk, pool) );

    // Setup extraneous files
    LCN_TEST( create_random_file(tc, dir, "onetwothree", 100, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".notIn", NULL), 50, pool) );
    LCN_TEST( create_random_file(tc, dir, apr_pstrcat(pool, segment, ".notIn2", NULL), 51, pool) );

    lcn_compound_file_writer_t *cfw;
    LCN_TEST( lcn_compound_file_writer_create ( &cfw, dir, cf_name, pool ) );

    char *data[] = {
            ".zero", ".one", ".ten", ".hundred", ".big1", ".big2", ".big3",
            ".big4", ".big5", ".big6", ".big7"
    };
    int data_size = 11;

    for (i = 0; i < data_size; i++)
    {
        lcn_compound_file_writer_add_file( cfw, apr_pstrcat(pool, segment, data[i], NULL ) );
    }
    lcn_compound_file_writer_close(cfw);

    lcn_compound_file_reader_t *cfr;
    LCN_TEST( lcn_compound_file_reader_create ( &cfr, dir, cf_name, pool ) );

    for (i = 0; i < data_size; i++)
    {
        lcn_index_input_t *expected = NULL;
        lcn_index_input_t *actual = NULL;

        LCN_TEST( lcn_directory_open_input( dir, &expected, apr_pstrcat(pool, segment, data[i], NULL ), LCN_IO_CONTEXT_READONCE, pool ) );
        LCN_TEST( lcn_compound_file_reader_open_input( cfr, &actual, apr_pstrcat(pool, segment, data[i], NULL ) ) );

        assert_same_stream ( tc, expected, actual);
        assert_same_seek_behavior ( tc, expected, actual );

        LCN_TEST ( lcn_index_input_close( expected ) );
        LCN_TEST ( lcn_index_input_close( actual ) );
    }

    LCN_TEST ( lcn_compound_file_reader_close ( cfr ) );

    delete_files( tc, dir_test );
}

static void
test_file_exists_in_cfs( CuTest* tc )
{
    const char *index = "_1.cfs";
    lcn_compound_file_reader_t *cfr;
    lcn_directory_t *cf_dir;
    apr_pool_t *pool;

    apr_pool_create( &pool, NULL );

    LCN_TEST(lcn_fs_directory_create(&cf_dir, cf_static, LCN_FALSE, pool ));
    LCN_TEST( lcn_compound_file_reader_create(&cfr, cf_dir, index, pool) );

    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.tis") == LCN_TRUE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.frq") == LCN_TRUE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.tii") == LCN_TRUE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.fnm") == LCN_TRUE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.prx") == LCN_TRUE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.fdt") == LCN_TRUE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.fdx") == LCN_TRUE);

    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.aa") == LCN_FALSE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.bb") == LCN_FALSE);
    CuAssertTrue (tc, lcn_compound_file_reader_file_name_exists(cfr, "_1.cc") == LCN_FALSE);
}

static void
test_entries_as_list ( CuTest* tc )
{
    const char *index = "_1.cfs";
    lcn_compound_file_reader_t *cfr;
    lcn_directory_t *cf_dir;
    lcn_list_t *entries;
    apr_pool_t *pool;

    apr_pool_create( &pool, NULL );

    LCN_TEST( lcn_fs_directory_create(&cf_dir, cf_static, LCN_FALSE, pool ) );
    LCN_TEST( lcn_compound_file_reader_create(&cfr, cf_dir, index, pool) );
    LCN_TEST( lcn_compound_file_reader_entries_as_list(cfr, &entries, pool) );

    CuAssertTrue( tc, lcn_list_size(entries) == 7);
}

CuSuite *make_compound_file_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_single_file);
    SUITE_ADD_TEST(s, test_two_files);
    SUITE_ADD_TEST(s, test_random_files);
    SUITE_ADD_TEST(s, test_file_exists_in_cfs);
    SUITE_ADD_TEST(s, test_entries_as_list);

    return s;
}
