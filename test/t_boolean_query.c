#include "test_all.h"
#include "lcn_search.h"
#include "lcn_analysis.h"
#include "search/query.h"

static char *test_index_name;

#define CHECK_DOC( I, ID );                                             \
    {                                                                   \
        lcn_document_t* doc;                                            \
        char* text;                                                     \
        apr_pool_t* my_cp;                                              \
                                                                        \
        LCN_TEST( apr_pool_create( &my_cp, pool ) );                    \
        LCN_TEST( lcn_hits_doc( hits, &doc, I, my_cp ) );               \
        /*LCN_TEST( lcn_document_get( doc, &text, "text", my_cp ) );    \
          printf( "<%s>\n", text );*/                                   \
        LCN_TEST( lcn_document_get( doc, &text,"id",  my_cp ) );        \
        CuAssertStrEquals( tc, ID, text );                              \
    }

#define ADD_TERM_TO_LIST( LIST, FIELD, TEXT )                     \
{                                                                 \
    lcn_term_t* new_term;                                         \
                                                                  \
    lcn_term_create( &new_term, FIELD, TEXT, LCN_TRUE, pool );    \
    lcn_list_add( LIST, new_term );                               \
}

static void
test_failing_disjunction(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_directory_t *dir;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_analyzer_t *analyzer;
    lcn_searcher_t *searcher;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );


    /* create an index in RAM */

    LCN_TEST( lcn_ram_directory_create( &dir, pool ));
    LCN_TEST( lcn_index_writer_create_by_directory( &index_writer, dir, LCN_TRUE, pool ));

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "test",
                                LCN_FIELD_INDEXED |
                                LCN_FIELD_TOKENIZED,
                                LCN_FIELD_VALUE_COPY, pool ) );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );
    lcn_field_set_analyzer( field, analyzer );
    LCN_TEST( lcn_document_add_field( document, field, pool ) );
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    LCN_TEST( lcn_index_writer_close( index_writer ) );


    /* now let's search */
    LCN_TEST( lcn_index_searcher_create_by_directory( &searcher, dir, pool ));

    {
        lcn_query_t *tq;
        lcn_hits_t *hits;

        LCN_TEST( lcn_term_query_create_by_chars( &tq, "text", "test", pool ));
        LCN_TEST( lcn_searcher_search( searcher, &hits, tq, NULL, pool ) );

        CuAssertIntEquals( tc, 1, lcn_hits_length( hits ));

        LCN_TEST( lcn_term_query_create_by_chars( &tq, "text", "tests", pool ));
        LCN_TEST( lcn_searcher_search( searcher, &hits, tq, NULL, pool ) );

        CuAssertIntEquals( tc, 0, lcn_hits_length( hits ));
    }

    {
        lcn_query_t *bq;
        lcn_hits_t *hits;

        LCN_TEST( lcn_boolean_query_create( &bq, pool ));
        LCN_TEST( lcn_boolean_query_add_term( bq, "text", "test", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bq, "text", "tests", LCN_BOOLEAN_CLAUSE_SHOULD ));

        LCN_TEST( lcn_searcher_search( searcher, &hits, bq, NULL, pool ) );

        CuAssertIntEquals( tc, 1, lcn_hits_length( hits ));
    }

    apr_pool_destroy( pool );
}

