#include "test_all.h"
#include "fs_field.h"
#include "lcn_index.h"
#include "lcn_analysis.h"
#include "lcn_bitvector.h"


static void
test_sorting_by_fixed_size_field( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_list_t *sort_fields;
    lcn_sort_field_t *sort_field;
    apr_hash_t *map;
    lcn_searcher_t *searcher;
    lcn_hits_t *hits;
    lcn_query_t *query;
    unsigned int i;
    unsigned int save_val = 0;

    apr_pool_create( &pool, main_pool );

    /* set up the index */

    LCN_TEST( lcn_analyzer_map_create( &map, pool ) );

    delete_files( tc, "fs_index_1" );
    LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_index_1",
                                                     "fs_dump_1",
                                                     map,
                                                     LCN_FALSE, /* optimize */
                                                     pool ));

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher, "fs_index_1", pool ));

    LCN_TEST( lcn_sort_field_create ( &sort_field,
                                      "int_1",
                                      LCN_SORT_FIELD_INT,
                                      LCN_FALSE, /* reverse */
                                      pool ));

    LCN_TEST( lcn_list_create( &sort_fields, 1, pool ));
    LCN_TEST( lcn_list_add( sort_fields, sort_field ) );
    LCN_TEST( lcn_match_all_docs_query_create( &query, pool ));

    LCN_TEST( lcn_searcher_search_sort( searcher, &hits, query, NULL, sort_fields, pool ) );

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t *doc;
        unsigned int v;

        LCN_TEST( lcn_hits_doc( hits, &doc, i, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &v ));
        CuAssertTrue( tc, save_val <= v );
        save_val = v;
    }

    /* now the same but with revers sort order */

    LCN_TEST( lcn_sort_field_create ( &sort_field,
                                      "int_1",
                                      LCN_SORT_FIELD_INT,
                                      LCN_TRUE, /* reverse */
                                      pool ));

    LCN_TEST( lcn_list_create( &sort_fields, 1, pool ));
    LCN_TEST( lcn_list_add( sort_fields, sort_field ) );
    LCN_TEST( lcn_match_all_docs_query_create( &query, pool ));

    LCN_TEST( lcn_searcher_search_sort( searcher, &hits, query, NULL, sort_fields, pool ) );

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t *doc;
        unsigned int v;

        LCN_TEST( lcn_hits_doc( hits, &doc, i, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &v ));
        CuAssertTrue( tc, save_val >= v );
        save_val = v;
    }

    {
        /* test lcn_index_reader_has_field */

        lcn_bool_t has_field;
        lcn_index_reader_t* r = lcn_index_searcher_reader( searcher );
        LCN_TEST( lcn_index_reader_has_field( r, &has_field, "int_1" ));
        CuAssertTrue( tc, has_field );
        LCN_TEST( lcn_index_reader_has_field( r, &has_field, "some_field" ));
        CuAssertTrue( tc, ! has_field );
    }

    delete_files( tc, "fs_index_1" );
    apr_pool_destroy( pool );
}

