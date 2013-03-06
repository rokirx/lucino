#include "directory.h"
#include "test_all.h"
#include "term_infos_reader.h"
#include "term_infos_writer.h"
#include "lcn_index.h"

#define LCN_WRITE_TI( FIELD, TEXT, FREQ, FPTR, PPTR, FNUM )                             \
    LCN_TEST( lcn_term_create( &term, FIELD, TEXT, 0, p ) );                            \
    term_info.doc_freq = FREQ;                                                          \
    term_info.freq_pointer = FPTR;                                                      \
    term_info.prox_pointer = PPTR;                                                      \
    LCN_TEST( lcn_term_infos_writer_add_term( ti_writer, term, &term_info, FNUM ) );

#define LCN_CHECK_TI( FIELD, TEXT, FREQ, FPTR, PPTR )                                                           \
    next_status = lcn_term_enum_next( term_enum );                                                              \
    CuAssertTrue(tc, APR_SUCCESS == next_status );                                                              \
    CuAssertStrEquals(tc, FIELD, lcn_term_field( lcn_term_enum_term( term_enum )));                             \
    CuAssertStrEquals(tc, TEXT, lcn_term_text( lcn_term_enum_term( term_enum )));                               \
    LCN_TEST( lcn_term_infos_reader_get_by_term( ti_reader, &term_info, lcn_term_enum_term( term_enum)));       \
    CuAssertIntEquals(tc, FREQ, term_info->doc_freq );                                                          \
    CuAssertIntEquals(tc, FPTR, term_info->freq_pointer );                                                      \
    CuAssertIntEquals(tc, PPTR, term_info->prox_pointer );




static void
test_term_infos_writer(CuTest* tc)
{
    lcn_field_infos_t *f_infos;
    lcn_term_t *term;
    apr_pool_t *p;
    lcn_directory_t *dir;

    {
        lcn_term_infos_writer_t *ti_writer;
        lcn_term_info_t term_info;
        LCN_TEST( apr_pool_create( &p, main_pool ) );

        LCN_TEST( lcn_fs_directory_create( &dir, WRITE_DIR, 1, p ) );
        LCN_TEST( lcn_term_infos_writer_create( &ti_writer, dir, "test_segment", 20, p ));

        CuAssertTrue(tc, ti_writer->is_index == 0);

        LCN_WRITE_TI( "xid", "abc", 2, 5, 7, 0 );
        LCN_TEST( lcn_term_create( &term, "xid", "aac", 0, p ) );
        LCN_ERR( lcn_term_infos_writer_add_term( ti_writer, term, &term_info, 5 ), LCN_ERR_TERM_OUT_OF_ORDER );
        LCN_TEST( lcn_term_create( &term, "xid", "adc", 0, p ) );

        term_info.freq_pointer = 3;

        LCN_ERR( lcn_term_infos_writer_add_term( ti_writer, term, &term_info, 5 ), LCN_ERR_FREQ_POINTER_OUT_OF_ORDER );

        term_info.freq_pointer = 5;
        term_info.prox_pointer = 3;

        LCN_ERR( lcn_term_infos_writer_add_term( ti_writer, term, &term_info, 5 ), LCN_ERR_PROX_POINTER_OUT_OF_ORDER );

        LCN_WRITE_TI( "xid", "bfg", 3, 6, 9, 0 );
        LCN_WRITE_TI( "xid", "bzag", 3, 8, 11, 0 );
        LCN_WRITE_TI( "xid", "copf", 1, 11, 19, 0 );
        LCN_WRITE_TI( "xzf", "abf", 1, 15, 23, 1 );

        LCN_TEST( lcn_term_infos_writer_close( ti_writer ) );
        LCN_TEST( lcn_directory_close( dir ) );

        apr_pool_destroy( p );
    }

    {
        lcn_term_infos_reader_t *ti_reader;
        lcn_term_info_t *term_info;


        LCN_TEST( apr_pool_create( &p, main_pool ) );

        LCN_TEST( lcn_fs_directory_create( &dir, WRITE_DIR, LCN_FALSE, p ) );

        LCN_TEST( lcn_field_infos_create( &f_infos, p ) );
        LCN_TEST( lcn_field_infos_add_field_info( f_infos, "xid", LCN_FIELD_INFO_IS_INDEXED ));
        LCN_TEST( lcn_field_infos_add_field_info( f_infos, "xzf", LCN_FIELD_INFO_IS_INDEXED ));

        LCN_TEST( lcn_term_infos_reader_create( &ti_reader, dir, "test_segment", f_infos, p ) );

        {
            apr_pool_t *te_pool;
            lcn_term_enum_t *term_enum;
            apr_status_t next_status;

            LCN_TEST( apr_pool_create( &te_pool, p ) );
            LCN_TEST( lcn_term_infos_reader_terms( ti_reader, &term_enum, te_pool ) );

            LCN_CHECK_TI( "xid", "abc", 2, 5, 7 );
            LCN_CHECK_TI( "xid", "bfg", 3, 6, 9 );
            LCN_CHECK_TI( "xid", "bzag", 3, 8, 11 );
            LCN_CHECK_TI( "xid", "copf", 1, 11, 19 );
            LCN_CHECK_TI( "xzf", "abf", 1, 15, 23 );

            next_status = lcn_term_enum_next( term_enum );
            CuAssertTrue(tc, LCN_ERR_ITERATOR_NO_NEXT == next_status );

            LCN_TEST( lcn_term_enum_close( term_enum ) );
            apr_pool_destroy( te_pool );
        }

        apr_file_remove( "temp_index/test_segment.tii", p );
        apr_file_remove( "temp_index/test_segment.tis", p );

        apr_pool_destroy( p );
    }
}

static void
test_long_terms(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;
    lcn_index_writer_t *index_writer;
    char *buf;
    int i = 0;
    lcn_field_type_t indexed_type = {0};

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_field_type_set_indexed( &indexed_type, LCN_TRUE ));
    LCN_TEST( lcn_ram_directory_create( &dir, pool ) );
    LCN_TEST( lcn_index_writer_create_by_directory( &index_writer, dir, LCN_TRUE, pool ));

    buf = apr_pcalloc(pool, 2000);

    for( i=0; i < 1100; i++)
    {
        buf[i] = 'a';
    }

    /* prior to 5969a24d6 there is segmentation fault after 999 iterations */

    for( i=0; i< 3000; i++)
    {
        lcn_document_t *document;
        lcn_field_t *field;

        LCN_TEST( lcn_document_create( &document, pool ) );
        apr_snprintf( buf+1100, 10, "%d", i );
        LCN_TEST( lcn_field_create( &field, "text", buf, &indexed_type, pool ));
        LCN_TEST( lcn_document_add_field( document, field ));
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ));
    }

    LCN_TEST( lcn_index_writer_close( index_writer ));
}

CuSuite *
make_term_infos_writer_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_term_infos_writer );
    SUITE_ADD_TEST(s, test_long_terms );

    return s;
}
