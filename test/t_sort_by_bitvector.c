#include "test_all.h"
#include "lcn_search.h"
#include "lcn_bitvector.h"


#define CHECK_HIT( HITS, NTH, FIELD, TEXT )                     \
{                                                               \
    lcn_document_t* doc;                                        \
    char* content;                                              \
                                                                \
    LCN_TEST( lcn_hits_doc( HITS, &doc, NTH, pool ) );          \
    LCN_TEST( lcn_document_get( doc, &content, FIELD, pool ) ); \
    CuAssertStrEquals( tc, TEXT, content );                     \
}

static void
test_sort_by_bitvector_with_hc( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_bitvector_t* bitvector;
    lcn_list_t *bitvector_list;
    lcn_index_reader_t* index_reader;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_2",
                                                 pool ) );

    lcn_searcher_set_hit_collector_initial_size( searcher, 1 );

    index_reader = lcn_index_searcher_reader_get( searcher );

    LCN_TEST( lcn_index_reader_null_bitvector( index_reader, &bitvector, pool ));

    LCN_TEST( lcn_bitvector_set_bit( bitvector, 13 ));

    LCN_TEST( lcn_list_create( &bitvector_list, 1, pool ));
    LCN_TEST( lcn_list_add( bitvector_list, bitvector ));

    LCN_TEST( lcn_searcher_order_by( searcher, LCN_ORDER_BY_NATURAL ));

    LCN_TEST( lcn_searcher_order_by_bitvectors( searcher, bitvector_list ));

    LCN_TEST( lcn_term_query_create_by_chars( &query, "text", "a", pool ));

    LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );

    CuAssertIntEquals( tc, 47, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "id", "KW13" );
    CHECK_HIT( hits, 1, "id", "KW5" );
    CHECK_HIT( hits, 2, "id", "KW6" );
    CHECK_HIT( hits, 3, "id", "KW8" );

    lcn_index_searcher_close( searcher );

    apr_pool_destroy( pool );
}

static void
test_sort_by_bitvector( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* b_query1;
    lcn_query_t* t_query1;
    lcn_query_t* t_query2;
    lcn_hits_t* hits;
    lcn_bitvector_t* bitvector1, *bitvector2;
    lcn_list_t *bitvector_list, *counts_list;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_2",
                                                 pool ) );

    LCN_TEST( lcn_term_query_create_by_chars( &t_query1, "text", "better", pool ));
    LCN_TEST( lcn_query_bitvector_create( &bitvector1, t_query1, searcher, NULL, pool ));

    LCN_TEST( lcn_term_query_create_by_chars( &t_query2, "text", "created", pool ));
    LCN_TEST( lcn_query_bitvector_create( &bitvector2, t_query2, searcher, NULL, pool ));

    /* make bitvector for 'better or created' */
    LCN_TEST( lcn_boolean_query_create( &b_query1, pool ) );
    LCN_TEST( lcn_boolean_query_add_term( b_query1, "text", "better",
                                          LCN_BOOLEAN_CLAUSE_SHOULD  ) );
    LCN_TEST( lcn_boolean_query_add_term( b_query1, "text", "created",
                                          LCN_BOOLEAN_CLAUSE_SHOULD  ) );

    LCN_TEST( lcn_searcher_search( searcher, &hits, b_query1, NULL, pool ) );

    CuAssertIntEquals( tc, 5, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "id", "KW7" );            /* created */
    CHECK_HIT( hits, 1, "id", "KW13" );           /* better  */
    CHECK_HIT( hits, 2, "id", "KW317" );          /* better  */
    CHECK_HIT( hits, 3, "id", "KW116" );          /* created */
    CHECK_HIT( hits, 4, "id", "KW119" );          /* created */


    /* now search with bitvectors */

    LCN_TEST( lcn_list_create( &bitvector_list, 2, pool ));
    LCN_TEST( lcn_list_add( bitvector_list, bitvector2 ));
    LCN_TEST( lcn_list_add( bitvector_list, bitvector1 ));

    LCN_TEST( lcn_searcher_order_by_bitvectors( searcher, bitvector_list ));
    LCN_TEST( lcn_searcher_set_counting_bitvectors( searcher, bitvector_list ));

    LCN_TEST( lcn_searcher_search( searcher, &hits, b_query1, NULL, pool ) );

    CuAssertIntEquals( tc, 5, lcn_hits_length( hits ) );

    CuAssertIntEquals( tc, 3, lcn_hits_bitvector_count( hits, 0 ) );
    CuAssertIntEquals( tc, 2, lcn_hits_bitvector_count( hits, 1 ) );
    CuAssertIntEquals( tc, 0, lcn_hits_bitvector_count( hits, 2 ) );

    CHECK_HIT( hits, 0, "id", "KW7" );            /* created */
    CHECK_HIT( hits, 1, "id", "KW116" );          /* created */
    CHECK_HIT( hits, 2, "id", "KW119" );          /* created */
    CHECK_HIT( hits, 3, "id", "KW13" );           /* better  */
    CHECK_HIT( hits, 4, "id", "KW317" );          /* better  */

    /* now search with bitvectors with changed ordering */

    LCN_TEST( lcn_list_create( &bitvector_list, 2, pool ));
    LCN_TEST( lcn_list_add( bitvector_list, bitvector1 ));
    LCN_TEST( lcn_list_add( bitvector_list, bitvector2 ));

    LCN_TEST( lcn_list_create( &counts_list, 2, pool ));
    LCN_TEST( lcn_list_add( counts_list, bitvector1 ));

    LCN_TEST( lcn_searcher_set_counting_bitvectors( searcher, counts_list ));
    LCN_TEST( lcn_searcher_order_by_bitvectors( searcher, bitvector_list ));
    LCN_TEST( lcn_searcher_search( searcher, &hits, b_query1, NULL, pool ) );


    CuAssertIntEquals( tc, 5, lcn_hits_length( hits ) );

    CuAssertIntEquals( tc, 2, lcn_hits_bitvector_count( hits, 0 ) );
    CuAssertIntEquals( tc, 3, lcn_hits_bitvector_count( hits, 1 ) );
    CuAssertIntEquals( tc, 0, lcn_hits_bitvector_count( hits, 2 ) );

    CHECK_HIT( hits, 0, "id", "KW13" );           /* better  */
    CHECK_HIT( hits, 1, "id", "KW317" );          /* better  */
    CHECK_HIT( hits, 2, "id", "KW7" );            /* created */
    CHECK_HIT( hits, 3, "id", "KW116" );          /* created */
    CHECK_HIT( hits, 4, "id", "KW119" );          /* created */

    lcn_index_searcher_close( searcher );

    apr_pool_destroy( pool );
}


CuSuite*
make_sort_by_bitvector_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_sort_by_bitvector );
    SUITE_ADD_TEST( s, test_sort_by_bitvector_with_hc );

    return s;
}
