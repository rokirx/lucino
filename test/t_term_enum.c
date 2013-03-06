#include "test_all.h"
#include "lcn_term_enum.h"
#include "field_infos.h"
#include "term_enum.h"
#include "lcn_search.h"
#include "io_context.h"

#define CALL_NEXT( TERM_ENUM )                         \
   next_status = lcn_term_enum_next( TERM_ENUM );      \
   CuAssertTrue(tc, APR_SUCCESS == next_status )


#define CHECK_NEXT_TERM( TERM_ENUM, FIELD, TEXT )                                       \
   CuAssertStrEquals(tc, FIELD, lcn_term_field( lcn_term_enum_term( TERM_ENUM ) ) );    \
   CuAssertStrEquals(tc, TEXT,  lcn_term_text( lcn_term_enum_term( TERM_ENUM ) ) )



static void
test_segment_term_enum(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_fs_directory_create( &dir, "test_index_1", LCN_FALSE, pool ) );

    {
        apr_pool_t *is_pool;
        apr_pool_t *te_pool;
        apr_pool_t *fi_pool;

        lcn_term_enum_t *term_enum;
        lcn_index_input_t *istream;
        lcn_field_infos_t *field_infos;
        apr_status_t next_status;

        LCN_TEST( apr_pool_create( &is_pool, pool ) );
        LCN_TEST( apr_pool_create( &te_pool, pool ) );
        LCN_TEST( apr_pool_create( &fi_pool, pool ) );
        LCN_TEST( lcn_field_infos_create_from_dir( &field_infos, dir, "_2c", fi_pool ));

        LCN_TEST( lcn_directory_open_input( dir, &istream, "_2c.tii", LCN_IO_CONTEXT_READONCE, is_pool ) );
        LCN_TEST( lcn_segment_term_enum_create( &term_enum, istream, field_infos, LCN_TRUE, te_pool ));

        CALL_NEXT( term_enum );
        CHECK_NEXT_TERM( term_enum, "", "" );

        {
            apr_pool_t *clone_pool;
            lcn_term_enum_t *clone;

            LCN_TEST( apr_pool_create( &clone_pool, pool ) );
            LCN_TEST( lcn_term_enum_clone( term_enum, &clone, pool ) );

            CHECK_NEXT_TERM( clone, "", "" );

            CALL_NEXT( clone );
            CHECK_NEXT_TERM( clone, "text", "combined" );

            CALL_NEXT( clone );
            CHECK_NEXT_TERM( clone, "text", "package" );

            next_status = lcn_term_enum_next( clone );
            CuAssertTrue(tc, LCN_ERR_ITERATOR_NO_NEXT == next_status );
            LCN_TEST( lcn_term_enum_close( clone ) );

            apr_pool_destroy( clone_pool );
        }

        CALL_NEXT( term_enum );
        CHECK_NEXT_TERM( term_enum, "text", "combined" );

        {
            apr_pool_t *clone_pool;
            lcn_term_enum_t *clone;

            LCN_TEST( apr_pool_create( &clone_pool, pool ) );
            LCN_TEST( lcn_term_enum_clone( term_enum, &clone, pool ) );

            CHECK_NEXT_TERM( clone, "text", "combined" );

            CALL_NEXT( clone );
            CHECK_NEXT_TERM( clone, "text", "package" );

            next_status = lcn_term_enum_next( clone );
            CuAssertTrue(tc, LCN_ERR_ITERATOR_NO_NEXT == next_status );
            LCN_TEST( lcn_term_enum_close( clone ) );

            apr_pool_destroy( clone_pool );
        }


        CALL_NEXT( term_enum );
        CHECK_NEXT_TERM( term_enum, "text", "package" );

        {
            apr_pool_t *clone_pool;
            lcn_term_enum_t *clone;

            LCN_TEST( apr_pool_create( &clone_pool, pool ) );
            LCN_TEST( lcn_term_enum_clone( term_enum, &clone, pool ) );

            CHECK_NEXT_TERM( clone, "text", "package" );

            next_status = lcn_term_enum_next( clone );
            CuAssertTrue(tc, LCN_ERR_ITERATOR_NO_NEXT == next_status );
            LCN_TEST( lcn_term_enum_close( clone ) );

            apr_pool_destroy( clone_pool );
        }


        next_status = lcn_term_enum_next( term_enum );
        CuAssertTrue(tc, LCN_ERR_ITERATOR_NO_NEXT == next_status );

        {
            apr_pool_t *clone_pool;
            lcn_term_enum_t *clone;

            LCN_TEST( apr_pool_create( &clone_pool, pool ) );
            LCN_TEST( lcn_term_enum_clone( term_enum, &clone, pool ) );

            next_status = lcn_term_enum_next( clone );
            CuAssertTrue(tc, LCN_ERR_ITERATOR_NO_NEXT == next_status );
            LCN_TEST( lcn_term_enum_close( clone ) );

            apr_pool_destroy( clone_pool );
        }

        LCN_TEST( lcn_term_enum_close( term_enum ) );
    }

    LCN_TEST( lcn_directory_close( dir ) );
    apr_pool_destroy( pool );
}

static void
test_indexing_terms( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_directory_t *dir;
    lcn_document_t *doc;
    lcn_field_t *field;
    lcn_index_writer_t *index_writer;
    lcn_field_type_t indexed_type = {0};

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_fs_directory_create( &dir, "new_dir", LCN_TRUE, pool ) );
    delete_files( tc, "new_dir" );
    LCN_TEST( lcn_index_writer_create_by_path( &index_writer,
                                               "new_dir",
                                               LCN_TRUE,
                                               pool  ));

    LCN_TEST( lcn_document_create( &doc, pool ));
    LCN_TEST( lcn_field_type_set_indexed( &indexed_type, LCN_TRUE ));
    LCN_TEST( lcn_field_create( &field, "text", "a", &indexed_type, pool ));
    LCN_TEST( lcn_document_add_field( doc, field ));
    LCN_TEST( lcn_index_writer_add_document( index_writer, doc ));

    lcn_index_writer_close( index_writer ) ;
    lcn_index_writer_optimize( index_writer ) ;

    {
        apr_pool_t *p;
        lcn_term_t *term;
        lcn_term_enum_t *term_enum;
        lcn_index_reader_t *reader;
        apr_status_t next_status;

        apr_pool_create( &p, pool );

        LCN_TEST( lcn_term_create( &term, "text", "a", LCN_TERM_TEXT_COPY, p ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader, "new_dir", p ));
        LCN_TEST( lcn_index_reader_terms( reader, &term_enum, p ));
        CALL_NEXT( term_enum );
        CHECK_NEXT_TERM( term_enum, "text", "a" );

        CuAssertTrue(tc, LCN_ERR_ITERATOR_NO_NEXT == lcn_term_enum_next( term_enum ));

        lcn_term_enum_close( term_enum );
        lcn_index_reader_close( reader );

        apr_pool_destroy( p );
    }

    apr_pool_destroy( pool );
}

CuSuite *
make_segment_term_enum_suite (void)
{
    CuSuite *s= CuSuiteNew();
    SUITE_ADD_TEST(s, test_segment_term_enum );
    SUITE_ADD_TEST(s, test_indexing_terms );
    return s;
}
