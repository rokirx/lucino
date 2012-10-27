#include "test_all.h"
#include "lcn_store.h"

#include "istream.h"
#include "ostream.h"

static void
test_create_ostream_impl ( CuTest *tc, lcn_ostream_t *out )
{
    CuAssertTrue(tc, out->isOpen == 1);
    CuAssertTrue(tc, out->buffer_position == 0);
    CuAssertTrue(tc, out->buffer_start == 0);
}

static void
TestCuCreateOutputStream(CuTest* tc)
{
    lcn_ostream_t *out;
    lcn_ram_file_t *file;

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_ostream_create( &out, "test_file", pool ) );
        CuAssertStrEquals( tc, out->name, "test_file" );
        test_create_ostream_impl(tc, out );
        LCN_TEST( lcn_ostream_close( out ) );
        apr_file_remove( "test_file", pool );
        apr_pool_destroy( pool );
    }

    {
        apr_pool_t *ram_file_pool, *pool;
        LCN_TEST( apr_pool_create( &ram_file_pool, main_pool ) );
        LCN_TEST( lcn_ram_file_create( &file, ram_file_pool ) );

        {
            LCN_TEST( apr_pool_create( &pool, main_pool ) );
            LCN_TEST( lcn_ram_ostream_create( &out, file, pool ) );
            test_create_ostream_impl( tc, out );
            LCN_TEST( lcn_ostream_close( out ) );
            apr_pool_destroy( pool );
        }

        apr_pool_destroy( ram_file_pool );
    }
}

static void
TestCuWriteByte(CuTest* tc)
{
    lcn_istream_t *in;
    lcn_ram_file_t *file;
    lcn_ostream_t *out;
    apr_pool_t *pool;
    unsigned char byte;

    {
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_ostream_create( &out, "test_file", pool ) );
        LCN_TEST( lcn_ostream_write_byte( out, (unsigned char) 250 ) );
        LCN_TEST( lcn_ostream_close( out ) );
        apr_pool_destroy( pool );
    }

    {
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_istream_create( &in, "test_file", pool ) );
        LCN_TEST( lcn_istream_read_byte( in, &byte ) );
        CuAssertIntEquals(tc, byte, 250 );
        LCN_TEST( lcn_istream_close( in ) );
        LCN_TEST( apr_file_remove( "test_file", pool ) );
        apr_pool_destroy( pool );
    }

    /* Test for RAMlcn_ostream_t implementation */
    {
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_ram_file_create( &file, pool ) );

        {
            apr_pool_t *os_pool;
            LCN_TEST( apr_pool_create( &os_pool, main_pool ) );
            LCN_TEST( lcn_ram_ostream_create( &out, file, os_pool ) );
            LCN_TEST( lcn_ostream_write_byte( out, (unsigned char) 250 ) );
            LCN_TEST( lcn_ostream_close( out ) );
            apr_pool_destroy( os_pool );
        }

        {
            apr_pool_t *is_pool;
            LCN_TEST( apr_pool_create( &is_pool, main_pool ) );
            LCN_TEST( lcn_ram_input_stream_create( &in, file, is_pool ) );
            LCN_TEST( lcn_istream_read_byte(in, &byte ) );
            CuAssertIntEquals(tc, byte, 250 );
            LCN_TEST( lcn_istream_close( in ) );
            apr_pool_destroy( is_pool );
        }

        apr_pool_destroy( pool );
    }
}

