#include "test_all.h"
#include "index_input.h"
#include "io_context.h"

static lcn_ram_file_t *
make_ram_file ( char *file_name, CuTest *tc, apr_pool_t *pool )
{
    lcn_directory_t *t_dir;
    lcn_ram_file_t *f;
    lcn_index_input_t *in;
    lcn_index_output_t *out;
    char *buf;
    unsigned int len;

    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, pool ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, file_name, LCN_IO_CONTEXT_READONCE, pool ) );
    LCN_TEST( lcn_ram_file_create( &f, pool ) );
    LCN_TEST( lcn_ram_ostream_create( &out, f, pool ) );

    len = lcn_index_input_size( in );
    buf = (char *) apr_palloc( pool, sizeof(char) * len );

    LCN_TEST( lcn_index_input_read_bytes( in, buf, 0, &len ) );
    len = lcn_index_input_size( in );
    LCN_TEST( lcn_ostream_write_bytes( out, buf, len ) );

    LCN_TEST( lcn_ostream_close( out ) );
    LCN_TEST( lcn_index_input_close( in ) );

    return f;
}

static void
test_read_int(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_ram_file_t *f ;
    lcn_directory_t *t_dir;
    lcn_index_input_t *in;
    int len;
    int result;

    apr_pool_create( &pool, NULL );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, pool ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "segments", LCN_IO_CONTEXT_READONCE, pool ) );
    len = lcn_index_input_size( in );

    CuAssertPtrNotNull(tc, in );
    LCN_TEST( lcn_index_input_read_int( in, &result ) );
    CuAssertIntEquals(tc, 21, result );
    LCN_TEST( lcn_index_input_read_int( in, &result ));
    CuAssertIntEquals(tc, 1,  result );

    LCN_TEST( lcn_index_input_close( in ) );

    f = make_ram_file( "segments", tc, pool );
    LCN_TEST( lcn_ram_input_stream_create( &in, "segments", f, pool ) );

    CuAssertTrue(tc, len == lcn_index_input_size( in ) );
    CuAssertTrue(tc, in != NULL);
    LCN_TEST( lcn_index_input_read_int( in, &result ));
    CuAssertIntEquals(tc, 21, result );
    LCN_TEST( lcn_index_input_read_int( in, &result ));
    CuAssertIntEquals(tc, 1, result );

    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );

    apr_pool_destroy( pool );
}

#define CHECK_VINT( x )                                                  \
    LCN_TEST( lcn_index_input_read_vint( in, &result ));                \
    CuAssertIntEquals(tc, x, result )


static void
test_read_vint_impl (   CuTest* tc, lcn_index_input_t *in )
{

    unsigned int result;
    CuAssertPtrNotNull(tc, in );
    CHECK_VINT( 1 );
    CHECK_VINT( 10 );
    CHECK_VINT( 100 );
    CHECK_VINT( 1000 );
    CHECK_VINT( 10000 );
    CHECK_VINT( 100000 );
    CHECK_VINT( 1000000 );
    CHECK_VINT( 10000000 );
    CHECK_VINT( 100000000 );
}

static void
test_read_vint(CuTest* tc)
{
    apr_pool_t *p;
    lcn_directory_t *t_dir;
    lcn_index_input_t *in;
    lcn_ram_file_t *f;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, p ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "vint.dat", LCN_IO_CONTEXT_READONCE, p ) );
    test_read_vint_impl(tc, in );
    LCN_TEST( lcn_index_input_close( in ) );
    f = make_ram_file( "vint.dat", tc, p );
    LCN_TEST( lcn_ram_input_stream_create( &in, "vint.dat", f, p ) );
    test_read_vint_impl(tc, in );
    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );
    apr_pool_destroy( p );
}

#define CHECK_LONG( x )                                                 \
    LCN_TEST( lcn_index_input_read_long( in, &result ));               \
    CuAssertTrue(tc, x == result )


static void
test_read_long_impl (CuTest* tc, lcn_index_input_t *in )
{
    apr_int64_t result;
    CuAssertPtrNotNull(tc, in );
    CHECK_LONG( 1 );
    CHECK_LONG( 100 );
    CHECK_LONG( 0 );
    CHECK_LONG( 2147483647 );
}

static void
test_read_long(CuTest* tc)
{
    apr_pool_t *p;
    lcn_directory_t *t_dir;
    lcn_index_input_t *in;
    lcn_ram_file_t *f;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, p ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in,  "long.dat", LCN_IO_CONTEXT_READONCE, p ) );
    test_read_long_impl (tc, in );
    LCN_TEST( lcn_index_input_close( in ) );
    f = make_ram_file( "long.dat", tc, p );
    LCN_TEST( lcn_ram_input_stream_create( &in, "long.dat", f, p ) );
    test_read_long_impl ( tc, in );
    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );
    apr_pool_destroy( p );
}

static void
test_read_string(CuTest* tc)
{
    apr_pool_t *p;
    char *buf;
    unsigned int len;
    lcn_directory_t *t_dir;
    lcn_index_input_t *in;
    lcn_ram_file_t *f;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, p ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "string.dat", LCN_IO_CONTEXT_READONCE, p ) );

    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertTrue(tc, len == 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation" );
    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: \366, auml: \344, uuml: \374, Ouml: \326, Auml: \304, Uuml: \334, szlig: \337" );
    LCN_TEST( lcn_index_input_close( in ) );

    f = make_ram_file( "string.dat", tc, p );
    LCN_TEST( lcn_ram_input_stream_create( &in, "string.dat", f, p ) );
    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertTrue(tc, len == 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation" );
    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: \366, auml: \344, uuml: \374, Ouml: \326, Auml: \304, Uuml: \334, szlig: \337" );
    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );
    apr_pool_destroy( p );
}

