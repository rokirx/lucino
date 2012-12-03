#include "test_all.h"
#include "lcn_search.h"
#include "lcn_analysis.h"

#define CHECK_HIT( HITS, NTH, FIELD, TEXT )                     \
{                                                               \
    lcn_document_t* doc;                                        \
    char* content;                                              \
                                                                \
    LCN_TEST( lcn_hits_doc( HITS, &doc, NTH, pool ) );          \
    LCN_TEST( lcn_document_get( doc, &content, FIELD, pool ) ); \
    CuAssertStrEquals( tc, TEXT, content );                     \
}

static char* test_index_name;

static lcn_list_t* _terms;
static lcn_list_t* _alternatives;
static apr_pool_t* _pool;

static char* words[] = {
    "so",
    "that" ,
    "it",
    "can",
    "be",
    "subclassed",
    "by",
    "classes"
};

static char* alternatives[] = {
    "they",
    "it"
};


static void fill_terms( const char *field,
                        lcn_list_t *terms_list,
                        lcn_list_t *alternatives_list,
                        apr_pool_t *pool )
{
    unsigned int i;

    for( i = 0; i < 8; i++ )
    {
        lcn_term_t* term;

        lcn_term_create( &term,
                         field,
                         words[i],
                         LCN_TRUE,
                         pool );

        lcn_list_add( terms_list, term );
    }

    for( i = 0; i < 2; i++ )
    {
        lcn_term_t* term;
        lcn_term_create( &term,
                         field,
                         alternatives[i],
                         LCN_TRUE,
                         pool );

        lcn_list_add( alternatives_list, term );
    }
}

static void
test_query( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    unsigned int i;
    char* query_str;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_list_t* terms;
    lcn_document_t* doc;
    char* id;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    lcn_list_create( &terms, 10, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_multi_phrase_query_create( &query, cp ) );

    for( i = 0; i < 8; i++ )
    {
        LCN_TEST( lcn_multi_phrase_query_add_term( query,
                                                   lcn_list_get( _terms,
                                                                 i ) ) );
    }

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc,
                       "text:\"so that it can be subclassed by classes\"",
                       query_str );


    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );


    CuAssertIntEquals( tc, 1, lcn_hits_length( hits ) );


    lcn_hits_doc( hits, &doc, 0, pool );
    lcn_document_get( doc, &id, "id", pool );

    CuAssertStrEquals( tc, "KW67", id );

    apr_pool_destroy( pool );
}

static void
test_query_with_wrong_field( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    unsigned int i;
    char* query_str;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_list_t* terms;

    lcn_list_t* terms_list;
    lcn_list_t* alternatives_list;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    lcn_list_create( &terms, 10, pool );
    lcn_list_create( &terms_list, 10, pool );
    lcn_list_create( &alternatives_list, 10, pool );

    fill_terms( "tt", terms_list, alternatives_list, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_2",
                                                 pool ) );

    LCN_TEST( lcn_multi_phrase_query_create( &query, cp ) );

    for( i = 0; i < 8; i++ )
    {
        LCN_TEST( lcn_multi_phrase_query_add_term( query, lcn_list_get( terms_list, i )));
    }

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc,
                       "tt:\"so that it can be subclassed by classes\"",
                       query_str );


    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );


    CuAssertIntEquals( tc, 0, lcn_hits_length( hits ) );

    apr_pool_destroy( pool );
}

static void
test_alternatives( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    char* query_str;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_list_t* terms;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    lcn_list_create( &terms, 10, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_multi_phrase_query_create( &query, cp ) );

    LCN_TEST( lcn_multi_phrase_query_add_term( query,
                                               lcn_list_get( _terms, 0 ) ) );

    LCN_TEST( lcn_multi_phrase_query_add_terms( query,
                                                _alternatives ) );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc,
                       "text:\"so (they it)\"",
                       query_str );

    LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool )  );

    CuAssertIntEquals( tc, 2, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "id", "KW260" );
    CHECK_HIT( hits, 1, "id", "KW255" );

    apr_pool_destroy( pool );
}