static void
test_multi_phrase_in_boolean( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* a_query;
    lcn_query_t* query;
    lcn_list_t* terms_1, *terms_2;

    char* query_str;

    apr_pool_create( &pool, main_pool );
    lcn_list_create( &terms_1, 20, pool );
    lcn_list_create( &terms_2, 20, pool );

    ADD_TERM_TO_LIST( terms_1, "text", "is" );
    ADD_TERM_TO_LIST( terms_1, "text", "can" );
    ADD_TERM_TO_LIST( terms_1, "text", "must" );
    ADD_TERM_TO_LIST( terms_1, "text", "just" );
    ADD_TERM_TO_LIST( terms_1, "text", "and" );
    ADD_TERM_TO_LIST( terms_1, "text", "or" );
    ADD_TERM_TO_LIST( terms_1, "text", "get" );
    ADD_TERM_TO_LIST( terms_1, "text", "set" );
    ADD_TERM_TO_LIST( terms_1, "text", "subclass" );
    ADD_TERM_TO_LIST( terms_1, "text", "by" );

    ADD_TERM_TO_LIST( terms_2, "text", "is" );
    ADD_TERM_TO_LIST( terms_2, "text", "can" );
    ADD_TERM_TO_LIST( terms_2, "text", "must" );
    ADD_TERM_TO_LIST( terms_2, "text", "just" );
    ADD_TERM_TO_LIST( terms_2, "text", "and" );
    ADD_TERM_TO_LIST( terms_2, "text", "or" );
    ADD_TERM_TO_LIST( terms_2, "text", "get" );
    ADD_TERM_TO_LIST( terms_2, "text", "set" );
    ADD_TERM_TO_LIST( terms_2, "text", "subclass" );
    ADD_TERM_TO_LIST( terms_2, "text", "by" );


    lcn_multi_phrase_query_create( &a_query, pool );
    lcn_multi_phrase_query_add_terms( a_query, terms_1 );
    lcn_multi_phrase_query_add_terms( a_query, terms_2 );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, pool ) );

    lcn_boolean_query_add( query, a_query, LCN_BOOLEAN_CLAUSE_SHOULD );

    lcn_query_to_string( query, &query_str, "x", pool );
    fprintf( stderr, "%s\n", query_str );
    apr_pool_destroy( pool );
}

static void
test_prefix_combo( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* a_query, *b_query;
    lcn_query_t* query;
    char* query_str;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_prefix_query_create_by_chars( &a_query,
                                              "text", "a", pool ) );
    LCN_TEST( lcn_prefix_query_create_by_chars( &b_query,
                                              "text", "b", pool ) );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, pool ) );

    lcn_boolean_query_add( query, a_query, LCN_BOOLEAN_CLAUSE_SHOULD );
    lcn_boolean_query_add( query, b_query, LCN_BOOLEAN_CLAUSE_SHOULD );

    lcn_query_to_string( query, &query_str, "x", pool );
    CuAssertStrEquals( tc, "text:a* text:b*", query_str );
    apr_pool_destroy( pool );
}

static void
test_req_prohib( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* is_query, *can_query;
    lcn_query_t* b_query;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_term_query_create_by_chars( &is_query,
                                              "text", "is", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &can_query,
                                              "text", "can", pool ) );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &b_query, pool ) );

    LCN_TEST( lcn_boolean_query_add( b_query, is_query,
                                     LCN_BOOLEAN_CLAUSE_MUST  ) );

    LCN_TEST( lcn_boolean_query_add( b_query, can_query,
                                     LCN_BOOLEAN_CLAUSE_MUST_NOT  ) );

    lcn_searcher_search( searcher, &hits, b_query, NULL, pool );

    CuAssertIntEquals( tc, 26, lcn_hits_length( hits ) );
#if 0
    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t* doc;
        char* text;
        lcn_hits_doc( hits, &doc, i, pool );

        lcn_document_get( doc, &text, "text", pool );
        fprintf( stderr, "%u) %s\n---------\n\n", i, text );
    }
#endif
    apr_pool_destroy( pool );
}

static void
test_opt_prohib( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    char* query_str;
    lcn_query_t* query, *tquery, *tquery2, *tquery3;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_term_query_create_by_chars( &tquery, "text", "can", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tquery2,"text", "must", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tquery3,"text", "it", pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, cp ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery,
                                     LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery2,
                                     LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery3,
                                     LCN_BOOLEAN_CLAUSE_MUST_NOT ) );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc, "text:can text:must -text:it", query_str );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 8, lcn_hits_length( hits ) );

    CHECK_DOC( 0, "KW265" );
    CHECK_DOC( 1, "KW21" );
    CHECK_DOC( 2, "KW70" );
    CHECK_DOC( 3, "KW105" );
    CHECK_DOC( 4, "KW172" );
    CHECK_DOC( 5, "KW299" );
    CHECK_DOC( 6, "KW45" );
    CHECK_DOC( 7, "KW130" );

    apr_pool_destroy( pool );
}