static void
TestCuWriteBytes(CuTest* tc)
{
    char buf[17];
    char s[] = "Das ist ein Test";
    lcn_ostream_t *out;
    lcn_ram_file_t *file;
    unsigned int len;
    lcn_istream_t *in;
    char t[2000];
    int i = 2000;

    /* first test fs output stream */
    /* write out some bytes */
    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_ostream_create( &out, "test_file", pool ) );
        LCN_TEST( lcn_ostream_write_bytes( out, s, 16 ) );
        LCN_TEST( lcn_ostream_close( out ) );
        apr_pool_clear( pool );
    }

    /* read bytes and check them */
    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        buf[16] = 0;
        LCN_TEST( lcn_istream_create( &in, "test_file", pool ) );
        len = 16;
        LCN_TEST( lcn_istream_read_bytes( in, buf, 0, &len ) );
        CuAssertStrEquals(tc, s, buf );
        LCN_TEST( lcn_istream_close( in ) );
        LCN_TEST( apr_file_remove( "test_file", pool ) );
        apr_pool_destroy( pool );
    }

    /* now test ram output stream */
    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_ram_file_create( &file, pool ) );

        /* write some bytes */
        {
            apr_pool_t *os_pool;
            LCN_TEST( apr_pool_create( &os_pool, main_pool ) );
            LCN_TEST( lcn_ram_ostream_create( &out, file, os_pool ) );
            LCN_TEST( lcn_ostream_write_bytes( out,  s, 16 ) );
            LCN_TEST( lcn_ostream_close( out ) );
            apr_pool_clear( os_pool );
        }

        while( --i > 0 ){ t[i] = 5; }

        /* ... and read them */
        {
            apr_pool_t *is_pool;
            LCN_TEST( apr_pool_create( &is_pool, main_pool ) );
            LCN_TEST( lcn_ram_input_stream_create( &in, file, is_pool ) );
            len = 16;
            LCN_TEST( lcn_istream_read_bytes( in, buf, 0, &len ) );
            CuAssertStrEquals(tc, s, buf );
            LCN_TEST( lcn_istream_close( in ) );
            apr_pool_destroy( is_pool );
        }

        apr_pool_destroy( pool );
    }
}

static void
test_write_int_write_impl ( CuTest* tc, apr_status_t (*write) (lcn_ostream_t *, int), lcn_ostream_t *out )
{
    write( out, 1 );
    write( out, 10 );
    write( out, 100 );
    write( out, 1000 );
    write( out, 10000 );
    write( out, 100000 );
    write( out, 1000000 );
    write( out, 10000000 );
    write( out, 100000000 );
}

static void
test_write_vint_write_impl ( CuTest* tc, apr_status_t (*write) (lcn_ostream_t *, unsigned int), lcn_ostream_t *out )
{
    write( out, 1 );
    write( out, 10 );
    write( out, 100 );
    write( out, 1000 );
    write( out, 10000 );
    write( out, 100000 );
    write( out, 1000000 );
    write( out, 10000000 );
    write( out, 100000000 );
}

static void
test_write_vlong_write_impl ( CuTest* tc, apr_status_t (*write) (lcn_ostream_t *, apr_uint64_t), lcn_ostream_t *out )
{
    write( out, 1 );
    write( out, 10 );
    write( out, 100 );
    write( out, 1000 );
    write( out, 10000 );
    write( out, 100000 );
    write( out, 1000000 );
    write( out, 10000000 );
    write( out, 100000000 );
}

#define CHECK_INT( x )                          \
    read( in, &result );                        \
    CuAssertIntEquals(tc, x, result )

#define CHECK_LONG( x )                         \
    read( in, &result );                        \
    CuAssertTrue(tc, x == result )


static void
test_write_int_read_impl ( CuTest* tc, apr_status_t (*read) (lcn_istream_t *, int*), lcn_istream_t *in )
{
    int result;

    CHECK_INT( 1 );
    CHECK_INT( 10);
    CHECK_INT( 100);
    CHECK_INT( 1000);
    CHECK_INT( 10000);
    CHECK_INT( 100000);
    CHECK_INT( 1000000);
    CHECK_INT( 10000000);
    CHECK_INT( 100000000);
}

static void
test_write_vlong_read_impl( CuTest* tc, apr_status_t (*read) (lcn_istream_t *, apr_uint64_t*), lcn_istream_t *in )
{
    apr_uint64_t result;

    CHECK_LONG( 1 );
    CHECK_LONG( 10);
    CHECK_LONG( 100);
    CHECK_LONG( 1000);
    CHECK_LONG( 10000);
    CHECK_LONG( 100000);
    CHECK_LONG( 1000000);
    CHECK_LONG( 10000000);
    CHECK_LONG( 100000000);
}