static void
test_multi_indexes( CuTest* tc )
{
    apr_pool_t *pool;
    unsigned int i;

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

        delete_files( tc, "fs_index_3" );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_index_3",
                                                         "fs_dump_3",
                                                         map,
                                                         LCN_TRUE, /* optimize */
                                                         pool ));
    }

    return;

    {
        /* reading fixed sized fields from non optimized index */

        lcn_index_reader_t *reader;
        lcn_document_t *doc;
        unsigned int val;

        LCN_TEST( lcn_index_reader_create_by_path( &reader, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_document( reader, &doc, 2, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 5, val );

        LCN_TEST( lcn_index_reader_close( reader ));
    }

    {
        /* modifying fixed sized fields in the optimized index */

        lcn_index_reader_t *reader;
        lcn_document_t *doc;
        unsigned int val;

        /* check field value first */

        LCN_TEST( lcn_index_reader_create_by_path( &reader, "fs_index_3", pool ));
        LCN_TEST( lcn_index_reader_document( reader, &doc, 0, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 3, val );

        LCN_TEST( lcn_index_reader_close( reader ));

        /* modify the value  3 -> 4   */

        LCN_TEST( lcn_index_reader_create_by_path( &reader, "fs_index_3", pool ));

        LCN_TEST( lcn_index_reader_set_int_value( reader,
                                                  0,    /* doc index */
                                                  "int_1",
                                                  4 )); /* int value */

        LCN_TEST( lcn_index_reader_document( reader, &doc, 0, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 4, val );
        LCN_TEST( lcn_index_reader_close( reader ));

        /* assert the value is still there */

        LCN_TEST( lcn_index_reader_create_by_path( &reader, "fs_index_3", pool ));
        LCN_TEST( lcn_index_reader_document( reader, &doc, 0, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 4, val );
        LCN_TEST( lcn_index_reader_close( reader ));
    }

    {
        /* reading fixed sized fields defined in two indexes */

        lcn_index_reader_t *reader1, *reader2, *reader;
        lcn_document_t *doc;
        unsigned int val;
        lcn_list_t *list;

        LCN_TEST( lcn_index_reader_create_by_path( &reader1, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader2, "fs_index_2", pool ));

        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list, pool ));
        LCN_TEST( lcn_index_reader_document( reader, &doc, 2, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 5, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 11, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 2, val );
        LCN_TEST( lcn_document_get_int( doc, "dump2", &val ) );
        CuAssertIntEquals( tc, 5, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 12, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 4, val );

        LCN_TEST( lcn_document_get_int( doc, "dump2", &val ));
        CuAssertIntEquals( tc, 5, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 6, val );
        LCN_TEST( lcn_document_get_int( doc, "dump2", &val ));
        CuAssertIntEquals( tc, 30, val );

        LCN_TEST( lcn_index_reader_close( reader ));
    }

    {
        /* reading fixed sized fields defined in three indexes */

        lcn_index_reader_t *reader1, *reader2, *reader3, *reader12, *reader;
        lcn_document_t *doc;
        unsigned int val;
        lcn_list_t *list, *list1;

        LCN_TEST( lcn_index_reader_create_by_path( &reader1, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader2, "fs_index_2", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader3, "fs_index_3", pool ));


        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader12, list, pool ));


        LCN_TEST( lcn_list_create( &list1, 10, pool ));
        LCN_TEST( lcn_list_add( list1, reader12 ));
        LCN_TEST( lcn_list_add( list1, reader3  ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list1, pool ));


        {
            lcn_list_t *field_infos;

            lcn_list_create( &field_infos, 10, pool );
            LCN_TEST( lcn_index_reader_get_field_infos( reader, field_infos, 0, 0 ));

            CuAssertIntEquals( tc, 7, lcn_list_size( field_infos ));
        }

        LCN_TEST( lcn_index_reader_document( reader, &doc, 2, pool ));

        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 5, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 11, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 2, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 12, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 4, val );

        LCN_TEST( lcn_document_get_int( doc, "dump2", &val ));
        CuAssertIntEquals( tc, 5, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 6, val );

        LCN_TEST( lcn_document_get_int( doc, "dump2", &val ));
        CuAssertIntEquals( tc, 30, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 24, pool ));
        LCN_TEST( lcn_document_get_int( doc, "dump3", &val ));
        CuAssertIntEquals( tc, 25, val );

        LCN_TEST( lcn_index_reader_close( reader ));
    }

    {
        /* modifying fixed sized fields defined in three indexes */

        lcn_index_reader_t *reader1, *reader2, *reader3, *reader12, *reader;
        lcn_document_t *doc;
        unsigned int val;
        lcn_list_t *list, *list1;

        LCN_TEST( lcn_index_reader_create_by_path( &reader1, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader2, "fs_index_2", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader3, "fs_index_3", pool ));

        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader12, list, pool ));


        LCN_TEST( lcn_list_create( &list1, 10, pool ));
        LCN_TEST( lcn_list_add( list1, reader12 ));
        LCN_TEST( lcn_list_add( list1, reader3  ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list1, pool ));
        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 6, val );

        CuAssertIntEquals( tc, LCN_ERR_DOCUMENT_NO_SUCH_FIELD,
                           lcn_document_get_int( doc, "dump1", &val ));

        /* modify the value  6 -> 2   */

        LCN_TEST( lcn_index_reader_set_int_value( reader,
                                                  13,    /* doc index */
                                                  "int_1",
                                                  2 )); /* int value */

        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 2, val );


        LCN_TEST( lcn_index_reader_close( reader ));

        /* assert the value is still there */

        LCN_TEST( lcn_index_reader_create_by_path( &reader1, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader2, "fs_index_2", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader3, "fs_index_3", pool ));


        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader12, list, pool ));


        LCN_TEST( lcn_list_create( &list1, 10, pool ));
        LCN_TEST( lcn_list_add( list1, reader12 ));
        LCN_TEST( lcn_list_add( list1, reader3  ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list1, pool ));

        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));
        CuAssertIntEquals( tc, 2, val );

        LCN_TEST( lcn_index_reader_close( reader ));
    }

    {
        /* adding new fields in three indexes */

        lcn_index_reader_t *reader1, *reader2, *reader3, *reader12, *reader;
        lcn_document_t *doc;
        unsigned int val;
        lcn_list_t *list, *list1;
        lcn_field_t* field;

        LCN_TEST( lcn_index_reader_create_by_path( &reader1, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader2, "fs_index_2", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader3, "fs_index_3", pool ));

        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader12, list, pool ));


        LCN_TEST( lcn_list_create( &list1, 10, pool ));
        LCN_TEST( lcn_list_add( list1, reader12 ));
        LCN_TEST( lcn_list_add( list1, reader3  ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list1, pool ));
        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));

        CuAssertIntEquals( tc, 2, val );

        CuAssertIntEquals( tc, LCN_ERR_DOCUMENT_NO_SUCH_FIELD, lcn_document_get_int( doc, "dump1", &val ));
        CuAssertIntEquals( tc, 2, val );

        /* now add field 'new_field'  */

        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "new_field",
                                               NULL,
                                               0,
                                               15,
                                               pool ));

        LCN_TEST( lcn_index_reader_add_fs_field_def( reader, field ));

        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "new_field",
                                               NULL,
                                               0,
                                               16,
                                               pool ));

        CuAssertIntEquals(tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION,
                          lcn_index_reader_add_fs_field_def( reader, field ));

        LCN_TEST( lcn_index_reader_set_int_value( reader,
                                                  13,    /* doc index */
                                                  "new_field",
                                                  4321 )); /* int value */

        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "new_field", &val ));
        CuAssertIntEquals( tc, 4321, val );

        LCN_TEST( lcn_index_reader_set_int_value( reader,
                                                  1,    /* doc index */
                                                  "new_field",
                                                  14321 )); /* int value */
        LCN_TEST( lcn_index_reader_close( reader ));

        /* assert the value is still there */
        reader1 = NULL;
        LCN_TEST( lcn_index_reader_create_by_path( &reader1, "fs_index_1", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader2, "fs_index_2", pool ));
        LCN_TEST( lcn_index_reader_create_by_path( &reader3, "fs_index_3", pool ));



        LCN_TEST( lcn_list_create( &list, 10, pool ));
        LCN_TEST( lcn_list_add( list, reader1 ));
        LCN_TEST( lcn_list_add( list, reader2 ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader12, list, pool ));


        LCN_TEST( lcn_list_create( &list1, 10, pool ));
        LCN_TEST( lcn_list_add( list1, reader12 ));
        LCN_TEST( lcn_list_add( list1, reader3  ));

        LCN_TEST( lcn_multi_reader_create_by_sub_readers( &reader, list1, pool ));

        LCN_TEST( lcn_index_reader_document( reader, &doc, 13, pool ));
        LCN_TEST( lcn_document_get_int( doc, "new_field", &val ));
        CuAssertIntEquals( tc, 4321, val );

        LCN_TEST( lcn_index_reader_document( reader, &doc, 1, pool ));
        LCN_TEST( lcn_document_get_int( doc, "new_field", &val ));
        CuAssertIntEquals( tc, 14321, val );

        for( i = 0; i < lcn_index_reader_num_docs( reader ); i++ )
        {
            LCN_TEST( lcn_index_reader_set_int_value( reader,
                                                      i,      /* doc index */
                                                      "new_field",
                                                      (i * 100 + 73) )); /* int value */
        }

        LCN_TEST( lcn_index_reader_close( reader ));
    }

    delete_files( tc, "fs_index_1" );
    delete_files( tc, "fs_index_2" );
    delete_files( tc, "fs_index_3" );

    apr_pool_destroy( pool );
}