static void
test_combo_booleans( CuTest* tc )
{
    apr_status_t s;
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    char* query_str;
    lcn_query_t* bool1, *bool2, *tq, *combo_bool, *combo_bool2;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &bool1, pool ) );
    LCN_TEST( lcn_boolean_query_create( &bool2, pool ) );
    LCN_TEST( lcn_boolean_query_create( &combo_bool, pool ) );
    LCN_TEST( lcn_boolean_query_create( &combo_bool2, pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tq, "id", "KW45", pool ) );

    do
    {
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW21", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW70", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW105", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW172", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW299", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW299", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW45", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool1, "id", "KW130", LCN_BOOLEAN_CLAUSE_SHOULD ));

        LCN_TEST( lcn_boolean_query_add_term( bool2, "id", "KW45", LCN_BOOLEAN_CLAUSE_SHOULD ));
        LCN_TEST( lcn_boolean_query_add_term( bool2, "id", "KW130", LCN_BOOLEAN_CLAUSE_SHOULD ));

        LCN_TEST( lcn_boolean_query_add( combo_bool, bool1, LCN_BOOLEAN_CLAUSE_SHOULD ) );
        LCN_TEST( lcn_boolean_query_add( combo_bool, bool2, LCN_BOOLEAN_CLAUSE_MUST_NOT ) );

        LCN_TEST( lcn_query_to_string( combo_bool, &query_str, "", pool ) );

        CuAssertStrEquals( tc, "(id:KW21 id:KW70 id:KW105 id:KW172 "
                           "id:KW299 id:KW299 id:KW45 id:KW130) "
                           "-(id:KW45 id:KW130)", query_str );

        LCNCE( lcn_searcher_search( searcher, &hits, combo_bool, NULL, pool ) );
        CuAssertIntEquals( tc, 5, lcn_hits_length( hits ) );

        CHECK_DOC( 0, "KW299" );
        CHECK_DOC( 1, "KW21" );
        CHECK_DOC( 2, "KW70" );
        CHECK_DOC( 3, "KW105" );
        CHECK_DOC( 4, "KW172" );

        LCN_TEST( lcn_boolean_query_add( combo_bool2, combo_bool,
                                         LCN_BOOLEAN_CLAUSE_SHOULD ) );
        LCN_TEST( lcn_boolean_query_add( combo_bool2, tq,
                                         LCN_BOOLEAN_CLAUSE_SHOULD ) );

        LCN_TEST( lcn_query_to_string( combo_bool2, &query_str, "", pool ) );

        CuAssertStrEquals( tc,
                           "((id:KW21 id:KW70 id:KW105 "
                           "id:KW172 id:KW299 id:KW299 id:KW45 "
                           "id:KW130) -(id:KW45 id:KW130)) id:KW45",
                           query_str );

        LCNCE( lcn_searcher_search( searcher, &hits, combo_bool2, NULL, pool ) );
        CuAssertIntEquals( tc, 6, lcn_hits_length( hits ) );

        CHECK_DOC( 0, "KW45" );
        CHECK_DOC( 1, "KW299" );
        CHECK_DOC( 2, "KW21" );
        CHECK_DOC( 3, "KW70" );
        CHECK_DOC( 4, "KW105" );
        CHECK_DOC( 5, "KW172" );


    }
    while( FALSE );

    apr_pool_destroy( pool );
}

/**
 * This tests sorting behaviour, if the result-sets
 * of differently boosted clauses is not disjunct
 */

