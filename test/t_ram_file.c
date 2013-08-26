#include "test_all.h"
#include "index_input.h"

static void
test_read_write(CuTest* tc)
{
    apr_pool_t *p;
    lcn_index_output_t *os;
    lcn_index_input_t *is;
    lcn_ram_file_t *file;
    unsigned char i;
    unsigned char b;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_ram_file_create( &file, p ) );
    LCN_TEST( lcn_ram_index_output_create( &os, file, p ) );

    for( i = 0; i < 100; i++ )
    {
        LCN_TEST( lcn_index_output_write_byte( os, i ));
    }

    LCN_TEST( lcn_index_output_close( os ) );

    LCN_TEST( lcn_ram_input_stream_create( &is, NULL, file, p ) );

    for( i = 0; i < 100; i++ )
    {
        LCN_TEST( lcn_index_input_read_byte( is, &b ) );
        CuAssertIntEquals(tc, (int) i, (int) b );
    }

    CuAssertIntEquals( tc, LCN_ERR_READ_PAST_EOF, lcn_index_input_read_byte( is, &b ) );

    apr_pool_destroy( p );
}

CuSuite *
make_ram_file_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_read_write );

    return s;
}