static void
test_write_vint_read_impl ( CuTest* tc, apr_status_t (*read) (lcn_istream_t *, unsigned int*), lcn_istream_t *in )
{
    unsigned int result;

    CHECK_INT( 1 );
    CHECK_INT( 10);
    CHECK_INT( 100);
    CHECK_INT( 1000);
    CHECK_INT( 10000);
    CHECK_INT( 100000);
    CHECK_INT( 1000000);
    CHECK_INT( 10000000);
    CHECK_INT( 100000000);
}

static void
TestCuWriteInt(CuTest* tc)
{
    lcn_ostream_t *out;
    lcn_ram_file_t *file;
    lcn_istream_t *in;

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_ostream_create( &out, "test_file", pool ) );
        test_write_int_write_impl( tc, lcn_ostream_write_int, out );
        LCN_TEST( lcn_ostream_close( out ) );
        apr_pool_destroy( pool );
    }

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_istream_create( &in, "test_file", pool ) );
        test_write_int_read_impl( tc, lcn_istream_read_int, in );
        LCN_TEST( lcn_istream_close( in ) );
        LCN_TEST( apr_file_remove( "test_file", pool ) );
        apr_pool_destroy( pool );
    }

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_ram_file_create( &file, pool ) );

        {
            apr_pool_t *os_pool;
            LCN_TEST( apr_pool_create( &os_pool, main_pool ) );
            LCN_TEST( lcn_ram_ostream_create( &out, file, os_pool ) );
            test_write_int_write_impl( tc, lcn_ostream_write_int, out );
            LCN_TEST( lcn_ostream_close( out ) );
            apr_pool_destroy( os_pool );
        }

        {
            apr_pool_t *is_pool;
            LCN_TEST( apr_pool_create( &is_pool, main_pool ) );
            LCN_TEST( lcn_ram_input_stream_create( &in, file, is_pool ) );
            test_write_int_read_impl( tc, lcn_istream_read_int, in );
            LCN_TEST( lcn_istream_close( in ) );
            apr_pool_destroy( is_pool );
        }

        apr_pool_destroy( pool );
    }
}

static void
TestCuWriteLong(CuTest* tc)
{
    lcn_ostream_t *out;
    lcn_istream_t *in;

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_ostream_create( &out, "test_file_long", pool ) );
        test_write_vlong_write_impl( tc, lcn_ostream_write_vlong, out );
        LCN_TEST( lcn_ostream_close( out ) );
        apr_pool_destroy( pool );
    }

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_istream_create( &in, "test_file_long", pool ) );
        test_write_vlong_read_impl( tc, lcn_istream_read_vlong, in );
        LCN_TEST( lcn_istream_close( in ) );
        LCN_TEST( apr_file_remove( "test_file_long", pool ) );
        apr_pool_destroy( pool );
    }

#if 0
    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_ram_file_create( &file, pool ) );

        {
            apr_pool_t *os_pool;
            LCN_TEST( apr_pool_create( &os_pool, main_pool ) );
            LCN_TEST( lcn_ram_ostream_create( &out, file, os_pool ) );
            test_write_int_write_impl( lcn_ostream_write_int, out );
            LCN_TEST( lcn_ostream_close( out ) );
            apr_pool_destroy( os_pool );
        }

        {
            apr_pool_t *is_pool;
            LCN_TEST( apr_pool_create( &is_pool, main_pool ) );
            LCN_TEST( lcn_ram_input_stream_create( &in, file, is_pool ) );
            test_write_int_read_impl( lcn_istream_read_int, in );
            LCN_TEST( lcn_istream_close( in ) );
            apr_pool_destroy( is_pool );
        }

        apr_pool_destroy( pool );
    }
#endif
}