static void
test_boost_sorting( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    lcn_query_t* query, *bq1, *bq2;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );



    LCN_TEST( lcn_boolean_query_create( &query, pool ) );
    LCN_TEST( lcn_boolean_query_create( &bq1, pool ) );
    LCN_TEST( lcn_boolean_query_create( &bq2, pool ) );


    LCN_TEST( lcn_boolean_query_add_term( bq1, "id", "KW1",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq1, "id", "KW2",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq1, "id", "KW3",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq1, "id", "KW4",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq1, "id", "KW5",  LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_boolean_query_add_term( bq2, "id", "KW6",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq2, "id", "KW7",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq2, "id", "KW2",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq2, "id", "KW3",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq2, "id", "KW8",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq2, "id", "KW9",  LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq2, "id", "KW10", LCN_BOOLEAN_CLAUSE_SHOULD ) );

    lcn_query_boost_set( bq2, 30 );
    LCN_TEST( lcn_boolean_query_add( query, bq1, LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add( query, bq2, LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );
    CuAssertIntEquals( tc, 10, lcn_hits_length( hits ) );

    CHECK_DOC( 0, "KW2" );
    CHECK_DOC( 1, "KW3" );
    CHECK_DOC( 2, "KW6" );
    CHECK_DOC( 3, "KW7" );
    CHECK_DOC( 4, "KW8" );
    CHECK_DOC( 5, "KW9" );
    CHECK_DOC( 6, "KW10" );
    CHECK_DOC( 7, "KW1" );
    CHECK_DOC( 8, "KW4" );
    CHECK_DOC( 9, "KW5" );

    LCN_TEST( lcn_index_searcher_close( searcher ) );

    apr_pool_destroy( pool );
}

static void
test_boolean_query( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    char* query_str;
    lcn_query_t* query, *tquery, *tquery2, *tquery3, *clone, *empty_query;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_term_query_create_by_chars( &tquery, "text", "it", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &tquery2,"text", "the", pool ) );

    CuAssertIntEquals( tc,
                       LCN_ERR_INVALID_ARGUMENT,
                       lcn_boolean_query_add( tquery,
                                              tquery2,
                                              LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_term_query_create_by_chars( &tquery3,"text", "is", pool ) );
    LCN_TEST( lcn_boolean_query_create( &query, cp ) );
    LCN_TEST( lcn_boolean_query_create( &empty_query, pool ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery,
                                     LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery2,
                                     LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_boolean_query_add( query,
                                     tquery3,
                                     LCN_BOOLEAN_CLAUSE_MUST_NOT ) );
    LCN_TEST( lcn_boolean_query_add( query,
                                     empty_query,
                                     LCN_BOOLEAN_CLAUSE_SHOULD ) );
    lcn_query_set_name( query, "a named query" );
    LCN_TEST( lcn_query_clone( query, &clone, pool ) );

    CuAssertStrEquals( tc, "a named query", lcn_query_name( clone ));

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc, "+text:it +text:the -text:is ()", query_str );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 4, lcn_hits_length( hits ) );

    CHECK_DOC( 0, "KW324" );
    CHECK_DOC( 1, "KW97" );
    CHECK_DOC( 2, "KW69" );
    CHECK_DOC( 3, "KW157" );

    apr_pool_destroy( pool );
}

static void
test_failing_query( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    lcn_query_t* query;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, cp ) );

    LCN_TEST( lcn_boolean_query_add_term( query, "text", "zzzzzzzzzzzzz",
                                          LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST_STATUS( lcn_searcher_search( searcher,
                                          &hits,
                                          query,
                                          NULL,
                                          pool ),  APR_SUCCESS );
    CuAssertIntEquals( tc, 0, lcn_hits_length( hits ) );


    apr_pool_destroy( pool );

}

static void
test_clone( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* mpq, *tq, *pq, *bq, *bq1, *bq2, *clone;
    lcn_term_t* term;
    char* q_str, *clone_q_str;

    apr_pool_create( &pool, main_pool );
    LCN_TEST( lcn_boolean_query_create( &bq, pool ) );

    LCN_TEST( lcn_multi_phrase_query_create( &mpq, pool ) );
    LCN_TEST( lcn_term_create( &term, "text", "so", LCN_TRUE, pool ) );
    LCN_TEST( lcn_multi_phrase_query_add_term( mpq, term ) );
    LCN_TEST( lcn_term_create( &term, "text", "that", LCN_TRUE, pool ) );
    LCN_TEST( lcn_multi_phrase_query_add_term( mpq, term ) );
    LCN_TEST( lcn_term_create( &term, "text", "it", LCN_TRUE, pool ) );
    LCN_TEST( lcn_multi_phrase_query_add_term( mpq, term ) );
    LCN_TEST( lcn_term_create( &term, "text", "can", LCN_TRUE, pool ) );
    LCN_TEST( lcn_multi_phrase_query_add_term( mpq, term ) );

    LCN_TEST( lcn_term_query_create_by_chars( &tq, "text", "can", pool ) );
    LCN_TEST( lcn_prefix_query_create_by_chars( &pq, "text", "id", pool ) );

    LCN_TEST( lcn_boolean_query_create( &bq1, pool ) );
    LCN_TEST( lcn_boolean_query_add_term( bq1, "text", "is",
                                          LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq1, "text", "must",
                                          LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_boolean_query_create( &bq2, pool ) );
    LCN_TEST( lcn_boolean_query_add( bq2, tq, LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add( bq2, mpq, LCN_BOOLEAN_CLAUSE_MUST ) );
    LCN_TEST( lcn_boolean_query_add( bq2, pq, LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_boolean_query_add( bq, bq1, LCN_BOOLEAN_CLAUSE_MUST ) );
    LCN_TEST( lcn_boolean_query_add( bq, bq2, LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_query_to_string( bq, &q_str, "", pool ) );
    LCN_TEST( lcn_query_clone( bq, &clone, pool ) );
    LCN_TEST( lcn_query_to_string( clone, &clone_q_str, "", pool ) );

    CuAssertStrEquals( tc, q_str, clone_q_str );

    apr_pool_destroy( pool );
}

static void
test_first_record( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* query;
    lcn_query_t* term_query1;
    lcn_query_t* term_query2;
    lcn_hits_t* hits;
    lcn_searcher_t* searcher;

    apr_pool_create(&pool, main_pool);

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, pool ) );

    lcn_term_query_create_by_chars( &term_query1, "id", "KW0", pool );
    lcn_term_query_create_by_chars( &term_query2, "text", "lucene", pool );

    lcn_boolean_query_add( query, term_query1, LCN_BOOLEAN_CLAUSE_MUST) ;
    lcn_boolean_query_add( query, term_query2, LCN_BOOLEAN_CLAUSE_MUST) ;

    LCN_TEST_STATUS( lcn_searcher_search( searcher,
                                          &hits,
                                          query,
                                          NULL,
                                          pool ),  APR_SUCCESS );
    CuAssertIntEquals( tc, 1, lcn_hits_length( hits ) );

    apr_pool_destroy( pool );
}

static void
test_null_record( CuTest* tc )
{
    apr_pool_t* pool;

    lcn_query_t* boolquery1;
    lcn_query_t* boolquery2;
    lcn_query_t* boolquery3;

    lcn_query_t* term_query1;
    lcn_query_t* term_query2;
    lcn_query_t* term_query3;
    lcn_hits_t* hits;
    lcn_searcher_t* searcher;
    char* aus;
    apr_pool_create(&pool, main_pool);

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &boolquery1, pool ) );
    LCN_TEST( lcn_boolean_query_create( &boolquery2, pool ) );
    LCN_TEST( lcn_boolean_query_create( &boolquery3, pool ) );

    lcn_term_query_create_by_chars( &term_query1, "id", "0", pool );
    lcn_term_query_create_by_chars( &term_query2, "id", "KW0", pool );
    lcn_boolean_query_add( boolquery1, term_query1, LCN_BOOLEAN_CLAUSE_SHOULD) ;
    lcn_boolean_query_add( boolquery1, term_query2, LCN_BOOLEAN_CLAUSE_SHOULD) ;

    lcn_term_query_create_by_chars( &term_query3, "text", "lucene", pool );
    lcn_boolean_query_add( boolquery2, term_query3, LCN_BOOLEAN_CLAUSE_MUST) ;

    lcn_boolean_query_add( boolquery3, boolquery1, LCN_BOOLEAN_CLAUSE_MUST) ;
    lcn_boolean_query_add( boolquery3, boolquery2, LCN_BOOLEAN_CLAUSE_MUST) ;

    lcn_query_to_string(boolquery3, &aus, "x", pool);

    LCN_TEST_STATUS( lcn_searcher_search( searcher,
                                          &hits,
                                          boolquery3,
                                          NULL,
                                          pool ),  APR_SUCCESS );
    CuAssertIntEquals( tc, 1, lcn_hits_length( hits ) );

    apr_pool_destroy( pool );
}

static void
test_stack_overflow( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    unsigned int i;

    char *data[][2] = {
        { "dstz",  "2000" },
        { "nza",   "2000" },
        { "dstre", "2000" },
        { 0,0 }
    };

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    delete_files( tc, "test_index_writer" );

    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );

    i=0;

    while( data[i][0] != NULL )
    {
        LCN_TEST( lcn_document_create( &document, pool ) );

        LCN_TEST( lcn_field_create( &field,
                                    "titel",
                                    data[i][0],
                                    LCN_FIELD_INDEXED,
                                    LCN_FIELD_VALUE_COPY, pool ) );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        LCN_TEST( lcn_field_create( &field,
                                    "jahr",
                                    data[i][1],
                                    LCN_FIELD_INDEXED,
                                    LCN_FIELD_VALUE_COPY, pool ) );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );

        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

        i++;
    }

    LCN_TEST( lcn_index_writer_close( index_writer) );
    LCN_TEST( lcn_index_writer_optimize( index_writer ));

    {
        lcn_hits_t* hits;
        unsigned int hits_len;
        lcn_query_t *boolquery1;
        lcn_query_t *boolquery2;
        lcn_query_t *boolquery3;

        lcn_query_t *term_query1;
        lcn_query_t *term_query2;
        lcn_query_t *term_query3;

        lcn_searcher_t *searcher;

        LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                     "test_index_writer",
                                                     pool ) );


        LCN_TEST( lcn_boolean_query_create(&boolquery1, pool));
        LCN_TEST( lcn_boolean_query_create(&boolquery2, pool));
        LCN_TEST( lcn_boolean_query_create(&boolquery3, pool));

        LCN_TEST( lcn_term_query_create_by_chars( &term_query1, "jahr", "2000", pool ) );
        LCN_TEST( lcn_term_query_create_by_chars( &term_query2, "jahr", "0", pool ) );

        LCN_TEST( lcn_term_query_create_by_chars( &term_query3, "titel", "dstre", pool ) );

        LCN_TEST( lcn_boolean_query_add( boolquery1, term_query1, LCN_BOOLEAN_CLAUSE_SHOULD) );
        LCN_TEST( lcn_boolean_query_add( boolquery1, term_query2, LCN_BOOLEAN_CLAUSE_SHOULD) );

        LCN_TEST( lcn_boolean_query_add( boolquery2, term_query3, LCN_BOOLEAN_CLAUSE_MUST) );

        LCN_TEST( lcn_boolean_query_add( boolquery3, boolquery1, LCN_BOOLEAN_CLAUSE_MUST) );
        LCN_TEST( lcn_boolean_query_add( boolquery3, boolquery2, LCN_BOOLEAN_CLAUSE_MUST) );

        LCN_TEST( lcn_searcher_search( searcher, &hits, boolquery3, NULL, pool ) );

        hits_len = lcn_hits_length( hits );
        CuAssertIntEquals( tc, 1, hits_len );
    }

    apr_pool_destroy( pool );
}

