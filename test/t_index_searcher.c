#include "test_all.h"
#include "lcn_analysis.h"
#include "lcn_bitvector.h"

char *test_index_name;

static void
test_index_searcher( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    unsigned int doc_freq;
    lcn_term_t* term;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_term_create( &term, "text", "a", 1, pool ) );
    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    CuAssertIntEquals( tc, 331,  lcn_searcher_max_doc( searcher ) );

    LCN_TEST( lcn_searcher_doc_freq( searcher, term, &doc_freq ) );

    CuAssertIntEquals( tc, 47, doc_freq );

    apr_pool_destroy( pool );
}


static void
test_field_exists( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_bool_t field_exists;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_searcher_field_exists( searcher, "text", &field_exists ));
    CuAssertTrue( tc, field_exists );

    LCN_TEST( lcn_searcher_field_exists( searcher, "no_text", &field_exists ));
    CuAssertTrue( tc, ! field_exists );

    apr_pool_destroy( pool );
}

static void
test_sort_multi_fields_impl( CuTest* tc,
                             lcn_bool_t reverse_year,
                             lcn_bool_t reverse_month,
                             lcn_bool_t sort_by_day,
                             unsigned int default_year )
{
    apr_pool_t *pool;
    lcn_searcher_t *searcher;
    lcn_query_t *query;
    lcn_hits_t *hits;
    lcn_list_t *sort_fields, *bv_list;
    unsigned int i, pyear, pmonth, pday, pid;
    lcn_bitvector_t *bv;
    unsigned int bitvector_size = 0;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "sort_test",
                                                 pool ));

    /* create a counting filter for "the" */

    {
        lcn_query_t *t;

        LCN_TEST( lcn_term_query_create_by_chars( &t, "year", "2000", pool ) );
        LCN_TEST( lcn_searcher_search( searcher, &hits, t, NULL, pool ));

        LCN_TEST( lcn_query_bitvector_create( &bv, t, searcher, NULL, pool ) );
        CuAssertIntEquals( tc, lcn_bitvector_count(bv), lcn_hits_length( hits ));

        bitvector_size = lcn_hits_length( hits );

        LCN_TEST( lcn_list_create( &bv_list, 2, pool ));
        LCN_TEST( lcn_list_add( bv_list, bv ));
    }

    LCN_TEST( lcn_match_all_docs_query_create( &query, pool ));
    LCN_TEST( lcn_list_create( &sort_fields, 2, pool ));

    {
        lcn_sort_field_t *sfield;

        LCN_TEST( lcn_sort_field_create( &sfield,
                                         "year",
                                         LCN_SORT_FIELD_INT,
                                         reverse_year,
                                         pool ) );

        if ( 0 != default_year )
        {
            LCN_TEST( lcn_sort_field_set_default_int_value( sfield, default_year ));
        }

        LCN_TEST( lcn_list_add( sort_fields, sfield ) );

        LCN_TEST( lcn_sort_field_create( &sfield,
                                         "month",
                                         LCN_SORT_FIELD_INT,
                                         reverse_month,
                                         pool ) );
        LCN_TEST( lcn_list_add( sort_fields, sfield ) );

        if ( sort_by_day )
        {
            LCN_TEST( lcn_sort_field_create( &sfield,
                                             "day",
                                             LCN_SORT_FIELD_INT,
                                             LCN_FALSE,
                                             pool ) );
            LCN_TEST( lcn_list_add( sort_fields, sfield ) );
        }
    }

    LCN_TEST( lcn_searcher_set_counting_bitvectors( searcher, bv_list ));
    LCN_TEST( lcn_searcher_search_sort( searcher, &hits, query, NULL, sort_fields, pool ) );

    pyear = reverse_year ? 100000000 : 0 ;
    pmonth = reverse_year ? 10000: 0;
    pday = 0;
    pid = 0;

    CuAssertIntEquals( tc, bitvector_size, lcn_hits_bitvector_count( hits, 0 ) );

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t* doc;
        char *str;
        unsigned int year, month, day, id;

        LCN_TEST( lcn_hits_doc( hits, &doc, i, pool ) );

        if ( lcn_document_field_exists( doc, "year" ))
        {
            LCN_TEST( lcn_document_get( doc, &str, "year", pool ));
            year = atoi( str );
        }
        else
        {
            year = default_year;
        }

        if ( lcn_document_field_exists( doc, "month" ))
        {
            LCN_TEST( lcn_document_get( doc, &str, "month", pool ));
            month = atoi( str );
        }
        else
        {
            month = 0;
        }

        if ( lcn_document_field_exists( doc, "day" ))
        {
            LCN_TEST( lcn_document_get( doc, &str, "day", pool ));
            day = atoi( str );
        }
        else
        {
            day = 0;
        }

        id = lcn_document_id( doc );

        if ( i < 2 && ! reverse_year )
        {
            if ( default_year < 2000 )
            {
                CuAssertIntEquals( tc, default_year, year );
                CuAssertIntEquals( tc, 0, month);
                CuAssertIntEquals( tc, 0, day  );
            }
        }

        CuAssertTrue( tc, reverse_year ? pyear >= year : pyear <= year );

        if ( pyear == year )
        {
            CuAssertTrue( tc, reverse_month ? pmonth >= month : pmonth <= month );
        }

        if ( pyear == year && pmonth == month )
        {
            if ( sort_by_day )
            {
                CuAssertTrue( tc, pday <= day );
            }
            else
            {
                CuAssertTrue( tc, pid < id );
            }
        }

        pyear = year; pmonth = month;  pid = id; pday = day;
    }

    apr_pool_destroy( pool );
}

