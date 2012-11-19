#include "test_all.h"
#include "field_infos.h"

static void
TestCuFieldInfos(CuTest* tc)
{
    apr_pool_t *p;
    lcn_field_infos_t *field_infos;
    lcn_field_info_t *field_info;
    unsigned int number;
    char *name;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_field_infos_create( &field_infos, p ) );

    LCN_ERR( lcn_field_infos_field_number( field_infos, &number, "field" ),
             LCN_ERR_FIELD_NOT_FOUND );

    LCN_ERR( lcn_field_infos_by_name( field_infos, &field_info, "field" ),
             LCN_ERR_FIELD_NOT_FOUND );

    LCN_ERR( lcn_field_infos_name_by_number( field_infos, &name, 2 ),
             LCN_ERR_FIELD_NOT_FOUND );

    LCN_ERR( lcn_field_infos_by_number( field_infos, &field_info, 0 ),
             LCN_ERR_FIELD_NOT_FOUND );

    LCN_ERR( lcn_field_infos_by_number( field_infos, &field_info, 1 ),
             LCN_ERR_FIELD_NOT_FOUND );


    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "text", LCN_FIELD_INFO_IS_INDEXED ) );
    LCN_TEST( lcn_field_infos_by_number( field_infos, &field_info, 0 ) );
    CuAssertTrue(tc, lcn_field_info_is_indexed( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_omit_norms( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_offset_with_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_position_with_term_vector( field_info ) );

    CuAssertStrEquals(tc, "text", field_info->name );
    LCN_TEST( lcn_field_infos_name_by_number( field_infos, &name, 0 ) );
    CuAssertStrEquals(tc, "text", name );
    LCN_TEST( lcn_field_infos_field_number( field_infos, &number, "text" ) );
    CuAssertIntEquals( tc, number, 0 );
    LCN_ERR( lcn_field_infos_field_number( field_infos, &number, "none" ), LCN_ERR_FIELD_NOT_FOUND );
    CuAssertIntEquals(tc, 1, lcn_field_infos_size( field_infos ) );



    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "title",
                                              LCN_FIELD_INFO_OMIT_NORMS |
                                              LCN_FIELD_INFO_IS_INDEXED ));

    LCN_TEST( lcn_field_infos_by_number( field_infos, &field_info, 1 ) );
    CuAssertTrue(tc, lcn_field_info_is_indexed( field_info ) );
    CuAssertTrue(tc, lcn_field_info_omit_norms( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_offset_with_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_position_with_term_vector( field_info ) );

    CuAssertStrEquals(tc, "title", field_info->name );
    LCN_TEST( lcn_field_infos_name_by_number( field_infos, &name, 0 ) );
    CuAssertStrEquals(tc, "text", name );
    LCN_TEST( lcn_field_infos_name_by_number( field_infos, &name, 1 ) );
    CuAssertStrEquals(tc, "title", name );
    LCN_TEST( lcn_field_infos_field_number( field_infos, &number, "text" ) );
    CuAssertIntEquals( tc, number, 0 );
    LCN_TEST( lcn_field_infos_field_number( field_infos, &number, "title" ) );
    CuAssertIntEquals( tc, number, 1 );
    CuAssertIntEquals(tc, 2, lcn_field_infos_size( field_infos ) );

    LCN_ERR( lcn_field_infos_add_field_info( field_infos, "title_1",
                                             LCN_FIELD_INFO_OMIT_NORMS ),
             LCN_ERR_FIELD_INFO_OMIT_NORMS_ON_UNINDEXED );


    LCN_ERR( lcn_field_infos_add_field_info( field_infos, "author", 
                                             LCN_FIELD_INFO_STORE_TERM_VECTOR ),
             LCN_ERR_FIELD_INFO_STORE_TERM_VECTOR_ON_UNINDEXED );

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "author",
                                              LCN_FIELD_INFO_IS_INDEXED |
                                              LCN_FIELD_INFO_STORE_TERM_VECTOR ));
    LCN_TEST( lcn_field_infos_by_number( field_infos, &field_info, 2 ) );
    CuAssertTrue(tc, lcn_field_info_is_indexed( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_omit_norms( field_info ) );
    CuAssertTrue(tc, lcn_field_info_store_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_offset_with_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_position_with_term_vector( field_info ) );

    CuAssertStrEquals(tc, "author", field_info->name );
    LCN_TEST( lcn_field_infos_name_by_number( field_infos, &name, 2 ) );
    CuAssertStrEquals(tc, "author", name );
    LCN_TEST( lcn_field_infos_name_by_number( field_infos, &name, 0 ) );
    CuAssertStrEquals(tc, "text", name );
    LCN_TEST( lcn_field_infos_name_by_number( field_infos, &name, 1 ) );
    CuAssertStrEquals(tc, "title", name );
    LCN_TEST( lcn_field_infos_field_number( field_infos, &number, "text" ) );
    CuAssertIntEquals( tc, number, 0 );
    LCN_TEST( lcn_field_infos_field_number( field_infos, &number, "title" ) );
    CuAssertIntEquals( tc, number, 1 );
    LCN_TEST( lcn_field_infos_field_number( field_infos, &number, "author" ) );
    CuAssertIntEquals( tc, number, 2 );
    CuAssertIntEquals(tc, 3, lcn_field_infos_size( field_infos ) );

    LCN_TEST( lcn_field_infos_by_name( field_infos, &field_info, "text" ) );
    CuAssertTrue(tc, lcn_field_info_is_indexed( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_omit_norms( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_term_vector( field_info ) );

    LCN_ERR( lcn_field_infos_add_field_info( field_infos, "text",
                                             LCN_FIELD_INFO_OMIT_NORMS |
                                             LCN_FIELD_INFO_IS_INDEXED ),
             LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION );

    CuAssertTrue(tc, lcn_field_info_is_indexed( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_omit_norms( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_offset_with_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_position_with_term_vector( field_info ) );

    LCN_ERR( lcn_field_infos_add_field_info( field_infos, "date",
                                             LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR ),
             LCN_ERR_FIELD_INFO_STORE_TERM_VECTOR_ON_UNINDEXED );

    LCN_ERR( lcn_field_infos_add_field_info( field_infos, "date",
                                             LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR ),
             LCN_ERR_FIELD_INFO_STORE_TERM_VECTOR_ON_UNINDEXED );

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "date",
                                              LCN_FIELD_INFO_IS_INDEXED |
                                              LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR ));

    LCN_TEST( lcn_field_infos_by_number( field_infos, &field_info, 3 ) );
    CuAssertTrue(tc, lcn_field_info_is_indexed( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_omit_norms( field_info ) );
    CuAssertTrue(tc, lcn_field_info_store_term_vector( field_info ) );
    CuAssertTrue(tc, lcn_field_info_store_offset_with_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_position_with_term_vector( field_info ) );

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "key",
                                              LCN_FIELD_INFO_IS_INDEXED |
                                              LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR ));

    LCN_TEST( lcn_field_infos_by_number( field_infos, &field_info, 4 ) );
    CuAssertTrue(tc, lcn_field_info_is_indexed( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_omit_norms( field_info ) );
    CuAssertTrue(tc, lcn_field_info_store_term_vector( field_info ) );
    CuAssertTrue(tc, ! lcn_field_info_store_offset_with_term_vector( field_info ) );
    CuAssertTrue(tc, lcn_field_info_store_position_with_term_vector( field_info ) );

    apr_pool_destroy( p );
}

static void
TestCuFieldInfosReadWrite(CuTest* tc)
{
    apr_pool_t *p;
    lcn_field_infos_t *field_infos;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_field_infos_create( &field_infos, p ) );

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "text", LCN_FIELD_INFO_IS_INDEXED ));

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "date",
                                              LCN_FIELD_INFO_IS_INDEXED |
                                              LCN_FIELD_INFO_OMIT_NORMS ));

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "type",
                                              LCN_FIELD_INFO_IS_INDEXED |
                                              LCN_FIELD_INFO_STORE_TERM_VECTOR ));

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "test",
                                              LCN_FIELD_INFO_IS_INDEXED |
                                              LCN_FIELD_INFO_STORE_OFFSET_WITH_TERM_VECTOR ));

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "field",
                                              LCN_FIELD_INFO_IS_INDEXED ));

    LCN_TEST( lcn_field_infos_add_field_info( field_infos, "value",
                                              LCN_FIELD_INFO_IS_INDEXED |
                                              LCN_FIELD_INFO_STORE_POSITION_WITH_TERM_VECTOR ));
    {
        apr_pool_t *dir_pool;
        LCN_TEST( apr_pool_create( &dir_pool, main_pool ) );
        LCN_TEST( lcn_ram_directory_create( &dir, dir_pool ) );
        LCN_TEST( lcn_field_infos_write( field_infos, dir, "tseg.fnm" ) );

        {
            apr_pool_t *pool;
            lcn_field_infos_t *new_field_infos;
            lcn_field_info_t *fi;

            LCN_TEST( apr_pool_create( &pool, main_pool ) );

            {
                lcn_index_input_t *in;
                apr_pool_t *is_pool;
                LCN_TEST( apr_pool_create( &is_pool, main_pool ) );

                LCN_TEST( lcn_directory_open_segment_file( dir, &in, "tseg", ".fnm", p ) );
                LCN_TEST( lcn_field_infos_create( &new_field_infos, is_pool ));
                new_field_infos->format = -1;
                LCN_TEST( lcn_field_infos_read( new_field_infos, in, is_pool ) );
                LCN_TEST( lcn_index_input_close( in ) );
                apr_pool_destroy( is_pool );
            }

            LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 0 ) );
            CuAssertStrEquals(tc, "text", fi->name );
            CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );


            LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 1 ) );
            CuAssertStrEquals(tc, "date", fi->name );
            CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
            CuAssertTrue( tc, lcn_field_info_omit_norms( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );


            LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 2 ) );
            CuAssertStrEquals(tc, "type", fi->name );
            CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
            CuAssertTrue( tc, lcn_field_info_store_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );

            LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 3 ) );
            CuAssertStrEquals(tc, "test", fi->name );
            CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
            CuAssertTrue( tc, lcn_field_info_store_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
            CuAssertTrue( tc, lcn_field_info_store_offset_with_term_vector( fi ) );

            LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 4 ) );
            CuAssertStrEquals(tc, "field", fi->name );
            CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );


            LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 5 ) );
            CuAssertStrEquals(tc, "value", fi->name );
            CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
            CuAssertTrue( tc, lcn_field_info_store_term_vector( fi ) );
            CuAssertTrue( tc, lcn_field_info_store_position_with_term_vector( fi ) );
            CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );

            apr_pool_destroy( pool );
        }
    }
}