static void
test_empty_query( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    char* query_str, *rewritten_str;
    lcn_query_t* query, *empty_query, *rewritten;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, cp ) );
    LCN_TEST( lcn_boolean_query_create( &empty_query, cp ) );

    LCN_TEST( lcn_boolean_query_add_term( query, "text", "a",
                                          LCN_BOOLEAN_CLAUSE_MUST ) );
    LCN_TEST( lcn_boolean_query_add_term( query, "text", "is",
                                          LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_boolean_query_add( query, empty_query,
                                     LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    LCN_TEST( lcn_query_rewrite( query, &rewritten, NULL, pool ) );
    LCN_TEST( lcn_query_to_string( rewritten, &rewritten_str, "", pool ) );
    CuAssertStrEquals( tc, "+text:a +text:is", rewritten_str );

    CuAssertIntEquals( tc, 7, lcn_hits_length( hits ) );
    apr_pool_destroy( pool );

}

#define ADD_TERM( QUERY, FIELD, TEXT )                           \
{                                                                \
    lcn_term_t* term;                                            \
    lcn_term_create( &term, FIELD, TEXT, LCN_TRUE, pool );       \
    lcn_multi_phrase_query_add_term( QUERY, term );              \
}

static void
test_phrase_disjunction( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* query, *pq1, *pq2;
    lcn_hits_t* hits;
    lcn_searcher_t* searcher;

    apr_pool_create(&pool, main_pool);

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, pool ) );


    lcn_multi_phrase_query_create( &pq1, pool );

    ADD_TERM( pq1, "text", "can" );
    ADD_TERM( pq1, "text", "be" );
    ADD_TERM( pq1, "text", "subclassed" );

    lcn_multi_phrase_query_create( &pq2, pool );
    ADD_TERM( pq2, "text", "can" );
    ADD_TERM( pq2, "text", "be" );
    ADD_TERM( pq2, "text", "stupid" );

    lcn_boolean_query_add( query, pq1, LCN_BOOLEAN_CLAUSE_SHOULD );
    lcn_boolean_query_add( query, pq2, LCN_BOOLEAN_CLAUSE_SHOULD );

    LCN_TEST_STATUS( lcn_searcher_search( searcher,
                                          &hits,
                                          query,
                                          NULL,
                                          pool ),  APR_SUCCESS );


    CuAssertIntEquals( tc, 1, lcn_hits_length( hits ) );

    lcn_index_searcher_close( searcher );

    apr_pool_destroy( pool );

}
#undef CHECK_HIT
#define CHECK_HIT( HITS, I, FIELD, TEXT )                    \
{                                                            \
    lcn_document_t* doc;                                     \
    char* str;                                               \
    LCN_TEST( lcn_hits_doc( HITS, &doc, I, pool ) );         \
    LCN_TEST( lcn_document_get( doc, &str, FIELD, pool ) );  \
    CuAssertStrEquals( tc, TEXT, str );                      \
}