static void
test_reading_index( CuTest *tc, int docs, int fields_count )
{
    lcn_index_reader_t *index_reader;
    unsigned int i;
    apr_pool_t *pool;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_reader_create_by_path( &index_reader,
                                               "test_fixed_size_index",
                                               pool ));

    for( i = 0; i < docs; i++ )
    {
        lcn_document_t *doc;
        lcn_list_t *fields;
        char *buf, *buf2, *buf16, *buf1;
        unsigned int len;
        unsigned int val, val2, val16, val1;
        apr_status_t stat;

        if ( i == 4 )
        {
            continue;
        }

        LCN_TEST( lcn_index_reader_document( index_reader, &doc, i, pool ));

        fields = lcn_document_get_fields( doc );

        CuAssertIntEquals( tc, fields_count, lcn_list_size( fields ));

        stat = lcn_document_get( doc, &buf, "int_1", pool );
        CuAssertIntEquals( tc, LCN_ERR_DOCUMENT_FIELD_IS_BINARY, stat );

        LCN_TEST( lcn_document_get_binary_field_value( doc, "int_1", &buf, &len, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_1", &val ));

        stat = lcn_document_get_binary_field_value( doc, "int_2", &buf2, &len, pool );
        CuAssertIntEquals(tc, docs < 3 ? LCN_ERR_DOCUMENT_NO_SUCH_FIELD : APR_SUCCESS, stat );

        stat = lcn_document_get_int( doc, "int_2", &val2 );
        CuAssertIntEquals(tc, docs < 3 ? LCN_ERR_DOCUMENT_NO_SUCH_FIELD : APR_SUCCESS, stat );

        LCN_TEST( lcn_document_get_binary_field_value( doc, "int_16", &buf16, &len, pool ));
        LCN_TEST( lcn_document_get_int( doc, "int_16", &val16 ));

        stat = lcn_document_get_binary_field_value( doc, "bit", &buf1, &len, pool );
        CuAssertIntEquals(tc, docs < 4 ? LCN_ERR_DOCUMENT_NO_SUCH_FIELD : APR_SUCCESS, stat );
        stat = lcn_document_get_int( doc, "bit", &val1 );
        CuAssertIntEquals(tc, docs < 4 ? LCN_ERR_DOCUMENT_NO_SUCH_FIELD : APR_SUCCESS, stat );

        switch (i)
        {
        case 0:
            CuAssertTrue( tc, 3 == buf[0] );
            CuAssertIntEquals(tc, 3, val );
            CuAssertTrue( tc, 3 == buf16[0] );
            CuAssertTrue( tc, 1 == buf16[1] );
            CuAssertIntEquals( tc, 259, val16 );

            if ( docs >= 3 )
            {
                CuAssertTrue( tc, 1 == buf2[0] );
                CuAssertTrue( tc, 0 == buf2[1] );
                CuAssertIntEquals( tc, 1, val2 );
            }

            if ( docs >= 4 )
            {
                CuAssertTrue( tc, 1 == buf1[0] );
                CuAssertIntEquals( tc, 1, val1 );
            }
            break;

        case 1:
            CuAssertTrue( tc, 5 == buf[0] );
            CuAssertIntEquals(tc, 5, val );
            CuAssertTrue( tc, 1 == buf16[0] );
            CuAssertTrue( tc, 0 == buf16[1] );
            CuAssertIntEquals( tc, 1, val16 );

            if ( docs >= 3 )
            {
                CuAssertTrue( tc, 1 == buf2[0] );
                CuAssertTrue( tc, 0 == buf2[1] );
                CuAssertIntEquals( tc, 1, val2 );
            }

            if ( docs >= 4 )
            {
                CuAssertTrue( tc, 1 == buf1[0] );
                CuAssertIntEquals( tc, 1, val1 );
            }

            {
                unsigned int v;
                stat = lcn_document_get_int( doc, "new_field", &v );

                if ( docs < 5 )
                {
                    CuAssertIntEquals(tc, LCN_ERR_DOCUMENT_NO_SUCH_FIELD, stat );
                }
                else
                {
                    CuAssertIntEquals(tc, APR_SUCCESS, stat );
                    CuAssertIntEquals(tc, 1234, v );
                }
            }

            break;
        case 2:
            CuAssertTrue( tc, 1 == buf[0] );
            CuAssertIntEquals(tc, 1, val );
            CuAssertTrue( tc, 10 == buf2[0] );
            CuAssertIntEquals( tc, 10, val2 );
            CuAssertTrue( tc, 1 == buf16[0] );
            CuAssertTrue( tc, 0 == buf16[1] );
            CuAssertIntEquals( tc, 1, val16 );

            if ( docs >= 4 )
            {
                CuAssertTrue( tc, 1 == buf1[0] );
                CuAssertIntEquals( tc, 1, val1 );
            }

            break;
        case 3:
            CuAssertTrue( tc, 7 == buf[0] );
            CuAssertIntEquals(tc, 7, val );
            CuAssertTrue( tc, 1 == buf2[0] );
            CuAssertIntEquals( tc, 1, val2 );
            CuAssertTrue( tc, 7 == buf16[0] );
            CuAssertTrue( tc, 5 == buf16[1] );
            CuAssertIntEquals( tc, 1287, val16 );

            if ( docs >= 4 )
            {
                CuAssertTrue( tc, 0 == buf1[0] );
                CuAssertIntEquals( tc, 0, val1 );
            }

            break;
        }
    }

    apr_pool_destroy( pool );
}

static void
test_indexing( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    char field_content[5];
    char default_content[5];
    FILE *log_stream = fopen("fs_dump", "w" );

    memset( field_content, 0, 5 );
    memset(default_content, 0, 5 );

    apr_pool_create( &pool, main_pool );

    delete_files( tc, "test_fixed_size_index" );

    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_TRUE, pool ) );
    lcn_index_writer_set_log_stream( index_writer, log_stream );
    lcn_index_writer_set_max_buffered_docs( index_writer, 2 );
    lcn_index_writer_set_merge_factor( index_writer, 2 );

    {
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 3;
        default_content[0] = 1;

        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        field_content[1] = 1;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_16",
                                               field_content,
                                               default_content,
                                               16,
                                               pool ) );
        field_content[1] = 0;

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    fclose( log_stream );


    /* check if we can use the dumpfile */

    {
        apr_hash_t *map;
        delete_files( tc, "fs_dump_test" );
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_dump_test",
                                                         "fs_dump",
                                                         map,
                                                         LCN_TRUE,
                                                         pool ));
        compare_directories(tc, "fs_dump_test", "test_fixed_size_index" );
    }

    test_reading_index( tc, 1, 2 );

    {
        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 5;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    test_reading_index( tc, 2, 2 );

    {
        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 10;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_2",
                                               field_content,
                                               default_content,
                                               15,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    test_reading_index( tc, 3, 3 );

    {
        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 7;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ) );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        field_content[1] = 5;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_16",
                                               field_content,
                                               default_content,
                                               16,
                                               pool ) );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        field_content[1] = 0;
        field_content[0] = 0;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "bit",
                                               field_content,
                                               default_content,
                                               1,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    test_reading_index( tc, 4, 4 );

    {
        /* Test defining and modifying new fs field */

        lcn_field_t* field;
        lcn_index_reader_t *index_reader;

        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "new_field",
                                               NULL,
                                               0,
                                               24,
                                               pool ));

        LCN_TEST( lcn_index_reader_create_by_path( &index_reader,
                                                   "test_fixed_size_index",
                                                   pool ));

        LCN_TEST( lcn_index_reader_add_fs_field_def( index_reader, field ));

        LCN_TEST( lcn_index_reader_set_int_value( index_reader,
                                                  1,
                                                  "new_field",
                                                  1234 ));

        LCN_TEST( lcn_index_reader_close( index_reader ));

        /* try reading */

        test_reading_index( tc, 5, 5 );

        /* try adding conflicting field */

        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "new_field",
                                               NULL,
                                               0,
                                               23,
                                               pool ));

        CuAssertIntEquals(tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION,
                          lcn_index_reader_add_fs_field_def( index_reader, field ));
    }

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_1( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             1,  /* field_size    */
                                             NULL, /* directory */
                                             pool ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 1, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));

    LCN_TEST( lcn_fs_field_set_int_value( field, 1, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 1, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 1, val );

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_1_default( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             1,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field, 1 ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));

    LCN_TEST( lcn_fs_field_set_int_value( field, 1, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 1, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_8( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             8,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 255, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 255, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 13, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 13, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    apr_pool_destroy( pool );
}


static void
test_fs_field_size_8d( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             8,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t *)field, 200 ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 200, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 200, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 255, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 255, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 13, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 13, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 200, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 200, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 200, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 42, 2000 ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1500 ));
    CuAssertIntEquals( tc, 200, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2000 ));
    CuAssertIntEquals( tc, 42, val );

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_16( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             16,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 256, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 256, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 255 * 255, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 255 * 255, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 13, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 13, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_16d( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             16,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field, 400 ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 400, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 400, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 256, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 256, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 255 * 255, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 255 * 255, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 13, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 13, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 400, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 400, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 400, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_int( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;
    unsigned int large_int = 255 << ((sizeof(int)-1) * 8);

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             8 * sizeof(int),  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, large_int, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, large_int, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 256, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 256, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 255 * 255, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 255 * 255, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 1111111, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 1111111, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, large_int, val );

    apr_pool_destroy( pool );
}


static void
test_fs_field_size_intd( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;
    unsigned int large_int = 255 << ((sizeof(int)-1) * 8);

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             8 * sizeof(int),  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field, large_int + 5 ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, large_int + 5, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, large_int + 5, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 256, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 256, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 255 * 255, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 255 * 255, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 13, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 13, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, large_int + 5, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, large_int + 5, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, large_int + 5, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 7, val );

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_2( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 3, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 3, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 2, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 3, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 1, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 3, val );

    apr_pool_destroy( pool );
}