static void
test_sloppy_query( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    unsigned int i;
    char* query_str;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_list_t* terms;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    lcn_list_create( &terms, 10, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_multi_phrase_query_create( &query, cp ) );

    for( i = 0; i < 3; i+=2 )
    {
        LCN_TEST( lcn_multi_phrase_query_add_term( query,
                                                   lcn_list_get( _terms,
                                                                 i ) ) );
    }

    lcn_multi_phrase_query_slop_set( query, 4 );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );


    //CuAssertStrEquals( tc, "text:\"so it\"~4", query_str );


    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 4, lcn_hits_length( hits ) );


    CHECK_HIT( hits, 0, "id", "KW255" );
    CHECK_HIT( hits, 1, "id", "KW260" );
    CHECK_HIT( hits, 2, "id", "KW67" );
    CHECK_HIT( hits, 3, "id", "KW69" );
    apr_pool_destroy( pool );
}

static void
test_clone( CuTest* tc )
{
    apr_pool_t* pool, *cp, *cp2;
    lcn_searcher_t* searcher;
    unsigned int i;
    char* query_str, *clone_str;
    lcn_query_t* query, *clone;
    lcn_list_t* terms;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    apr_pool_create( &cp2, pool );
    lcn_list_create( &terms, 10, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_multi_phrase_query_create( &query, cp ) );

    for( i = 0; i < 3; i+=2 )
    {
        LCN_TEST( lcn_multi_phrase_query_add_term( query,
                                                   lcn_list_get( _terms,
                                                                 i ) ) );
    }

    lcn_multi_phrase_query_slop_set( query, 4 );
    LCN_TEST( lcn_query_clone( query, &clone, cp2 ) );
    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    LCN_TEST( lcn_query_to_string( clone, &clone_str, "", pool ) );
    apr_pool_destroy( cp );

    CuAssertStrEquals( tc, query_str, clone_str );

    apr_pool_destroy( pool );
}

static void
test_sloppy_query_with_natural_sorting( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    unsigned int i;
    char* query_str;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_list_t* terms;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    lcn_list_create( &terms, 10, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_searcher_order_by( searcher, LCN_ORDER_BY_NATURAL ));

    LCN_TEST( lcn_multi_phrase_query_create( &query, cp ) );

    for( i = 0; i < 3; i+=2 )
    {
        LCN_TEST( lcn_multi_phrase_query_add_term( query,
                                                   lcn_list_get( _terms,
                                                                 i ) ) );
    }

    lcn_multi_phrase_query_slop_set( query, 4 );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );


    //CuAssertStrEquals( tc, "text:\"so it\"~4", query_str );


    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 4, lcn_hits_length( hits ) );


    CHECK_HIT( hits, 0, "id", "KW67" );
    CHECK_HIT( hits, 1, "id", "KW69" );
    CHECK_HIT( hits, 2, "id", "KW255" );
    CHECK_HIT( hits, 3, "id", "KW260" );
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
test_failing_query( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_analyzer_t *sa;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    delete_files( tc, "test_index_writer" );

    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );

    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "text",
                                "test njw abc, 622 test",
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

    {
        lcn_hits_t* hits;
        unsigned int hits_len, i;
        lcn_query_t *phrase_query;
        lcn_term_t *term, *term1;
        lcn_searcher_t *searcher;

        LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                     "test_index_writer",
                                                     pool ) );

        LCN_TEST( lcn_multi_phrase_query_create( &phrase_query, pool ));

        LCN_TEST( lcn_term_create( &term, "text", "njw", LCN_TERM_TEXT_COPY, pool ));
        LCN_TEST( lcn_multi_phrase_query_add_term( phrase_query, term));

        LCN_TEST( lcn_searcher_search( searcher, &hits, phrase_query, NULL, pool ) );
        hits_len = lcn_hits_length( hits );
        CuAssertIntEquals( tc, 1, hits_len );

        LCN_TEST( lcn_term_create( &term1, "text", "abc", LCN_TERM_TEXT_COPY, pool ));
        LCN_TEST( lcn_multi_phrase_query_add_term( phrase_query, term1 ) );

        LCN_TEST( lcn_searcher_search( searcher, &hits, phrase_query, NULL, pool ) );
        hits_len = lcn_hits_length( hits );
        CuAssertIntEquals( tc, 1, hits_len );
   
        for( i = 0; i < lcn_hits_length( hits ); i++ )
        {
            char* str;
            lcn_document_t* doc;

            lcn_hits_doc( hits, &doc, i, pool );
            lcn_document_get( doc, &str, "text", pool );
            LCNLOG_STR( "Text", str );
        }
