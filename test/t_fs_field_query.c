#include "test_all.h"
#include "lcn_bitvector.h"
#include "lcn_analysis.h"

static void
test_fs_field_query( CuTest* tc )
{
    apr_pool_t* pool;

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

        {
            /* try to search an then check result */
            lcn_searcher_t *searcher;
            lcn_query_t *query;
            lcn_hits_t *hits;
            lcn_document_t* doc;
            char* fval;

            LCN_TEST( lcn_index_searcher_create_by_reader( &searcher, reader, pool ));
            LCN_TEST( lcn_parse_query( &query, "dump2:23", pool ));
            LCN_TEST( lcn_searcher_search( searcher,
                                           &hits,
                                           query,
                                           NULL,
                                           pool ) );

            CuAssertIntEquals( tc, 1, lcn_hits_length( hits ));
            LCN_TEST( lcn_hits_doc( hits, &doc, 0, pool ) );
            LCN_TEST( lcn_document_get( doc, &fval, "string", pool ));
            CuAssertStrEquals( tc, "field23", fval );

            LCN_TEST( lcn_parse_query( &query, "dump2:30", pool ));
            LCN_TEST( lcn_searcher_search( searcher,
                                           &hits,
                                           query,
                                           NULL,
                                           pool ));
            CuAssertIntEquals( tc, 1, lcn_hits_length( hits ));
            LCN_TEST( lcn_hits_doc( hits, &doc, 0, pool ) );
            LCN_TEST( lcn_document_get( doc, &fval, "string", pool ));
            CuAssertStrEquals( tc, "field30", fval );

            LCN_TEST( lcn_parse_query( &query, "dump2:30 dump2:23", pool ));
            LCN_TEST( lcn_searcher_search( searcher,
                                           &hits,
                                           query,
                                           NULL,
                                           pool ));
            CuAssertIntEquals( tc, 2, lcn_hits_length( hits ));
            LCN_TEST( lcn_hits_doc( hits, &doc, 0, pool ) );
            LCN_TEST( lcn_document_get( doc, &fval, "string", pool ));
            CuAssertStrEquals( tc, "field30", fval );

            LCN_TEST( lcn_hits_doc( hits, &doc, 1, pool ) );
            LCN_TEST( lcn_document_get( doc, &fval, "string", pool ));
            CuAssertStrEquals( tc, "field23", fval );
        }
    }

    apr_pool_destroy( pool );
}

CuSuite*
make_fs_field_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_fs_field_query );

    return s;
}
