#include "test_all.h"
#include "lcn_search.h"
#include "lcn_index.h"
#include "lcn_analysis.h"

static const char* test_index;

#define CHECK_POS( TERM, POS, ID, STR )					\
{									\
    lcn_document_t* doc;						\
    char* id, *text;							\
									\
    LCN_TEST( lcn_term_create( &term, "text", TERM, LCN_TRUE, pool ) );	\
    LCN_TEST( lcn_term_pos_query_create( &query, term, POS, pool ) );	\
    LCN_TEST( lcn_searcher_search( searcher,				\
                                   &hits,				\
				   query,				\
                                   NULL,				\
                                   pool ) );                            \
									\
    LCN_TEST( lcn_hits_doc( hits, &doc, 0, cp ) );			\
    LCN_TEST( lcn_document_get( doc, &id, "id", cp ) );			\
    LCN_TEST( lcn_document_get( doc, &text, "text", cp ) );		\
									\
    CuAssertStrEquals( tc, id, ID );					\
    CuAssertStrEquals( tc, text, STR );					\
}


static void
test_term_pos_query_impl( CuTest* tc, const char *index_dir )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    lcn_term_t* term;
    char* query_str;
    lcn_query_t* query;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 index_dir,		
                                                 pool ) );		
    LCN_TEST( lcn_term_create( &term, "text", "can", 2, pool ) );
    LCN_TEST( lcn_term_pos_query_create( &query, term, 1, pool ) );
    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ));

    CuAssertStrEquals( tc, "term_pos_query",
		       lcn_query_type_string( query ) );

    LCN_TEST( lcn_query_to_string( query, &query_str, "", pool ) );

    CHECK_POS( "can", 1, "KW299", "- Analyzers can choose tokenizer based on "
	       "field name" );
    CHECK_POS( "analyzers", 0, "KW299", "- Analyzers can choose tokenizer "
	       "based on field name" );
    CHECK_POS( "be", 0, "KW117", "be correct.  With the new code and an old "
	       "index, searches will" );
    CHECK_POS( "subclassed", 7, "KW67", "11. Changed IndexReader so that "
	       "it can be subclassed by classes" );

    LCN_TEST( lcn_index_reader_close( lcn_index_searcher_reader( searcher ) ) );

    apr_pool_destroy( pool );
}

static void
test_term_pos_in_boolean( CuTest* tc )
{
    apr_pool_t* pool, *cp;
    lcn_searcher_t* searcher;
    lcn_query_t* tpq1, *tpq2, *b_query;
    lcn_hits_t* hits;
    lcn_document_t* doc;
    char* str;

    apr_pool_create( &pool, main_pool );
    apr_pool_create( &cp, pool );
    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
						 test_index,
                                                 pool ) );		
    LCN_TEST( lcn_term_pos_query_create_by_chars( &tpq1, 
						  "text", "can", 9, pool ) );
    LCN_TEST( lcn_term_pos_query_create_by_chars( &tpq2, 
						  "text", "be", 11, pool ) );
    LCN_TEST( lcn_boolean_query_create( &b_query, pool ) );

    LCN_TEST( lcn_boolean_query_add( b_query, tpq1,
				     LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_boolean_query_add( b_query, tpq2,
				     LCN_BOOLEAN_CLAUSE_MUST ) );

    LCN_TEST( lcn_searcher_search( searcher, &hits, b_query, NULL, pool ) );
    
    CuAssertIntEquals( tc, 1, lcn_hits_length( hits ) );
    lcn_hits_doc( hits, &doc, 0, pool );
    lcn_document_get( doc, &str, "id", pool );
    CuAssertStrEquals( tc, str, "KW130" );

    LCN_TEST( lcn_index_searcher_close( searcher ) );

    apr_pool_destroy( pool );
}

static void setup_single( CuTest* tc ) { test_index="test_index_1"; }
static void setup_multi( CuTest* tc ) { test_index="test_index_2"; }

static void
test_term_pos_query( CuTest* tc )
{
    test_term_pos_query_impl( tc, "test_index_2" );
    test_term_pos_query_impl( tc, "test_index_1" );
}

CuSuite* make_term_pos_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_term_pos_query );

    SUITE_ADD_TEST( s, setup_single );
    SUITE_ADD_TEST( s, test_term_pos_in_boolean );
    SUITE_ADD_TEST( s, setup_multi );
    SUITE_ADD_TEST( s, test_term_pos_in_boolean );

    return s;
}