static void
test_fs_field_size_2d( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field, 2 ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 3, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 255, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 3, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 0, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 13, 5 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 4 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 3 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 3, val );

    apr_pool_destroy( pool );
}

#define CHECK_CHAR_VAL(x,y)                                             \
    CuAssertIntEquals(tc, ((unsigned int) ((unsigned char)x[0])),((unsigned int) ((unsigned char)y[0]))); \
    CuAssertIntEquals(tc, ((unsigned int) (unsigned char)x[1]),((unsigned int) (unsigned char)y[1])); \
    CuAssertIntEquals(tc, ((unsigned int) (unsigned char)x[2]),((unsigned int) (unsigned char)y[2])); \
    CuAssertIntEquals(tc, ((unsigned int) (unsigned char)x[3]),((unsigned int) (unsigned char)y[3])); \
    CuAssertIntEquals(tc, ((unsigned int) (unsigned char)x[4]),((unsigned int) (unsigned char)y[4])); \
    CuAssertIntEquals(tc, ((unsigned int) (unsigned char)(x[5]&7)),((unsigned int) (unsigned char)(y[5]&7)));

//extern int CHECK;

static void
test_fs_field_size_char( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field, *f1;
    char dv[6] = { 'f', 'r', '3', 'c', '6', 17 };
    char val[6];
    char v1[6] = { 'a', 'b', 'c', 'd', 'e', 128 };
    char v2[6] = { 'a', 'b', 'c', 'd', 'e', 0 };
    char v3[6] = { '3', 'x', 'c', 'd', 'e', 1 };

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,   /* docs_count    */
                                             43,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_value( (lcn_directory_fs_field_t*) field, dv ));

    LCN_TEST( lcn_fs_field_value( field, val, 0 ));
    CHECK_CHAR_VAL(val, dv);

    LCN_TEST( lcn_fs_field_value( field, val, 100 ));
    CHECK_CHAR_VAL(val, dv);

    LCN_TEST( lcn_fs_field_set_value( field, v1, 0 ));

    LCN_TEST( lcn_fs_field_value( field, val, 0 ));
    CHECK_CHAR_VAL(val, v2);

    LCN_TEST( lcn_fs_field_set_value( field, v1, 1 ));
    LCN_TEST( lcn_fs_field_value( field, val, 1 ));
    CHECK_CHAR_VAL( val, v2 );

    LCN_TEST( lcn_fs_field_set_value( field, dv, 1 ));
    LCN_TEST( lcn_fs_field_value( field, val, 1 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_set_value( field, v3, 5 ));
    LCN_TEST( lcn_fs_field_value( field, val, 5 ));
    CHECK_CHAR_VAL( val, v3 );

    LCN_TEST( lcn_fs_field_value( field, val, 4 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( field, val, 3 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( field, val, 2 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( field, val, 1 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( field, val, 0 ));
    CHECK_CHAR_VAL( val, v2 );

    {
        lcn_ostream_t *os;
        LCN_TEST( lcn_fs_ostream_create( &os, "fsfile", pool ));
        LCN_TEST( lcn_fs_field_write_info( (lcn_directory_fs_field_t*) field, os ));
        LCN_TEST( lcn_fs_field_write_content( (lcn_directory_fs_field_t*) field, os ));
        LCN_TEST( lcn_ostream_close( os ));
    }

    {
        lcn_istream_t *is;
        lcn_directory_fs_field_t* f;
        LCN_TEST( lcn_istream_create( &is, "fsfile", pool ));
        LCN_TEST( lcn_directory_fs_field_read( &f, "tfield", is, pool ));
        f1 = (lcn_fs_field_t*) f;
        LCN_TEST( lcn_istream_close( is ));
    }

    LCN_TEST( lcn_fs_field_value( f1, val, 0 ));
    CHECK_CHAR_VAL(val, v2);

    LCN_TEST( lcn_fs_field_value( f1, val, 100 ));
    CHECK_CHAR_VAL(val, dv);

    LCN_TEST( lcn_fs_field_value( f1, val, 10000 ));
    CHECK_CHAR_VAL(val, dv);

    LCN_TEST( lcn_fs_field_value( f1, val, 1 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( f1, val, 5 ));
    CHECK_CHAR_VAL( val, v3 );

    LCN_TEST( lcn_fs_field_value( f1, val, 4 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( f1, val, 3 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( f1, val, 2 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( f1, val, 1 ));
    CHECK_CHAR_VAL( val, dv );

    LCN_TEST( lcn_fs_field_value( f1, val, 0 ));
    CHECK_CHAR_VAL( val, v2 );

    apr_pool_destroy( pool );
}

static void
test_indexing_errors( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    char field_content[5];
    char default_content[5];
    FILE *log_stream = fopen("fs_dump", "w" );

    memset( field_content, 0, 5 );
    memset(default_content, 0, 5 );

    apr_pool_create( &pool, main_pool );

    delete_files( tc, "test_fixed_size_index" );

    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_TRUE, pool ) );
    lcn_index_writer_set_log_stream( index_writer, log_stream );
    lcn_index_writer_set_max_buffered_docs( index_writer, 2 );
    lcn_index_writer_set_merge_factor( index_writer, 2 );

    {
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 3;
        default_content[0] = 1;

        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        field_content[1] = 1;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_16",
                                               field_content,
                                               default_content,
                                               16,
                                               pool ) );
        field_content[1] = 0;

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    fclose( log_stream );

    /* check if we can use the dumpfile */

    {
        apr_hash_t *map;
        delete_files( tc, "fs_dump_test" );
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_dump_test",
                                                         "fs_dump",
                                                         map,
                                                         LCN_TRUE,
                                                         pool ));
        compare_directories(tc, "fs_dump_test", "test_fixed_size_index" );
    }

    test_reading_index( tc, 1, 2 );

    {
        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 5;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    test_reading_index( tc, 2, 2 );

    {
        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 10;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_2",
                                               field_content,
                                               default_content,
                                               15,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    test_reading_index( tc, 3, 3 );

    {
        apr_status_t stat;

        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 7;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               4,
                                               pool ));

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        field_content[1] = 5;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_16",
                                               field_content,
                                               default_content,
                                               16,
                                               pool ) );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        field_content[1] = 0;
        field_content[0] = 0;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "bit",
                                               field_content,
                                               default_content,
                                               1,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        stat = lcn_index_writer_add_document( index_writer, document );
        CuAssertIntEquals( tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION, stat );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    apr_pool_destroy( pool );
}

static void
test_indexing_errors_default_val( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    char field_content[5];
    char default_content[5];
    FILE *log_stream = fopen("fs_dump", "w" );

    memset( field_content, 0, 5 );
    memset(default_content, 0, 5 );

    apr_pool_create( &pool, main_pool );

    delete_files( tc, "test_fixed_size_index" );

    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_TRUE, pool ) );
    lcn_index_writer_set_log_stream( index_writer, log_stream );
    lcn_index_writer_set_max_buffered_docs( index_writer, 2 );
    lcn_index_writer_set_merge_factor( index_writer, 2 );

    {
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 3;
        default_content[0] = 1;

        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        field_content[1] = 1;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_16",
                                               field_content,
                                               default_content,
                                               16,
                                               pool ) );
        field_content[1] = 0;

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    fclose( log_stream );

    /* check if we can use the dumpfile */

    {
        apr_hash_t *map;
        delete_files( tc, "fs_dump_test" );
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_dump_test",
                                                         "fs_dump",
                                                         map,
                                                         LCN_TRUE,
                                                         pool ));
        compare_directories(tc, "fs_dump_test", "test_fixed_size_index" );
    }

    test_reading_index( tc, 1, 2 );

    {
        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 5;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    test_reading_index( tc, 2, 2 );

    {
        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        field_content[0] = 10;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_2",
                                               field_content,
                                               default_content,
                                               15,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    test_reading_index( tc, 3, 3 );

    {
        apr_status_t stat;

        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_FALSE, pool ) );
        LCN_TEST( lcn_document_create( &document, pool ) );

        default_content[0] = 7;
        field_content[0]   = 7;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_1",
                                               field_content,
                                               default_content,
                                               3,
                                               pool ));

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        field_content[1] = 5;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "int_16",
                                               field_content,
                                               default_content,
                                               16,
                                               pool ) );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        field_content[1] = 0;
        field_content[0] = 0;
        LCN_TEST( lcn_field_create_fixed_size( &field,
                                               "bit",
                                               field_content,
                                               default_content,
                                               1,
                                               pool ) );

        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        stat = lcn_index_writer_add_document( index_writer, document );
        CuAssertIntEquals( tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION, stat );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    apr_pool_destroy( pool );
}