#if 0
        LCN_TEST( lcn_term_create( &term1, "volltext", "2004", LCN_TERM_TEXT_COPY, pool ));
        LCN_TEST( lcn_multi_phrase_query_add_term( phrase_query, term) );

        LCN_TEST( lcn_term_create( &term, "volltext", "622", LCN_TERM_TEXT_COPY, pool ));
        LCN_TEST( lcn_multi_phrase_query_add_term( phrase_query, term ));
#endif
        //lcn_multi_phrase_query_slop_set( phrase_query, 1 );

        LCN_TEST( lcn_searcher_search( searcher, &hits, phrase_query, NULL, pool ) );

        hits_len = lcn_hits_length( hits );
        CuAssertIntEquals( tc, 1, hits_len );
    }

    apr_pool_destroy( pool );
}

static void setup( CuTest* tc )
{
    apr_pool_create( &_pool, main_pool );

    lcn_list_create( &_terms, 10, _pool );
    lcn_list_create( &_alternatives, 10, _pool );
    fill_terms( "text", _terms, _alternatives, _pool );
    test_index_name = "test_index_2";
}

static void setup_multi()
{
    test_index_name = "test_index_1";
}

static void teardown()
{
    apr_pool_destroy( _pool );
}

static void
setup_ram_dir_test_index( CuTest *tc,
                          lcn_directory_t **dir,
                          char **phrase_list,
                          apr_pool_t *pool )
{
    lcn_analyzer_t *sa;
    int i = 0;
    lcn_index_writer_t *index_writer;
    char *text;

    LCN_TEST( lcn_ram_directory_create( dir, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &sa, pool ) );
    LCN_TEST( lcn_index_writer_create_by_directory( &index_writer, *dir, LCN_TRUE, pool ));

    while( (text = phrase_list[i++]) != NULL )
    {
        lcn_field_t *field;
        lcn_document_t *document;

        LCN_TEST( lcn_document_create( &document, pool ) );

        LCN_TEST( lcn_field_create( &field,
                                    "text",
                                    text,
                                    LCN_FIELD_INDEXED |
                                    LCN_FIELD_TOKENIZED |
                                    LCN_FIELD_STORED,
                                    LCN_FIELD_VALUE_COPY, pool ) );

        lcn_field_set_analyzer( field, sa );
        LCN_TEST( lcn_document_add_field( document, field, pool ) );
        LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );
    }

    LCN_TEST( lcn_index_writer_close( index_writer) );
    LCN_TEST( lcn_index_writer_optimize( index_writer ));
}

static void
test_hits_length( CuTest *tc,
                  lcn_directory_t *dir,
                  char *query_string,
                  int hits_length,
                  apr_pool_t *pool )
{
    lcn_hits_t* hits;
    lcn_query_t *q;
    lcn_searcher_t *searcher;
    lcn_index_reader_t *reader;

    LCN_TEST( lcn_index_reader_create_by_directory( &reader, dir, LCN_FALSE, pool ));
    LCN_TEST( lcn_index_searcher_create_by_reader( &searcher, reader, pool ));

    LCN_TEST( lcn_parse_query( &q, query_string, pool ));
    LCN_TEST( lcn_searcher_search( searcher, &hits, q, NULL, pool ));

    CuAssertIntEquals( tc, hits_length, lcn_hits_length( hits ) );

    LCN_TEST( lcn_index_reader_close( reader ));
}