static void
TestCuWriteVint(CuTest* tc)
{
    lcn_ostream_t *out;
    lcn_istream_t *in;
    lcn_ram_file_t *file;

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_ostream_create( &out, "test_file", pool ) );
        test_write_vint_write_impl( tc, lcn_ostream_write_vint, out );
        LCN_TEST( lcn_ostream_close( out ) );
        apr_pool_destroy( pool );
    }

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_istream_create( &in, "test_file", pool ) );
        test_write_vint_read_impl( tc, lcn_istream_read_vint, in );
        LCN_TEST( lcn_istream_close( in ) );
        LCN_TEST( apr_file_remove( "test_file", pool ) );
        apr_pool_destroy( pool );
    }

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_ram_file_create( &file, pool ) );

        {
            apr_pool_t *os_pool;
            LCN_TEST( apr_pool_create( &os_pool, main_pool ) );
            LCN_TEST( lcn_ram_ostream_create( &out, file, os_pool ) );
            test_write_vint_write_impl( tc, lcn_ostream_write_vint, out );
            LCN_TEST( lcn_ostream_close( out ) );
            apr_pool_destroy( os_pool );
        }

        {
            apr_pool_t *is_pool;
            LCN_TEST( apr_pool_create( &is_pool, main_pool ) );
            LCN_TEST( lcn_ram_input_stream_create( &in, file, is_pool ) );
            test_write_vint_read_impl( tc, lcn_istream_read_vint, in );
            LCN_TEST( lcn_istream_close( in ) );
            apr_pool_destroy( is_pool );
        }

        apr_pool_destroy( pool );
    }
}

static void
TestCuWriteString(CuTest* tc)
{
    char *buf;
    unsigned int len;

    lcn_ostream_t *out;
    lcn_istream_t *in;
    lcn_ram_file_t *file;

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_fs_ostream_create( &out, "test_file", pool ) );
        LCN_TEST( lcn_ostream_write_string( out, "Teste Sonderzeichen: ouml: ö, auml: ä,"
                                                  "uuml: ü, Ouml: Ö, Auml: Ä, Uuml: Ü, szlig: ß" ) );
        LCN_TEST( lcn_ostream_close( out ) );
        apr_pool_destroy( pool );
    }

    {
        apr_pool_t *pool, *str_pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( apr_pool_create( &str_pool, main_pool ) );

        LCN_TEST( lcn_istream_create( &in, "test_file", pool ) );
        LCN_TEST( lcn_istream_read_string( in, &buf, &len, str_pool ) );
        LCN_TEST( apr_file_remove( "test_file", pool ) );
        LCN_TEST( lcn_istream_close( in ) );
        apr_pool_destroy( pool );

        CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: ö, auml: ä,"
                          "uuml: ü, Ouml: Ö, Auml: Ä, Uuml: Ü, szlig: ß" );
        apr_pool_destroy( str_pool );
    }

    {
        apr_pool_t *pool;
        LCN_TEST( apr_pool_create( &pool, main_pool ) );
        LCN_TEST( lcn_ram_file_create( &file, pool ) );

        {
            apr_pool_t *os_pool;
            LCN_TEST( apr_pool_create( &os_pool, main_pool ) );
            LCN_TEST( lcn_ram_ostream_create( &out, file, os_pool ) );
            LCN_TEST( lcn_ostream_write_string( out, "Teste Sonderzeichen: ouml: ö, "
                                                      "auml: ä, uuml: ü, Ouml: Ö, Auml: Ä, Uuml: Ü, szlig: ß" ) );
            LCN_TEST( lcn_ostream_close( out ) );
            apr_pool_destroy( os_pool );
        }

        {
            apr_pool_t *is_pool, *str_pool;
            LCN_TEST( apr_pool_create( &is_pool, main_pool ) );
            LCN_TEST( apr_pool_create( &str_pool, main_pool ) );

            LCN_TEST( lcn_ram_input_stream_create( &in, file, is_pool ) );
            LCN_TEST( lcn_istream_read_string( in, &buf, &len, str_pool ) );
            LCN_TEST( lcn_istream_close( in ) );
            apr_pool_destroy( is_pool );

            CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: ö, auml: ä, uuml: ü,"
                              " Ouml: Ö, Auml: Ä, Uuml: Ü, szlig: ß");
            apr_pool_destroy( str_pool );
        }

        apr_pool_destroy( pool );
    }
}