static void
test_create_fixed_size_by_ints( CuTest* tc )
{
    apr_pool_t *pool;
    unsigned int test_val = 0x9CF0630F;
    FILE *log_stream = fopen("int_dump", "w" );

    apr_pool_create( &pool, main_pool );
    delete_files( tc, "test_fixed_size_index" );


    {
        lcn_index_writer_t *index_writer;
        lcn_document_t *document;
        lcn_field_t *field;

        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_fixed_size_index", LCN_TRUE, pool ) );
        lcn_index_writer_set_log_stream( index_writer, log_stream );
        LCN_TEST( lcn_document_create( &document, pool ) );

        LCN_TEST( lcn_field_create_fixed_size_by_ints( &field,
                                                       "int32",
                                                       test_val /* 2632999695 */,
                                                       0,
                                                       32,
                                                       pool ));
        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
        LCN_TEST( lcn_index_writer_close( index_writer ) );
    }

    fclose( log_stream );

    {
        lcn_index_reader_t *index_reader;
        lcn_document_t *doc;
        apr_status_t stat;
        char *buf;
        unsigned int val;

        LCN_TEST( lcn_index_reader_create_by_path( &index_reader,
                                                   "test_fixed_size_index",
                                                   pool ));

        LCN_TEST( lcn_index_reader_document( index_reader,
                                             &doc,
                                             0,
                                             pool ));

        stat = lcn_document_get( doc, &buf, "int32", pool );
        CuAssertIntEquals( tc, LCN_ERR_DOCUMENT_FIELD_IS_BINARY, stat );
        stat = lcn_document_get_int( doc, "int32", &val );
        CuAssertIntEquals( tc, test_val, (unsigned int) val );
        lcn_index_reader_close( index_reader );
    }

    apr_pool_destroy( pool );
}

#define CHECK_INT( INDEX, DOC_NUM, FIELD_NAME, TEST_VAL )               \
{                                                                       \
    lcn_index_reader_t *index_reader;                                   \
    lcn_document_t *doc;                                                \
    unsigned int val;                                                   \
                                                                        \
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, INDEX, pool )); \
    LCN_TEST( lcn_index_reader_document( index_reader, &doc, DOC_NUM, pool )); \
    LCN_TEST( lcn_document_get_int( doc, FIELD_NAME, &val ));           \
    CuAssertIntEquals( tc, TEST_VAL, val );                             \
    LCN_TEST( lcn_index_reader_close( index_reader ));                  \
}

static void
test_adding_two_equal_fields( CuTest *tc )
{
    apr_pool_t *pool;

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

        delete_files( tc, "fs_index_3" );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "fs_index_3",
                                                         "fs_dump_3",
                                                         map,
                                                         LCN_TRUE, /* optimize */
                                                         pool ));
    }

    /* int_16 ist in beiden Indexen vorhanden */

    {
        lcn_directory_t *dir;
        lcn_list_t *dir_list;
        unsigned int max_doc_1;
        lcn_index_writer_t *index_writer;

        LCN_TEST( lcn_fs_directory_create( &dir, "fs_index_2", LCN_FALSE, pool ));
        LCN_TEST( lcn_list_create( &dir_list, 10, pool ));
        LCN_TEST( lcn_list_add( dir_list, dir ));

        {
            lcn_index_reader_t *index_reader;
            LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "fs_index_1", pool ));
            max_doc_1 = lcn_index_reader_max_doc( index_reader );
            LCN_TEST( lcn_index_reader_close( index_reader ));
        }

        {
            lcn_index_reader_t *index_reader;
            LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "fs_index_2", pool ));
            LCN_TEST( lcn_index_reader_close( index_reader ));
        }

        /* check some values */

        CHECK_INT( "fs_index_2", 0,  "int_1", 4 );
        CHECK_INT( "fs_index_1", max_doc_1 - 1 , "int_1", 2 );
        CHECK_INT( "fs_index_2", 7, "int_10", 259 );

        LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "fs_index_1", LCN_FALSE, pool ) );
        LCN_TEST( lcn_index_writer_add_indexes( index_writer, dir_list ));
        LCN_TEST( lcn_index_writer_close( index_writer ));

        CHECK_INT( "fs_index_1", max_doc_1 - 2, "int_1", 3 );
        CHECK_INT( "fs_index_1", max_doc_1 - 1, "int_1", 2 );
        CHECK_INT( "fs_index_1", max_doc_1,     "int_1", 4 );

        /* check int_10 field */

        CHECK_INT( "fs_index_1", 0, "int_10", 1 );
        CHECK_INT( "fs_index_1", 1, "int_10", 1 );
        CHECK_INT( "fs_index_1", max_doc_1 + 6, "int_10", 1   );
        CHECK_INT( "fs_index_1", max_doc_1 + 7, "int_10", 259 );
        CHECK_INT( "fs_index_1", max_doc_1 + 8, "int_10", 1   );
        CHECK_INT( "fs_index_1", max_doc_1 + 9, "int_10", 1   );
    }
}