static void
test_boost( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* query, *query2, *query3, *c_query;
    lcn_hits_t* hits;
    lcn_searcher_t* searcher;

    apr_pool_create(&pool, main_pool);

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query,  pool ) );
    lcn_boolean_query_add_term( query, "id", "KW1",
                                LCN_BOOLEAN_CLAUSE_SHOULD );

    LCN_TEST( lcn_boolean_query_create( &query2, pool ) );
    lcn_boolean_query_add_term( query2, "id", "KW2",
                                LCN_BOOLEAN_CLAUSE_SHOULD );

    LCN_TEST( lcn_boolean_query_create( &query3, pool ) );
    lcn_boolean_query_add_term( query3, "id", "KW3",
                                LCN_BOOLEAN_CLAUSE_SHOULD );


    lcn_query_boost_set( query3, 10 );
    lcn_query_boost_set( query2, 4 );
    LCN_TEST( lcn_boolean_query_create( &c_query, pool ) );
    lcn_boolean_query_add( c_query, query,
                           LCN_BOOLEAN_CLAUSE_SHOULD );
    lcn_boolean_query_add( c_query, query2,
                           LCN_BOOLEAN_CLAUSE_SHOULD );
    lcn_boolean_query_add( c_query, query3,
                           LCN_BOOLEAN_CLAUSE_SHOULD );

    LCN_TEST_STATUS( lcn_searcher_search( searcher,
                                          &hits,
                                          c_query,
                                          NULL,
                                          pool ),  APR_SUCCESS );

    CuAssertIntEquals( tc, 3, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "id", "KW3" );
    CHECK_HIT( hits, 1, "id", "KW2" );
    CHECK_HIT( hits, 2, "id", "KW1" );

    lcn_index_searcher_close( searcher );

    apr_pool_destroy( pool );

}