static void
TestCuFieldInfosRead(CuTest* tc)
{
    apr_pool_t *p;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &p, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &dir, "test_index_1", LCN_FALSE, p ) );

    {
        apr_pool_t *fi_pool;
        lcn_field_infos_t *field_infos;
        lcn_field_info_t *fi;

        LCN_TEST( apr_pool_create( &fi_pool, p ) );
        LCN_TEST( lcn_field_infos_create_from_dir( &field_infos, dir, "_2c", fi_pool ))

        LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 0 ) );
        CuAssertStrEquals(tc, "text", fi->name );
        CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_term_vector( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );

        LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 1 ) );
        CuAssertStrEquals(tc, "id", fi->name );
        CuAssertTrue( tc, lcn_field_info_is_indexed( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_term_vector( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );

        LCN_TEST( lcn_field_infos_by_number( field_infos, &fi, 2 ) );
        CuAssertStrEquals(tc, "content", fi->name );
        CuAssertTrue( tc, ! lcn_field_info_is_indexed( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_omit_norms( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_term_vector( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_position_with_term_vector( fi ) );
        CuAssertTrue( tc, ! lcn_field_info_store_offset_with_term_vector( fi ) );
    }

    apr_pool_destroy( p );
}

CuSuite *make_field_infos_suite (void)
{
    CuSuite *s = CuSuiteNew();

    SUITE_ADD_TEST(s,TestCuFieldInfos);
    SUITE_ADD_TEST(s,TestCuFieldInfosReadWrite);
    SUITE_ADD_TEST(s,TestCuFieldInfosRead);

    return s;
}
