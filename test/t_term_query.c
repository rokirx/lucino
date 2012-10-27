#include "test_all.h"
#include "lcn_search.h"
#include "lcn_index.h"
#include "lcn_analysis.h"

static char* test_ids[]= { "KW21",
                           "KW67",
                           "KW70",
                           "KW105",
                           "KW172",
                           "KW299",
                           "KW322",
                           "KW45",
                           "KW130"
};

static void
test_term_query_impl( CuTest* tc, const char *index_dir )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    unsigned int doc_freq, i;
    lcn_term_t* term;
    char* query_str;
    lcn_query_t* query, *clone;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_term_create( &term, "text", "can", 1, pool ) );
    LCN_TEST( lcn_term_query_create( &query, term, pool ) );
    lcn_query_set_name( query, "a named query" );
    CuAssertStrEquals( tc, "a named query", lcn_query_name( query ));
    LCN_TEST( lcn_query_clone( query, &clone, pool ) );
    CuAssertStrEquals( tc, "a named query", lcn_query_name( clone ));
    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CuAssertStrEquals( tc, query_str, "text:can" );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 index_dir,
                                                 pool ) );


    CuAssertIntEquals( tc, 331,  lcn_searcher_max_doc( searcher ) );

    LCN_TEST( lcn_searcher_doc_freq( searcher, term, &doc_freq ) );
    CuAssertIntEquals( tc, 9, doc_freq );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   clone,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 9, lcn_hits_length( hits ) );

    for( i = 0; i < 9; i++ )
    {
        lcn_document_t* doc;
        char* id, *text;

        apr_pool_clear( cp );

        LCN_TEST( lcn_hits_doc( hits, &doc, i, cp ) );
        LCN_TEST( lcn_document_get( doc, &id, "id", cp ) );
        LCN_TEST( lcn_document_get( doc, &text, "text", cp ) );

        CuAssertStrEquals( tc, test_ids[i], id );
    }

    apr_pool_destroy( pool );
}

static void
test_omit_norms( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    unsigned int doc_freq;
    lcn_term_t* term;
    lcn_query_t* query;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );

    LCN_TEST( lcn_term_create( &term, "titel", "second", 1, pool ) );
    LCN_TEST( lcn_term_query_create( &query, term, pool ) );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "index_writer/index_10",
                                                 pool ) );


    CuAssertIntEquals( tc, 5,  lcn_searcher_max_doc( searcher ) );
    LCN_TEST( lcn_searcher_doc_freq( searcher, term, &doc_freq ) );
    CuAssertIntEquals( tc, 1, doc_freq );

    LCN_TEST( lcn_searcher_search( searcher,
                                   &hits,
                                   query,
                                   NULL,
                                   pool ) );

    CuAssertIntEquals( tc, 1, lcn_hits_length( hits ) );

    {
        lcn_document_t* doc;
        const char *value;
        lcn_field_t *field;
        char val[10];

        apr_pool_clear( cp );

        LCN_TEST( lcn_hits_doc( hits, &doc, 0, cp ) );
        LCN_TEST( lcn_document_get_field( doc, "text", &field ) );
        CuAssertIntEquals( tc, 4, lcn_field_size( field ));
        value = lcn_field_value( field );
        memcpy( val, value, lcn_field_size( field ));
        val[ lcn_field_size( field ) ] = '\0';
        CuAssertStrEquals( tc, "123Ö", val );
    }

    apr_pool_destroy( pool );
}

static void
test_term_query( CuTest* tc )
{
    test_term_query_impl( tc, "test_index_2" );
    test_term_query_impl( tc, "test_index_1" );
}

CuSuite* make_term_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_term_query );
    SUITE_ADD_TEST( s, test_omit_norms );

    return s;
}