static void
test_sort_multi_fields_reverse_month( CuTest *tc,
                                      lcn_bool_t reverse_month,
                                      lcn_bool_t sort_by_day,
                                      unsigned int default_year )
{
    test_sort_multi_fields_impl( tc,
                                 LCN_FALSE, /* reverse year  */
                                 reverse_month,
                                 sort_by_day,
                                 default_year );

    test_sort_multi_fields_impl( tc,
                                 LCN_TRUE,  /* reverse year  */
                                 reverse_month,
                                 sort_by_day,
                                 default_year );
}

static void
test_sort_multi_fields_sort_by_day( CuTest* tc, lcn_bool_t sort_by_day,
                                    unsigned int default_year )
{
    test_sort_multi_fields_reverse_month( tc, LCN_TRUE,  sort_by_day, default_year );
    test_sort_multi_fields_reverse_month( tc, LCN_FALSE, sort_by_day, default_year );
}

static void
test_sort_multi_fields_default_year( CuTest* tc, unsigned int default_year )
{
    test_sort_multi_fields_sort_by_day( tc, LCN_FALSE, default_year );
    test_sort_multi_fields_sort_by_day( tc, LCN_TRUE, default_year );
}

static void
test_sort_multi_fields( CuTest* tc )
{
    test_sort_multi_fields_default_year( tc, 0 );
    test_sort_multi_fields_default_year( tc, 1000000 );
}

static void
test_sort_top_docs_impl( CuTest* tc, lcn_bool_t reverse )
{
    apr_pool_t *pool;
    lcn_searcher_t *searcher;
    lcn_query_t *query;
    lcn_hits_t *hits;
    lcn_list_t *sort_fields;
    unsigned int i, prev, next;

    {
        apr_hash_t* map;

        apr_pool_create( &pool, main_pool );
        delete_files( tc, "sort_test" );
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "sort_test",
                                                         "sort.txt",
                                                         map,
                                                         LCN_TRUE, /* optimize */
                                                         pool ));
        apr_pool_destroy( pool );
    }

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "sort_test",
                                                 pool ));

    LCN_TEST( lcn_match_all_docs_query_create( &query, pool ));
    LCN_TEST( lcn_list_create( &sort_fields, 1, pool ));

    {
        lcn_sort_field_t *sfield;

        LCN_TEST( lcn_sort_field_create( &sfield,
                                         "int_field",
                                         LCN_SORT_FIELD_INT,
                                         reverse,
                                         pool ) );
        LCN_TEST( lcn_list_add( sort_fields, sfield ) );
    }

    LCN_TEST( lcn_searcher_search_sort( searcher, &hits, query, NULL, sort_fields, pool ) );

    prev = reverse ? 100000 : 0 ;

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t* doc;
        char* id;
        apr_status_t s;

        LCN_TEST( lcn_hits_doc( hits, &doc, i, pool ) );
        s = lcn_document_get( doc, &id, "int_field", pool );

        if ( APR_SUCCESS ==  s )
        {
            next = atoi( id );
        }
        else
        {
            next = 0;
        }

        CuAssertTrue( tc, (reverse ? next <= prev : next >= prev ) );
        prev = next;
    }

    apr_pool_destroy( pool );
}


static void
test_sort_top_docs( CuTest* tc )
{
    test_sort_top_docs_impl( tc, LCN_FALSE );
    test_sort_top_docs_impl( tc, LCN_TRUE );
}

static void
setup( CuTest* tc )
{
    test_index_name = "test_index_2";
}

static void
setup_multi( CuTest* tc )
{
    test_index_name = "test_index_1";
}

CuSuite*
make_index_searcher_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, setup );
    SUITE_ADD_TEST( s, test_index_searcher );
    SUITE_ADD_TEST( s, test_field_exists );
    SUITE_ADD_TEST( s, test_sort_top_docs );
    SUITE_ADD_TEST( s, test_sort_multi_fields );

    SUITE_ADD_TEST( s, setup_multi );
    SUITE_ADD_TEST( s, test_index_searcher );
    SUITE_ADD_TEST( s, test_field_exists );

    return s;
}
