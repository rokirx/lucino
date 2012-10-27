#include "directory.h"
#include "test_all.h"
#include "term_infos_reader.h"
#include "term_infos_writer.h"

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




#if 0
    s_infos->read( s_infos, t_dir );
    f_infos = t_dir->read_field_infos( t_dir, s_infos->first_info->name );

    CuAssertTrue(tc, strcmp( "", f_infos->first_info->name ) == 0);
    CuAssertTrue(tc, 0 == f_infos->first_info->number);
    CuAssertTrue(tc, strcmp( "xid", f_infos->first_info->next->name ) == 0);
    CuAssertTrue(tc, 1 == f_infos->first_info->next->number); 
    CuAssertTrue(tc, strcmp( "volltext", f_infos->first_info->next->next->name ) == 0);
    CuAssertTrue(tc, 2 == f_infos->first_info->next->next->number);
    CuAssertTrue(tc, 0 == f_infos->field_number( f_infos, "" ));
    CuAssertTrue(tc, 1 == f_infos->field_number( f_infos, "xid" ));
    CuAssertTrue(tc, 2 == f_infos->field_number( f_infos, "volltext" ));
    writer = create_TermInfosWriter( wr_dir, "_k", f_infos );

##########
    
    writer->add_term( writer, t, ti );

    t->text[2] = 'd';
    ti->doc_freq = 3;
    ti->freq_pointer = 17;
    ti->prox_pointer = 27;

    writer->add_term( writer, t, ti );

    CuAssertTrue(tc, writer->size == 2);

    writer->close( writer );
    writer->free( writer );
    free( ti );
    free_Term( t );

    ti_reader = create_TermInfosReader( wr_dir, s_infos->first_info->name, f_infos );

    CuAssertTrue(tc, ti_reader->size == 2);

    t_enum = ti_reader->terms( ti_reader );
    CuAssertTrue(tc, t_enum->size == 2);
    CuAssertTrue(tc, 1 == t_enum->next( t_enum ));
    CuAssertTrue(tc, strcmp( "xid", t_enum->term->field ) == 0);
    CuAssertTrue(tc, strcmp( "abc", t_enum->term->text   ) == 0);
    CuAssertTrue(tc, 2 == t_enum->term_info->doc_freq);
    CuAssertTrue(tc, 5 == t_enum->term_info->freq_pointer);
    CuAssertTrue(tc, 7 == t_enum->term_info->prox_pointer);

    CuAssertTrue(tc, 1 == t_enum->next( t_enum ));
    CuAssertTrue(tc, strcmp( "xid", t_enum->term->field ) == 0);
    CuAssertTrue(tc, strcmp( "abd", t_enum->term->text   ) == 0);
    CuAssertTrue(tc, 3 == t_enum->term_info->doc_freq);
    CuAssertTrue(tc, 17 == t_enum->term_info->freq_pointer);
    CuAssertTrue(tc, 27 == t_enum->term_info->prox_pointer);

    t_enum->free( t_enum );
    ti_reader->free( ti_reader );
    f_infos->free( f_infos );
    s_infos->free( s_infos );

    apr_pool_create( &pool, NULL );

    apr_file_remove( "temp_index/_k.tii", pool );
    apr_file_remove( "temp_index/_k.tis", pool );

    apr_pool_destroy( pool );

    t_dir->close( t_dir );
    wr_dir->close( wr_dir );
#endif
}

CuSuite *
make_term_infos_writer_suite (void)
{	
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_term_infos_writer );

    return s;
}
