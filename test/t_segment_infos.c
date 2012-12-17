#include "test_all.h"
#include "segment_infos.h"
#include "lcn_search.h"
#include "lcn_analysis.h"

static void
test_segment_infos(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_ram_directory_create( &dir, pool ) );

    {
        apr_pool_t *si_pool;
        lcn_segment_infos_t *infos;
        char *name;

        LCN_TEST( apr_pool_create( &si_pool, pool ) );
        LCN_TEST( lcn_segment_infos_create( &infos, si_pool ) );
        CuAssertIntEquals(tc, 0, lcn_segment_infos_size( infos ) );

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
        lcn_segment_info_per_commit_t *info_pc;
        char *name;

        LCN_TEST( apr_pool_create( &si_pool, pool ) );
        LCN_TEST( lcn_segment_infos_create( &infos, si_pool ) );
        LCN_TEST( lcn_segment_infos_read_directory( infos, dir ) );
        CuAssertIntEquals(tc, 0, lcn_segment_infos_size( infos ) );
        CuAssertIntEquals(tc, 1, lcn_segment_infos_version( infos ) );

        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_2", name );

        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_3", name );

        LCN_TEST( lcn_segment_infos_add_info( infos, dir, name, 23 ));
        CuAssertIntEquals(tc, 1, lcn_segment_infos_size( infos ) );

        LCN_TEST( lcn_segment_infos_get( infos, &info_pc, 0 ) );
        info = lcn_segment_info_per_commit_info( info_pc );
        CuAssertStrEquals(tc, "_3", lcn_segment_info_name( info ) );

        LCN_TEST( lcn_segment_infos_write( infos, dir ) );
        CuAssertIntEquals(tc, 2, lcn_segment_infos_version( infos ) );

        apr_pool_destroy( si_pool );
    }

    {
        apr_pool_t *si_pool;
        lcn_segment_infos_t *infos;
        lcn_segment_info_t *info;
        lcn_segment_info_per_commit_t *info_pc;
        char *name;

        LCN_TEST( apr_pool_create( &si_pool, pool ) );
        LCN_TEST( lcn_segment_infos_create( &infos, si_pool ) );
        LCN_TEST( lcn_segment_infos_read_directory( infos, dir ) );
        CuAssertIntEquals(tc, 1, lcn_segment_infos_size( infos ) );
        CuAssertIntEquals(tc, 2, lcn_segment_infos_version( infos ) );

        LCN_TEST( lcn_segment_infos_get( infos, &info_pc, 0 ) );
        info = lcn_segment_info_per_commit_info( info_pc );
        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_4", name );
        LCN_TEST( lcn_segment_infos_get_next_name( infos, &name, si_pool ) );
        CuAssertStrEquals(tc, "_5", name );
        LCN_TEST( lcn_segment_infos_add_info( infos, dir, name, 12 ));
        CuAssertIntEquals(tc, 2, lcn_segment_infos_size( infos ) );
        LCN_TEST( lcn_segment_infos_get( infos, &info_pc, 1 ) );
        info = lcn_segment_info_per_commit_info( info_pc );
        CuAssertStrEquals(tc, "_5", lcn_segment_info_name( info ) );
        LCN_TEST( lcn_segment_infos_write( infos, dir ) );
        CuAssertIntEquals(tc, 3, lcn_segment_infos_version( infos ) );

        apr_pool_destroy( si_pool );
    }
}

static void
test_combining_field_infos( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_document_t *document;
    lcn_directory_t *dir1, *dir2;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    /* create test indexes */
    {
        lcn_analyzer_t *sa;
        lcn_field_t *field;
        lcn_index_writer_t *index_writer;

        LCN_TEST( lcn_ram_directory_create( &dir1, pool ) );
        LCN_TEST( lcn_index_writer_create_by_directory( &index_writer,
                                                        dir1,
                                                        LCN_TRUE,
                                                        pool ));

        LCN_TEST( lcn_document_create( &document, pool ) );

        LCN_TEST( lcn_field_create( &field,
                                    "text",
                                    "sf foo bar sf",
                                    LCN_FIELD_INDEXED |
                                    LCN_FIELD_TOKENIZED |
                                    LCN_FIELD_STORED,
                                    LCN_FIELD_VALUE_COPY, pool ) );

        LCN_TEST( lcn_simple_analyzer_create( &sa, pool ) );
        lcn_field_set_analyzer( field, sa );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

        LCN_TEST( lcn_index_writer_close( index_writer) );
        LCN_TEST( lcn_index_writer_optimize( index_writer ));
    }

    {
        lcn_analyzer_t *sa;
        lcn_field_t *field;
        lcn_index_writer_t *index_writer;

        LCN_TEST( lcn_ram_directory_create( &dir2, pool ) );
        LCN_TEST( lcn_index_writer_create_by_directory( &index_writer,
                                                        dir2,
                                                        LCN_TRUE,
                                                        pool ));

        LCN_TEST( lcn_document_create( &document, pool ) );

        LCN_TEST( lcn_field_create( &field,
                                    "text",
                                    "sf foo bar sf",
                                    LCN_FIELD_STORED,
                                    LCN_FIELD_VALUE_COPY, pool ) );

        LCN_TEST( lcn_simple_analyzer_create( &sa, pool ) );
        lcn_field_set_analyzer( field, sa );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

        LCN_TEST( lcn_index_writer_close( index_writer) );
        LCN_TEST( lcn_index_writer_optimize( index_writer ));
    }

    {
        lcn_index_reader_t *reader1, *reader2, *mreader;
        lcn_list_t *list, *field_infos;
        lcn_field_info_t *fi;

        LCN_TEST( lcn_index_reader_create_by_directory( &reader1, dir1, LCN_TRUE, pool ));
        LCN_TEST( lcn_index_reader_create_by_directory( &reader2, dir1, LCN_TRUE, pool ));

        LCN_TEST( lcn_list_create( &list, 10, pool ) );

        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &mreader, list, pool ));

        LCN_TEST( lcn_list_create( &field_infos, 10, pool ) );

        LCN_TEST( lcn_index_reader_get_field_infos( mreader,
                                                    field_infos,
                                                    0,
                                                    0 ));

        CuAssertIntEquals( tc, 1, lcn_list_size( field_infos ));

        fi = (lcn_field_info_t*) lcn_list_get( field_infos, 0 );

        CuAssertStrEquals( tc, "text", lcn_field_info_name( fi ));
        CuAssertTrue( tc, lcn_field_info_is_indexed( fi ));
    }

    apr_pool_destroy( pool );
}

CuSuite *
make_segment_infos_suite(void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, test_segment_infos);
    SUITE_ADD_TEST( s, test_combining_field_infos );

    return s;
}