static void
TestCuSeek(CuTest* tc)
{
    char bytes[20];
    lcn_istream_t *in;
    lcn_directory_t *t_dir;
    unsigned int len = 10;
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, pool ) );

    LCN_TEST( lcn_directory_open_input( t_dir, &in, "string.dat", pool ) );
    LCN_TEST( lcn_istream_seek(in, 10) );
    LCN_TEST( lcn_istream_read_bytes( in, bytes, 0, &len ) );
    bytes[10] = 0;
    CuAssertStrEquals(tc, bytes, "ftware Fou" );
    LCN_TEST( lcn_istream_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );
}

static void
test_istream_clone(CuTest* tc)
{
    lcn_istream_t *clone;
    char *buf;
    unsigned int len;

    lcn_istream_t *in;
    lcn_directory_t *t_dir;
    apr_pool_t *pool;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &t_dir, TEST_DIR, LCN_FALSE, pool ) );
    LCN_TEST( lcn_directory_open_input( t_dir, &in, "string.dat", pool ) );
    CuAssertTrue(tc, in != NULL);

    LCN_TEST( lcn_istream_read_string( in, &buf, &len, pool ) );
    CuAssertIntEquals(tc, len, 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation" );

    LCN_TEST( lcn_istream_clone( in, &clone, pool ) );

    LCN_TEST( lcn_istream_read_string( in, &buf, &len, pool ) );
    CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: "
                               "ö, auml: ä, uuml: ü, Ouml: "
                               "Ö, Auml: Ä, Uuml: Ü, szlig: ß");

    LCN_TEST( lcn_istream_read_string( clone, &buf, &len, pool ) );
    CuAssertIntEquals(tc, len, 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation" );

    LCN_TEST( lcn_istream_seek( in, 0 ));
    LCN_TEST( lcn_istream_read_string( in, &buf, &len, pool ) );
    CuAssertIntEquals(tc, len, 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation" );

    LCN_TEST( lcn_istream_seek( clone, 0 ));

    LCN_TEST( lcn_istream_read_string( in, &buf, &len, pool ) );
    CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: "
                               "ö, auml: ä, uuml: ü, Ouml: "
                               "Ö, Auml: Ä, Uuml: Ü, szlig: ß");

    LCN_TEST( lcn_istream_seek( in, 0 ));
    LCN_TEST( lcn_istream_read_string( clone, &buf, &len, pool ) );
    CuAssertStrEquals(tc, buf, "Apache Software Foundation" );

    LCN_TEST( lcn_istream_read_string( clone, &buf, &len, pool ) );
    CuAssertStrEquals(tc, buf, "Teste Sonderzeichen: ouml: "
                               "ö, auml: ä, uuml: ü, Ouml: "
                               "Ö, Auml: Ä, Uuml: Ü, szlig: ß");

    LCN_TEST( lcn_istream_read_string( in, &buf, &len, pool ) );
    CuAssertIntEquals(tc, len, 26);
    CuAssertStrEquals(tc, buf, "Apache Software Foundation" );

    LCN_TEST( lcn_istream_close( clone ) );
    LCN_TEST( lcn_istream_close( in ) );
    LCN_TEST( lcn_directory_close( t_dir ) );

    apr_pool_destroy( pool );
}


CuSuite *
make_ostream_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, TestCuCreateOutputStream);
    SUITE_ADD_TEST(s, TestCuWriteByte);
    SUITE_ADD_TEST(s, TestCuWriteBytes);
    SUITE_ADD_TEST(s, TestCuWriteInt);
    SUITE_ADD_TEST(s, TestCuWriteVint);
    SUITE_ADD_TEST(s, TestCuWriteString);
    SUITE_ADD_TEST(s, TestCuSeek);
    SUITE_ADD_TEST(s, test_istream_clone);
    SUITE_ADD_TEST(s, TestCuWriteLong);

    return s;
}

