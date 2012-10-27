#include "test_all.h"
#include "lcn_bitvector.h"
#include "lcn_analysis.h"

static lcn_bool_t
range_function( void *data, unsigned int val, unsigned int n )
{
    int *vdata = (int*) data;
    return *vdata == val;
}

static void
test_bitvector_by_fs_field( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_bitvector_t* bitvector, *l_bitvector;

    apr_pool_create( &pool, main_pool );

    /* setup the indexes */
    {
        apr_hash_t *map;
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );

        delete_files( tc, "fs_index_1" );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_index_1",
                                                         "fs_dump_1",  /* 12 documents */
                                                         map,
                                                         LCN_FALSE, /* optimize */
                                                         pool ));

        delete_files( tc, "fs_index_2" );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_index_2",
                                                         "fs_dump_2",    /* 12 documents */
                                                         map,
                                                         LCN_TRUE, /* optimize */
                                                         pool ));
    }

    {
        /* reading fixed sized fields defined in two indexes */
        lcn_index_reader_t *reader1, *reader2, *reader;
        lcn_list_t *list;
        unsigned int date = 23;
        lcn_fs_field_t *fs_field;

        LCN_TEST( lcn_index_reader_create_by_path( &reader1, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader2, "fs_index_2", pool ));

        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list, pool ));
        LCN_TEST( lcn_index_reader_get_fs_field( reader, &fs_field, "dump2" ));

        LCN_TEST( lcn_index_reader_fs_int_field_bitvector( reader,
                                                           &bitvector,
                                                           "dump2",
                                                           range_function,
                                                           &date,
                                                           pool ));

        LCN_TEST( lcn_bitvector_create_by_int_fs_field ( &l_bitvector,
                                                         fs_field,
                                                         date,
                                                         pool ));

        /* check the interface of l_bitvector */

        /*
         * lcn_bitvector_size
         * lcn_bitvector_count
         * lcn_bitvector_get_bit
         */

        CuAssertIntEquals( tc, 24, lcn_bitvector_size( l_bitvector ));
        CuAssertIntEquals( tc, 24, lcn_bitvector_size( bitvector) );

        CuAssertIntEquals( tc, 1, lcn_bitvector_count( bitvector ));
        CuAssertIntEquals( tc, 1, lcn_bitvector_count( l_bitvector ));

        CuAssertIntEquals( tc, LCN_ERR_UNSUPPORTED_OPERATION, lcn_bitvector_set_bit( l_bitvector, 5 ));

        {
            /* try to search an then check result */
            lcn_searcher_t *searcher;
            lcn_query_t *query;
            lcn_hits_t *hits;
            lcn_document_t* doc;
            char* fval;

            LCN_TEST( lcn_index_searcher_create_by_reader( &searcher, reader, pool ));
            LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );
            LCN_TEST( lcn_searcher_search( searcher,
                                           &hits,
                                           query,
                                           l_bitvector,
                                           pool ) );

            CuAssertIntEquals( tc, 1, lcn_hits_length( hits ));

            LCN_TEST( lcn_hits_doc( hits, &doc, 0, pool ) );
            LCN_TEST( lcn_document_get( doc, &fval, "string", pool ));
            CuAssertStrEquals( tc, "field23", fval );
        }
    }

    apr_pool_destroy( pool );
}


static void
test_fs_field_bitvector( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_bitvector_t* bitvector;

    apr_pool_create( &pool, main_pool );

    /* setup the indexes */
    {
        apr_hash_t *map;
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );

        delete_files( tc, "fs_index_1" );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_index_1",
                                                         "fs_dump_1",
                                                         map,
                                                         LCN_FALSE, /* optimize */
                                                         pool ));

        delete_files( tc, "fs_index_2" );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_index_2",
                                                         "fs_dump_2",
                                                         map,
                                                         LCN_TRUE, /* optimize */
                                                         pool ));
    }

    {
        /* reading fixed sized fields defined in two indexes */
        lcn_index_reader_t *reader1, *reader2, *reader;
        lcn_list_t *list;
        unsigned int date = 23;

        LCN_TEST( lcn_index_reader_create_by_path( &reader1,
                                                   "fs_index_1",
                                                   pool ));

        LCN_TEST( lcn_index_reader_create_by_path( &reader2,
                                                   "fs_index_2",
                                                   pool ));

        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list, pool ));

        LCN_TEST( lcn_index_reader_fs_int_field_bitvector( reader,
                                                           &bitvector,
                                                           "dump2",
                                                           range_function,
                                                           &date,
                                                           pool ));

        CuAssertIntEquals( tc, 24, lcn_bitvector_size( bitvector) );
        CuAssertIntEquals( tc, 1, lcn_bitvector_count( bitvector ));

        {
            /* try to search an then check result */
            lcn_searcher_t *searcher;
            lcn_query_t *query;
            lcn_hits_t *hits;
            lcn_document_t* doc;
            char* fval;

            LCN_TEST( lcn_index_searcher_create_by_reader( &searcher, reader, pool ));
            LCN_TEST( lcn_match_all_docs_query_create( &query, pool ) );
            LCN_TEST( lcn_searcher_search( searcher,
                                           &hits,
                                           query,
                                           bitvector,
                                           pool ) );
            CuAssertIntEquals( tc, 1, lcn_hits_length( hits ));

            LCN_TEST( lcn_hits_doc( hits, &doc, 0, pool ) );
            LCN_TEST( lcn_document_get( doc, &fval, "string", pool ));
            CuAssertStrEquals( tc, "field23", fval );
        }
    }

    apr_pool_destroy( pool );
}

CuSuite*
make_fs_field_bitvector_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_fs_field_bitvector );
    SUITE_ADD_TEST( s, test_bitvector_by_fs_field );

    return s;
}