/**
 * Tests setting and getting field values of a directory and
 * multi fs field  without storing the fiel
 */
static void
test_multi_fs_field_create( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field;
    unsigned int val = 17;
    lcn_fs_field_t *m_field;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field,
                                             "tfield",
                                             0,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field, 3 ));

    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 3, val );

    LCN_TEST( lcn_fs_field_int_value( field, &val, 100 ));
    CuAssertIntEquals( tc, 3, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 1, 0 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 2, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_set_int_value( field, 7, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
    CuAssertIntEquals( tc, 3, val );

    {
        lcn_list_t *field_list;
        unsigned int offset = 0;
        LCN_TEST( lcn_list_create( &field_list, 10, pool ));
        LCN_TEST( lcn_list_add( field_list, field ));
        LCN_TEST( lcn_multi_fs_field_create_by_subfields( &m_field, field_list, &offset, pool ));
    }

    CuAssertStrEquals( tc, "tfield", lcn_fs_field_name( field ));
    CuAssertStrEquals( tc, "tfield", lcn_fs_field_name( (lcn_fs_field_t*) m_field ));

    CuAssertIntEquals( tc, 2, lcn_fs_field_docs_count( field ));
    CuAssertIntEquals( tc, 2, lcn_fs_field_docs_count( (lcn_fs_field_t*) m_field ));

    CuAssertIntEquals( tc, 2, lcn_fs_field_data_size( field ));
    CuAssertIntEquals( tc, 2, lcn_fs_field_data_size( (lcn_fs_field_t*) m_field ));

    {
        const char *defval = lcn_fs_field_default_val( field );
        CuAssertIntEquals( tc, 3, (int) (*defval) );

        defval = lcn_fs_field_default_val( (lcn_fs_field_t*) m_field );
        CuAssertIntEquals( tc, 3, (int) (*defval) );
    }

    CuAssertIntEquals( tc, 1, lcn_fs_field_is_modified( field ));
    CuAssertIntEquals( tc, 1, lcn_fs_field_is_modified( (lcn_fs_field_t*) m_field ));

    /* Test field_value */
    {
        char val;
        LCN_TEST( lcn_fs_field_value( field, &val, 0 ));
        CuAssertIntEquals( tc, 1, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_value( (lcn_fs_field_t*) m_field, &val, 0));
        CuAssertIntEquals( tc, 1, (int) val );

        LCN_TEST( lcn_fs_field_value( field, &val, 1 ));
        CuAssertIntEquals( tc, 3, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_value( (lcn_fs_field_t*) m_field, &val, 1));
        CuAssertIntEquals( tc, 3, (int) val );

        LCN_TEST( lcn_fs_field_value( field, &val, 2 ));
        CuAssertIntEquals( tc, 3, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_value( (lcn_fs_field_t*) m_field, &val, 2));
        CuAssertIntEquals( tc, 3, (int) val );

        LCN_TEST( lcn_fs_field_value( field, &val, 20 ));
        CuAssertIntEquals( tc, 3, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_value( (lcn_fs_field_t*) m_field, &val, 20));
        CuAssertIntEquals( tc, 3, (int) val );
    }

    /* Test int_value */
    {
        unsigned int val;
        LCN_TEST( lcn_fs_field_int_value( field, &val, 0 ));
        CuAssertIntEquals( tc, 1, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_int_value( (lcn_fs_field_t*) m_field, &val, 0));
        CuAssertIntEquals( tc, 1, (int) val );

        LCN_TEST( lcn_fs_field_int_value( field, &val, 1 ));
        CuAssertIntEquals( tc, 3, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_int_value( (lcn_fs_field_t*) m_field, &val, 1));
        CuAssertIntEquals( tc, 3, (int) val );

        LCN_TEST( lcn_fs_field_int_value( field, &val, 2 ));
        CuAssertIntEquals( tc, 3, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_int_value( (lcn_fs_field_t*) m_field, &val, 2));
        CuAssertIntEquals( tc, 3, (int) val );

        LCN_TEST( lcn_fs_field_int_value( field, &val, 20 ));
        CuAssertIntEquals( tc, 3, (int) val );
        val = 0;
        LCN_TEST( lcn_fs_field_int_value( (lcn_fs_field_t*) m_field, &val, 20));
        CuAssertIntEquals( tc, 3, (int) val );
    }

    /* Test set int value */
    {
        unsigned int val;

        LCN_TEST( lcn_fs_field_set_int_value( m_field, 2, 5 ));
        LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
        CuAssertIntEquals( tc, 2, val );

        LCN_TEST( lcn_fs_field_set_int_value( m_field, 1, 5 ));
        LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
        CuAssertIntEquals( tc, 1, val );
    }

    /* Test set value */
    {
        unsigned int val;
        char val2 = (char) 2;
        char val1 = (char) 1;

        LCN_TEST( lcn_fs_field_set_value( m_field, &val2, 5 ));
        LCN_TEST( lcn_fs_field_int_value( m_field, &val, 5 ));
        LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));
        CuAssertIntEquals( tc, 2, val );

        LCN_TEST( lcn_fs_field_set_value( m_field, &val1, 5 ));
        LCN_TEST( lcn_fs_field_int_value( m_field, &val, 5 ));
        LCN_TEST( lcn_fs_field_int_value( field, &val, 5 ));

        CuAssertIntEquals( tc, 1, val );
    }

    /* Test implicit field extension */
    {
        unsigned int val;

        CuAssertIntEquals( tc, 6, lcn_fs_field_docs_count( field ));
        CuAssertIntEquals( tc, 6, lcn_fs_field_docs_count( m_field ));

        LCN_TEST( lcn_fs_field_set_int_value( m_field, 2, 20 ));
        LCN_TEST( lcn_fs_field_int_value( field, &val, 20 ));
        CuAssertIntEquals( tc, 2, val );

        CuAssertIntEquals( tc, 21, lcn_fs_field_docs_count( field ));
        CuAssertIntEquals( tc, 21, lcn_fs_field_docs_count( m_field ));
    }

    apr_pool_destroy( pool );
}