static void
test_conjunction( CuTest* tc )
{
    apr_pool_t *pool;
    char index[] = "bq_index";

    {
        apr_hash_t* map;

        apr_pool_create( &pool, main_pool );
        delete_files( tc, index );
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );
        LCN_TEST( lcn_index_writer_create_index_by_dump( index,
                                                         "bq.txt",
                                                         map,
                                                         LCN_TRUE, /* optimize */
                                                         pool ));
        apr_pool_destroy( pool );
    }

    {
        lcn_searcher_t* searcher;
        lcn_query_t *query;
        lcn_hits_t *hits;

        apr_pool_create( &pool, main_pool );

        LCN_TEST( lcn_index_searcher_create_by_path( &searcher, index, pool ));
        LCN_TEST( lcn_boolean_query_create( &query, pool ));
        LCN_TEST( lcn_boolean_query_add_term( query, "field_A", "1",     LCN_BOOLEAN_CLAUSE_MUST ) );
        LCN_TEST( lcn_boolean_query_add_term( query, "field_B", "1995",  LCN_BOOLEAN_CLAUSE_MUST ) );
        LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );

        CuAssertIntEquals( tc, 1, lcn_hits_length( hits ) );

        apr_pool_destroy( pool );
    }
}

static void
test_must_should( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* query, *query2, *c_query;
    lcn_hits_t* hits;
    lcn_searcher_t* searcher;

    apr_pool_create(&pool, main_pool);

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query,  pool ) );

    LCN_TEST( lcn_boolean_query_add_term( query, "id", "KW1",
                                          LCN_BOOLEAN_CLAUSE_SHOULD ) );


    LCN_TEST( lcn_boolean_query_add_term( query, "id", "KW2",
                                          LCN_BOOLEAN_CLAUSE_SHOULD ) );


    LCN_TEST( lcn_boolean_query_add_term( query, "id", "KW3",
                                          LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_boolean_query_create( &query2, pool ) );

    LCN_TEST( lcn_boolean_query_add_term( query2, "id", "KW4",
                                LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_boolean_query_create( &c_query, pool ) );

    LCN_TEST( lcn_boolean_query_add( c_query, query,
                           LCN_BOOLEAN_CLAUSE_MUST ) );
    LCN_TEST( lcn_boolean_query_add( c_query, query2,
                           LCN_BOOLEAN_CLAUSE_SHOULD ) );


    LCN_TEST_STATUS( lcn_searcher_search( searcher,
                                          &hits,
                                          c_query,
                                          NULL,
                                          pool ),  APR_SUCCESS );

    CuAssertIntEquals( tc, 3, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "id", "KW1" );
    CHECK_HIT( hits, 1, "id", "KW2" );
    CHECK_HIT( hits, 2, "id", "KW3" );

    lcn_index_searcher_close( searcher );

    apr_pool_destroy( pool );

}

static void
test_seg_fault_query( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* query, *query2, *bq;
    lcn_hits_t* hits;
    lcn_searcher_t* searcher;

    apr_pool_create(&pool, main_pool);

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_boolean_query_create( &query, pool ) );
    LCN_TEST( lcn_boolean_query_add_term( query, "id", "KW1", LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_create( &query2, pool ) );
    LCN_TEST( lcn_boolean_query_add( query2, query, LCN_BOOLEAN_CLAUSE_MUST ) );
    LCN_TEST( lcn_boolean_query_create( &bq, pool ) );
    LCN_TEST( lcn_boolean_query_add_term( bq, "id", "KW2", LCN_BOOLEAN_CLAUSE_SHOULD ) );
    LCN_TEST( lcn_boolean_query_add_term( bq, "id", "KW3", LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST( lcn_boolean_query_add( query2, bq, LCN_BOOLEAN_CLAUSE_SHOULD ) );

    LCN_TEST_STATUS( lcn_searcher_search( searcher,
                                          &hits,
                                          query2,
                                          NULL,
                                          pool ),  APR_SUCCESS );

    lcn_index_searcher_close( searcher );

    apr_pool_destroy( pool );
}

static void setup()
{
    test_index_name = "test_index_2";
}

static void setup_multi()
{
    test_index_name = "test_index_1";
}

CuSuite* make_boolean_query_suite()
{
    CuSuite* s = CuSuiteNew();


    SUITE_ADD_TEST( s, setup );

    SUITE_ADD_TEST( s, test_failing_disjunction );
    SUITE_ADD_TEST( s, test_boost_sorting );
    SUITE_ADD_TEST( s, test_must_should );
    SUITE_ADD_TEST( s, test_boost );
    SUITE_ADD_TEST( s, test_phrase_disjunction );
    SUITE_ADD_TEST( s, test_boolean_query );
    SUITE_ADD_TEST( s, test_clone );
    SUITE_ADD_TEST( s, test_empty_query );
    SUITE_ADD_TEST( s, test_failing_query );
    SUITE_ADD_TEST( s, test_multi_phrase_in_boolean );
    SUITE_ADD_TEST( s, test_prefix_combo );
    SUITE_ADD_TEST( s, test_req_prohib );
    SUITE_ADD_TEST( s, test_combo_booleans );
    SUITE_ADD_TEST( s, test_opt_prohib );
    SUITE_ADD_TEST( s, test_first_record );
    SUITE_ADD_TEST( s, test_null_record );
    SUITE_ADD_TEST( s, test_conjunction );
    SUITE_ADD_TEST( s, test_seg_fault_query );

    SUITE_ADD_TEST( s, setup_multi );
    SUITE_ADD_TEST( s, test_failing_query );
    SUITE_ADD_TEST( s, test_empty_query );
    SUITE_ADD_TEST( s, test_boolean_query );
    SUITE_ADD_TEST( s, test_combo_booleans );
    SUITE_ADD_TEST( s, test_opt_prohib );
    SUITE_ADD_TEST( s, test_req_prohib );
    SUITE_ADD_TEST( s, test_prefix_combo );
    SUITE_ADD_TEST( s, test_multi_phrase_in_boolean );
    SUITE_ADD_TEST( s, test_stack_overflow );
    SUITE_ADD_TEST( s, test_boost_sorting );

    return s;
}