static void
test_read_bytes(CuTest* tc)
{
    apr_pool_t *p;
    char bytes[27];
    unsigned char byte;
    lcn_directory_t *t_dir;
    lcn_index_input_t *in;
    lcn_ram_file_t *f;
    unsigned int len;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, p ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "string.dat", LCN_IO_CONTEXT_READONCE, p ) );

    len = 26;

    LCN_TEST( lcn_index_input_read_byte( in, &byte ) );
    LCN_TEST( lcn_index_input_read_bytes( in, bytes, 0, &len ) );

    CuAssertIntEquals( tc, len, 26 );
    bytes[26] = 0;

    CuAssertStrEquals(tc, bytes, "Apache Software Foundation" );
    LCN_TEST( lcn_index_input_read_byte( in, &byte ) );

    len = 8;
    LCN_TEST( lcn_index_input_read_bytes( in, bytes, 7, &len ) );
    CuAssertStrEquals(tc, bytes, "Apache Teste So Foundation" );
    LCN_TEST( lcn_index_input_close( in ) );

    f = make_ram_file( "string.dat", tc, p );
    LCN_TEST( lcn_ram_input_stream_create( &in, "string.dat", f, p ) );
    LCN_TEST( lcn_index_input_read_byte( in, &byte ) );

    len = 26;
    LCN_TEST( lcn_index_input_read_bytes( in, bytes, 0, &len ) );
    bytes[26] = 0;

    CuAssertStrEquals(tc, bytes, "Apache Software Foundation");
    LCN_TEST( lcn_index_input_read_byte( in, &byte ) );
    len = 8;
    LCN_TEST( lcn_index_input_read_bytes( in, bytes, 7, &len ) );
    CuAssertStrEquals(tc, bytes, "Apache Teste So Foundation" );

    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );

    apr_pool_destroy( p );
}

static void
test_seek(CuTest* tc)
{
    apr_pool_t *p;
    char bytes[20];
    lcn_directory_t *t_dir;
    lcn_index_input_t *in;
    lcn_ram_file_t *f;
    unsigned int len;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, p ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "string.dat", LCN_IO_CONTEXT_READONCE, p ) );

    len = 10;
    LCN_TEST( lcn_index_input_seek(in, 10) );
    LCN_TEST( lcn_index_input_read_bytes( in, bytes, 0, &len ) );
    bytes[10] = 0;
    CuAssertStrEquals(tc, bytes, "ftware Fou" );
    LCN_TEST( lcn_index_input_close( in ) );

    f = make_ram_file( "string.dat", tc, p );
    LCN_TEST( lcn_ram_input_stream_create( &in, "string.dat", f, p ) );
    LCN_TEST( lcn_index_input_seek(in, 10) );
    LCN_TEST( lcn_index_input_read_bytes( in, bytes, 0, &len ) );
    bytes[10] = 0;
    CuAssertStrEquals(tc, bytes, "ftware Fou" );
    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );
    apr_pool_destroy( p );
}

static void
test_clone(CuTest* tc)
{
    apr_pool_t *p;
    lcn_index_input_t *clone;
    char *buf;
    unsigned int len;
    lcn_directory_t *t_dir;
    lcn_ram_file_t *f;
    lcn_index_input_t *in;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, p ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "string.dat", LCN_IO_CONTEXT_READONCE, p ) );
    CuAssertTrue(tc, in != NULL);
    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertIntEquals(tc, len, 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation");
    LCN_TEST( lcn_index_input_clone( in, &clone, p ) );
    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: \366, auml: \344, uuml: \374, Ouml: \326, Auml: \304, Uuml: \334, szlig: \337" );
    LCN_TEST( lcn_index_input_read_string( clone, &buf, &len, p ) );
    CuAssertIntEquals(tc, len, 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation");
    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_index_input_close( clone ) );

    f = make_ram_file( "string.dat", tc, p );
    LCN_TEST( lcn_ram_input_stream_create( &in, "string.dat", f, p ) );
    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertIntEquals(tc, len, 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation");
    LCN_TEST( lcn_index_input_clone( in, &clone, p ) );
    LCN_TEST( lcn_index_input_read_string( in, &buf, &len, p ) );
    CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: \366, auml: \344, uuml: \374, Ouml: \326, Auml: \304, Uuml: \334, szlig: \337");
    LCN_TEST( lcn_index_input_read_string( clone, &buf, &len, p ) );
    CuAssertIntEquals(tc, 26, len );
    CuAssertStrEquals(tc, "Apache Software Foundation", buf );

    LCN_TEST( lcn_index_input_close( in ) );
    LCN_TEST( lcn_index_input_close( clone ) );
    LCN_TEST( lcn_directory_close( t_dir ) );

    apr_pool_destroy( p );
}

static void
test_apr_off_t(CuTest* tc)
{
    apr_off_t i = -1;
    unsigned int j = -1;

    CuAssertTrue(tc, i < 0);
    CuAssertTrue(tc, j > 0);
}


CuSuite *
make_input_stream_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_read_vint);
    SUITE_ADD_TEST(s, test_read_long);
    SUITE_ADD_TEST(s, test_read_int);
    SUITE_ADD_TEST(s, test_read_string);
    SUITE_ADD_TEST(s, test_read_bytes);
    SUITE_ADD_TEST(s, test_seek);
    SUITE_ADD_TEST(s, test_clone);
    SUITE_ADD_TEST(s, test_apr_off_t);

    return s;
}