static void
test_multi_fs_fields_incomplete_third( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field_a, *field_b;
    unsigned int val = 17;
    lcn_fs_field_t *m_field;
    lcn_list_t *field_list;
    unsigned int i;
    unsigned int offset[10];

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field_a,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_a, 3 ));

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_b, 3 ));

    offset[0] =  0;
    offset[1] = 20;
    offset[2] = 25;

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));
    LCN_TEST( lcn_list_add( field_list, NULL ));

    CuAssertIntEquals( tc, 5, lcn_fs_field_docs_count( field_b ));
    CuAssertIntEquals( tc, 0, offset[0] );

    LCN_TEST( lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));
    CuAssertIntEquals( tc, 25, lcn_fs_field_docs_count( m_field ));

    /* test getting default value */

    for( i = 0; i < 30; i++ )
    {
        LCN_TEST( lcn_fs_field_int_value( m_field, &val, 0 ));
        CuAssertIntEquals( tc, 3, val );
    }

    /* test modified flag */

    CuAssertTrue( tc, ! lcn_fs_field_is_modified( m_field ));

    CuAssertIntEquals( tc, LCN_ERR_UNSUPPORTED_OPERATION, lcn_fs_field_set_int_value( m_field, 5, 26 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 26 ));
    CuAssertIntEquals( tc, 3, val );
    CuAssertIntEquals( tc, LCN_ERR_UNSUPPORTED_OPERATION, lcn_fs_field_set_int_value( m_field, 5, 25 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 25 ));
    CuAssertIntEquals( tc, 3, val );

    LCN_TEST(lcn_fs_field_set_int_value( m_field, 5, 24 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 24 ));
    CuAssertIntEquals( tc, 5, val );

    apr_pool_destroy( pool );
}

static void
test_multi_fs_fields_incomplete_second( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field_a, *field_b;
    unsigned int val = 17;
    lcn_fs_field_t *m_field;
    lcn_list_t *field_list;
    unsigned int i;
    unsigned int offset[10];

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field_a,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_a, 3 ));

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_b, 3 ));

    offset[0] =  0;
    offset[1] = 20;
    offset[2] = 25;

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, NULL ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    CuAssertIntEquals( tc, 5, lcn_fs_field_docs_count( field_b ));
    CuAssertIntEquals( tc, 0, offset[0] );

    LCN_TEST( lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* test getting default value */

    for( i = 0; i < 30; i++ )
    {
        LCN_TEST( lcn_fs_field_int_value( m_field, &val, 0 ));
        CuAssertIntEquals( tc, 3, val );
    }

    /* test modified flag */

    CuAssertTrue( tc, ! lcn_fs_field_is_modified( m_field ));
    CuAssertIntEquals( tc, LCN_ERR_UNSUPPORTED_OPERATION, lcn_fs_field_set_int_value( m_field, 5, 23 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 23 ));
    CuAssertIntEquals( tc, 3, val );

    CuAssertTrue( tc, ! lcn_fs_field_is_modified( m_field ));
    LCN_TEST( lcn_fs_field_int_value( field_a, &val, 3 ));
    CuAssertIntEquals( tc, 3, val );
    CuAssertTrue( tc, ! lcn_fs_field_is_modified( field_a ));
    CuAssertTrue( tc, ! lcn_fs_field_is_modified( field_b ));

    LCN_TEST( lcn_fs_field_set_int_value( m_field, 5, 2 ));
    CuAssertTrue( tc, lcn_fs_field_is_modified( field_a ));
    CuAssertTrue( tc, lcn_fs_field_is_modified( m_field ));

    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 2 ));
    CuAssertIntEquals( tc, 5, val );

    LCN_TEST( lcn_fs_field_int_value( field_a, &val, 2 ));
    CuAssertIntEquals( tc, 5, val );

    apr_pool_destroy( pool );
}


static void
test_multi_fs_fields_incomplete_first( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field_a, *field_b;
    unsigned int val = 17;
    lcn_fs_field_t *m_field;
    lcn_list_t *field_list;
    unsigned int i;
    unsigned int offset[10];

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field_a,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_a, 3 ));

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_b, 3 ));

    /* test offset consistency */

    offset[0] =  1;
    offset[1] = 20;
    offset[2] = 25;

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, NULL ));
    CuAssertIntEquals( tc, 1, lcn_list_size( field_list ));

    LCN_TEST( lcn_list_add( field_list, NULL ));
    LCN_TEST( lcn_list_add( field_list, NULL ));

    /* first offset must be 0 */

    CuAssertIntEquals( tc, LCN_ERR_FS_FIELD_INCONSISTENT_OFFSET,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    offset[0] = 0;

    CuAssertIntEquals( tc, 3, lcn_list_size( field_list ));

    CuAssertIntEquals( tc, LCN_ERR_INVALID_ARGUMENT,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, NULL ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    CuAssertIntEquals( tc, 5, lcn_fs_field_docs_count( field_b ));
    CuAssertIntEquals( tc, 0, offset[0] );

    LCN_TEST( lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* test getting default value */

    for( i = 0; i < 30; i++ )
    {
        LCN_TEST( lcn_fs_field_int_value( m_field, &val, 0 ));
        CuAssertIntEquals( tc, 3, val );
    }

    /* test modified flag */

    CuAssertTrue( tc, ! lcn_fs_field_is_modified( m_field ));
    CuAssertIntEquals( tc, LCN_ERR_UNSUPPORTED_OPERATION, lcn_fs_field_set_int_value( m_field, 5, 3 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 3 ));
    CuAssertIntEquals( tc, 3, val );

    CuAssertTrue( tc, ! lcn_fs_field_is_modified( m_field ));
    LCN_TEST( lcn_fs_field_int_value( field_a, &val, 3 ));
    CuAssertIntEquals( tc, 3, val );
    CuAssertTrue( tc, ! lcn_fs_field_is_modified( field_a ));
    CuAssertTrue( tc, ! lcn_fs_field_is_modified( field_b ));

    LCN_TEST( lcn_fs_field_set_int_value( m_field, 5, 22 ));
    CuAssertTrue( tc, lcn_fs_field_is_modified( field_a ));
    CuAssertTrue( tc, lcn_fs_field_is_modified( m_field ));

    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 22 ));
    CuAssertIntEquals( tc, 5, val );

    LCN_TEST( lcn_fs_field_int_value( field_a, &val, 2 ));
    CuAssertIntEquals( tc, 5, val );

    apr_pool_destroy( pool );
}

