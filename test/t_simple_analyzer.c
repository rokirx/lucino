#include "lucene.h"
#include "lucene_util.h"
#include "test_all.h"
#include "analysis/char_tokenizer.h"
#include "analysis/lowercase_tokenizer.h"

#define NEXT_TOKEN( TEXT )                                      \
{                                                               \
    LCN_TEST( lcn_token_stream_next( ts, &token ) ); 	        \
    lcn_token_term_text( token, &string, pool );                \
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
    lcn_analyzer_t* sa;
    lcn_token_stream_t* ts;
    lcn_token_t* token;
    char* string;

    char test_str[]="Eins1 Zwei2 asdf\304\374\326-2342 BLAH";
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_simple_analyzer_create( &sa, pool ) );
    CuAssertStrEquals( tc, "lcn_simple_analyzer",
                       lcn_analyzer_type( sa ) );
    LCN_TEST( lcn_analyzer_token_stream( sa,
                                         &ts,
                                         test_str,
                                         pool ));
    NEXT_TOKEN( "eins" );
    NEXT_TOKEN( "zwei" );
    NEXT_TOKEN( "asdf\344\374\366" );
    NEXT_TOKEN( "blah" );
    END_OF_STREAM;

    apr_pool_destroy( pool );
}

static void
test_empty_string( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_analyzer_t* sa;
    lcn_token_stream_t* ts;
    lcn_token_t* token;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_simple_analyzer_create( &sa, pool ) );
    LCN_TEST( lcn_analyzer_token_stream( sa, &ts, "", pool ));

    END_OF_STREAM;

    apr_pool_destroy( pool );
}

static void
test_invalid_chars_only( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_analyzer_t* sa;
    lcn_token_stream_t* ts;
    lcn_token_t* token;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_simple_analyzer_create( &sa, pool ) );
    LCN_TEST( lcn_analyzer_token_stream( sa, &ts,
                                         "2 3 4 2 3 - .,  234 .-,234.-",
                                         pool ));
    END_OF_STREAM;

    apr_pool_destroy( pool );
}

CuSuite* make_simple_analyzer_suite( void )
{
  CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, test_char_number_tokens );
    SUITE_ADD_TEST( s, test_empty_string );
    SUITE_ADD_TEST( s, test_invalid_chars_only );
    return s;
}
