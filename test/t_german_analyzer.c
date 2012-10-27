#include "lucene.h"
#include "lucene_util.h"
#include "test_all.h"
#include "analysis/char_tokenizer.h"
#include "analysis/lowercase_tokenizer.h"

#define NEXT_TOKEN( TEXT )                                      \
{                                                               \
    LCN_TEST( lcn_token_stream_next( ts, &token ) );  \
    lcn_token_term_text( token, &string, pool );                \
    CuAssertStrEquals( tc, TEXT, string );                      \
}

#define END_OF_STREAM                                           \
{                                                               \
    CuAssertIntEquals( tc, LCN_ERR_TOKEN_STREAM_EOS,            \
                       lcn_token_stream_next( ts, &token ) );   \
}

static void
test_char_number_tokens( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_analyzer_t* ga;
    lcn_token_stream_t* ts;
    lcn_token_t* token;
    char* string;

    char test_str[]="Mutter Vater Schule";

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_german_analyzer_create( &ga, pool ) );
    LCN_TEST( lcn_analyzer_token_stream( ga, &ts, test_str ) );
    CuAssertStrEquals( tc, "lcn_german_analyzer",
                       lcn_analyzer_type( ga ) );
    NEXT_TOKEN( "mutt" );
    NEXT_TOKEN( "vat" );
    NEXT_TOKEN( "schul" );
    END_OF_STREAM;

    apr_pool_destroy( pool );
}


CuSuite* make_german_analyzer_suite( void )
{
  CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, test_char_number_tokens );

    return s;
}