static void
test_multi_fs_fields_two_fields( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field_a, *field_b;
    unsigned int val = 17;
    lcn_fs_field_t *m_field;
    lcn_list_t *field_list;
    unsigned int i;
    unsigned int offset[10];

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field_a,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_a, 3 ));

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             5,  /* docs_count    */
                                             4,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_directory_fs_field_set_default_int_value( (lcn_directory_fs_field_t*) field_b, 3 ));

    /* test offset consistency */

    offset[0] = 1;
    offset[1] = 1;

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    /* first offset must be 0 */
    CuAssertIntEquals( tc, LCN_ERR_FS_FIELD_INCONSISTENT_OFFSET,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* second offset must be 5 */
    offset[0] = 0;
    CuAssertIntEquals( tc, LCN_ERR_FS_FIELD_INCONSISTENT_OFFSET,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* test getting default value */

    offset[1] = 5;
    LCN_TEST( lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool));

    for( i = 0; i < 10; i++ )
    {
        LCN_TEST( lcn_fs_field_int_value( m_field, &val, 0 ));
        CuAssertIntEquals( tc, 3, val );
    }

    /* test modified flag */

    CuAssertTrue( tc, ! lcn_fs_field_is_modified( m_field ));
    LCN_TEST( lcn_fs_field_set_int_value( field_a, 1, 0 ));
    CuAssertTrue( tc, lcn_fs_field_is_modified( field_a ));
    CuAssertTrue( tc, lcn_fs_field_is_modified( m_field ));

    /* test setting/getting value */

    LCN_TEST( lcn_fs_field_set_int_value( field_a, 0, 0 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 0 ));
    CuAssertIntEquals( tc, 0, val );

    LCN_TEST( lcn_fs_field_set_int_value( m_field, 1, 1 ));
    LCN_TEST( lcn_fs_field_int_value( field_a, &val, 1 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_set_int_value( m_field, 2, 2 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 2 ));
    CuAssertIntEquals( tc, 2, val );

    LCN_TEST( lcn_fs_field_set_int_value( m_field, 1, 5 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 5 ));
    CuAssertIntEquals( tc, 1, val );
    LCN_TEST( lcn_fs_field_int_value( field_b, &val, 0 ));
    CuAssertIntEquals( tc, 1, val );

    LCN_TEST( lcn_fs_field_set_int_value( m_field, 4, 5 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 5 ));
    CuAssertIntEquals( tc, 4, val );
    LCN_TEST( lcn_fs_field_int_value( field_b, &val, 0 ));
    CuAssertIntEquals( tc, 4, val );

    /* test implicit extension */

    LCN_TEST( lcn_fs_field_set_int_value( m_field, 2, 20 ));
    LCN_TEST( lcn_fs_field_int_value( m_field, &val, 20 ));
    CuAssertIntEquals( tc, 2, val );
    LCN_TEST( lcn_fs_field_int_value( field_b, &val, 15 ));
    CuAssertIntEquals( tc, 2, val );
    CuAssertIntEquals( tc, 21, lcn_fs_field_docs_count( m_field ));
    CuAssertIntEquals( tc, 16, lcn_fs_field_docs_count( field_b ));

    apr_pool_destroy( pool );
}


static void
test_multi_fs_fields_errorcheck( CuTest *tc )
{
    apr_pool_t *pool;
    lcn_fs_field_t *field_a, *field_b;
    lcn_fs_field_t *m_field;
    lcn_list_t *field_list;
    unsigned int offset[10];

    offset[0] = 0;
    offset[1] = 1;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_directory_fs_field_create( &field_a,
                                             "tfield",
                                             1,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));

    LCN_TEST( lcn_fs_field_set_default_int_value( (lcn_fs_field_t*) field_a, 3 ));

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfieldx",
                                             1,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));
    LCN_TEST( lcn_fs_field_set_default_int_value( (lcn_fs_field_t*) field_b, 3 ));

    /* add two fields with different names */

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    CuAssertIntEquals( tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* add two fields with different data size */

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             1,  /* docs_count    */
                                             3,  /* field_size    */
                                             NULL,
                                             pool ));
    LCN_TEST( lcn_fs_field_set_default_int_value( field_b, 3 ));

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    CuAssertIntEquals( tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* add two fields with different default value */

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             1,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));
    LCN_TEST( lcn_fs_field_set_default_int_value(field_b, 1 ));

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    CuAssertIntEquals( tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* add two fields with doc_count == 0 for some field */

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             0,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));
    LCN_TEST( lcn_fs_field_set_default_int_value( field_b, 1 ));

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    CuAssertIntEquals( tc, LCN_ERR_FIELD_INFO_INCONSISTENT_DEFINITION,
                       lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    /* add two fields with consistent definition */

    LCN_TEST( lcn_directory_fs_field_create( &field_b,
                                             "tfield",
                                             1,  /* docs_count    */
                                             2,  /* field_size    */
                                             NULL,
                                             pool ));
    LCN_TEST( lcn_fs_field_set_default_int_value(field_b, 3 ));

    LCN_TEST( lcn_list_create( &field_list, 10, pool ));
    LCN_TEST( lcn_list_add( field_list, field_a ));
    LCN_TEST( lcn_list_add( field_list, field_b ));

    CuAssertIntEquals( tc, 1, lcn_fs_field_docs_count( field_a ));
    CuAssertIntEquals( tc, 1, lcn_fs_field_docs_count( field_b ));
    CuAssertIntEquals( tc, 0, offset[0] );
    CuAssertIntEquals( tc, 1, offset[1] );

    LCN_TEST( lcn_multi_fs_field_create_by_subfields( &m_field, field_list, offset, pool ));

    apr_pool_destroy( pool );
}



CuSuite* make_field_fixed_size_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_create_fixed_size_by_ints );
    SUITE_ADD_TEST( s, test_fs_field_size_1 );
    SUITE_ADD_TEST( s, test_fs_field_size_16 );
    SUITE_ADD_TEST( s, test_fs_field_size_16d );
    SUITE_ADD_TEST( s, test_fs_field_size_1_default );
    SUITE_ADD_TEST( s, test_fs_field_size_2 );
    SUITE_ADD_TEST( s, test_fs_field_size_2d );
    SUITE_ADD_TEST( s, test_fs_field_size_8 );
    SUITE_ADD_TEST( s, test_fs_field_size_8d );
    SUITE_ADD_TEST( s, test_fs_field_size_char );
    SUITE_ADD_TEST( s, test_fs_field_size_int );
    SUITE_ADD_TEST( s, test_fs_field_size_intd );
    SUITE_ADD_TEST( s, test_indexing );
    SUITE_ADD_TEST( s, test_indexing_errors );
    SUITE_ADD_TEST( s, test_indexing_errors_default_val );
    SUITE_ADD_TEST( s, test_multi_fs_field_create );
    SUITE_ADD_TEST( s, test_multi_fs_fields_errorcheck );
    SUITE_ADD_TEST( s, test_multi_fs_fields_incomplete_first );
    SUITE_ADD_TEST( s, test_multi_fs_fields_incomplete_second );
    SUITE_ADD_TEST( s, test_multi_fs_fields_incomplete_third );
    SUITE_ADD_TEST( s, test_multi_fs_fields_two_fields );
    SUITE_ADD_TEST( s, test_multi_indexes );
    SUITE_ADD_TEST( s, test_sorting_by_fixed_size_field );

    /* merging fields */
    SUITE_ADD_TEST( s, test_adding_two_equal_fields );

    return s;
}