static void
test_phrase_bug_1( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    char *phrase_list[] = {
        "a sf foo bar sf",
        NULL
    };

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    setup_ram_dir_test_index( tc, &dir, phrase_list, pool );

    {
        lcn_hits_t* hits;
        lcn_query_t *pq;
        lcn_term_t *term;
        lcn_searcher_t *searcher;
        lcn_index_reader_t *reader;

        LCN_TEST( lcn_index_reader_create_by_directory( &reader, dir, LCN_TRUE, pool ));

        /* check the proximity data for the term "sf" */
        {
            lcn_term_docs_t *term_docs;
            lcn_term_t *term;
            apr_ssize_t pos;

            LCN_TEST( lcn_index_reader_term_positions( reader, &term_docs, pool ) );

            LCN_TEST( lcn_term_create( &term, "text", "a", LCN_TERM_TEXT_COPY, pool ));
            LCN_TEST( lcn_term_docs_seek_term( term_docs, term ));
            LCN_TEST( lcn_term_docs_next( term_docs ));
            LCN_TEST( lcn_term_positions_next_position( term_docs, &pos ) );
            CuAssertIntEquals( tc, 0, pos );

            LCN_TEST( lcn_term_create( &term, "text", "foo", LCN_TERM_TEXT_COPY, pool ));
            LCN_TEST( lcn_term_docs_seek_term( term_docs, term ));
            LCN_TEST( lcn_term_docs_next( term_docs ));
            LCN_TEST( lcn_term_positions_next_position( term_docs, &pos ) );
            CuAssertIntEquals( tc, 2, pos );

            LCN_TEST( lcn_term_create( &term, "text", "bar", LCN_TERM_TEXT_COPY, pool ));
            LCN_TEST( lcn_term_docs_seek_term( term_docs, term ));
            LCN_TEST( lcn_term_docs_next( term_docs ));
            LCN_TEST( lcn_term_positions_next_position( term_docs, &pos ) );
            CuAssertIntEquals( tc, 3, pos );

            LCN_TEST( lcn_term_create( &term, "text", "sf", LCN_TERM_TEXT_COPY, pool ));
            LCN_TEST( lcn_term_docs_seek_term( term_docs, term ));
            LCN_TEST( lcn_term_docs_next( term_docs ));
            LCN_TEST( lcn_term_positions_next_position( term_docs, &pos ) );
            CuAssertIntEquals( tc, 1, pos );
            LCN_TEST( lcn_term_positions_next_position( term_docs, &pos ) );
            CuAssertIntEquals( tc, 4, pos );
        }

        test_hits_length( tc, dir, "text:\"bar sf\"", 1, pool );
        test_hits_length( tc, dir, "text:\"foo bar sf\"", 1, pool );

        LCN_TEST( lcn_index_reader_close( reader ));
    }

    apr_pool_destroy( pool );
}


static void
test_phrase_bug_2( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    char *phrase_list[] = {
        "sf foo bar sf",
        NULL
    };

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    setup_ram_dir_test_index( tc, &dir, phrase_list, pool );

    test_hits_length( tc, dir, "text:\"bar sf\"", 1, pool );
    test_hits_length( tc, dir, "text:\"foo bar sf\"", 1, pool );

    apr_pool_destroy( pool );
}


static void
test_phrase_bug_3( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    char *phrase_list[] = {
        "sf foo bar bar bar",
        "sf foo bar ddd",
        NULL
    };

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    setup_ram_dir_test_index( tc, &dir, phrase_list, pool );
    test_hits_length( tc, dir, "text:\"sf foo bar bar bar\"", 1, pool );

    apr_pool_destroy( pool );
}

static void
test_phrase_bug_4( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_directory_t *dir;

    char *phrase_list[] = {
        "sf foo bar ddd",
        NULL
    };

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    setup_ram_dir_test_index( tc, &dir, phrase_list, pool );
    test_hits_length( tc, dir, "text:\"sf foo bar bar bar\"", 0, pool );

    apr_pool_destroy( pool );
}


static void
test_gap_query( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    unsigned int i;
    char* query_str;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_list_t* terms;
    lcn_document_t* doc;
    char* id;
    lcn_term_t *term1, *term2;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 test_index_name,
                                                 pool ) );

    LCN_TEST( lcn_multi_phrase_query_create( &query, cp ) );

    lcn_list_create( &terms, 10, pool );
    LCN_TEST( lcn_term_create( &term1, "text", "to", LCN_TRUE, pool ));
    lcn_list_add( terms, term1 );
    LCN_TEST( lcn_term_create( &term1, "text", "parser", LCN_TRUE, pool ));
    lcn_list_add( terms, term1 );
    LCN_TEST( lcn_multi_phrase_query_add_terms( query, terms ));

    lcn_list_create( &terms, 10, pool );
    LCN_TEST( lcn_term_create( &term1, "text", "query", LCN_TRUE, pool ));
    lcn_list_add( terms, term1 );
    LCN_TEST( lcn_term_create( &term1, "text", "index", LCN_TRUE, pool ));
    lcn_list_add( terms, term1 );
    LCN_TEST( lcn_multi_phrase_query_add_terms( query, terms ));


    lcn_multi_phrase_query_slop_set( query, 2 );
    lcn_multi_phrase_query_preserve_term_order_set( query, LCN_TRUE );

    //LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );
    //flog( stderr, "query_str: %s\n", query_str );


    LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );

    CuAssertIntEquals( tc, 5, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "id", "KW227" );
    CHECK_HIT( hits, 1, "id", "KW142" );
    CHECK_HIT( hits, 2, "id", "KW184" );
    CHECK_HIT( hits, 3, "id", "KW239" );
    CHECK_HIT( hits, 4, "id", "KW89"  );

#if 0
    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_hits_doc( hits, &doc, i, pool );
        lcn_document_get( doc, &id, "text", pool );
        flog( stderr, "text: <%s>\n", id );
        lcn_document_get( doc, &id, "id", pool );
        flog( stderr, "id: <%s>\n", id );
    }
#endif

    apr_pool_destroy( pool );
}

CuSuite*
make_multi_phrase_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_phrase_bug_1 );
    SUITE_ADD_TEST( s, test_phrase_bug_2 );
    SUITE_ADD_TEST( s, test_phrase_bug_3 );
    SUITE_ADD_TEST( s, test_phrase_bug_4 );
    SUITE_ADD_TEST( s, test_failing_query );
    SUITE_ADD_TEST( s, test_query_with_wrong_field );

    SUITE_ADD_TEST( s, setup );
    SUITE_ADD_TEST( s, test_query_with_wrong_field );
    SUITE_ADD_TEST( s, test_clone );
    SUITE_ADD_TEST( s, test_query );
    SUITE_ADD_TEST( s, test_gap_query );
    SUITE_ADD_TEST( s, test_sloppy_query );
    SUITE_ADD_TEST( s, test_sloppy_query_with_natural_sorting );
    SUITE_ADD_TEST( s, test_alternatives   );

    SUITE_ADD_TEST( s, setup_multi );
    SUITE_ADD_TEST( s, test_query );
    SUITE_ADD_TEST( s, test_sloppy_query );
    SUITE_ADD_TEST( s, test_sloppy_query_with_natural_sorting );
    SUITE_ADD_TEST( s, test_stack_overflow );
    SUITE_ADD_TEST( s, teardown );

    return s;
}
