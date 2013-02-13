#include "lucene.h"
#include "lucene_util.h"
#include "test_all.h"
#include "analysis/char_tokenizer.h"
#include "analysis/lowercase_tokenizer.h"

#define HUGE_TOKEN "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define HUGE_STRING " " HUGE_TOKEN " " HUGE_TOKEN " " HUGE_TOKEN

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

#define NEXT_FILTER_TOKEN( TEXT )                               \
{                                                               \
    LCN_TEST( lcn_token_stream_next( tf, &token ) );  \
    lcn_token_term_text( token, &string, pool );                \
    CuAssertStrEquals( tc, TEXT, string );                      \
}

#define END_OF_FILTER_STREAM                                    \
{                                                               \
    CuAssertIntEquals( tc, LCN_ERR_TOKEN_STREAM_EOS,            \
                       lcn_token_stream_next( tf, &token ) );   \
}


#define CHECK_OFFSETS( START, END )                                     \
{                                                                       \
  CuAssertIntEquals( tc, START, (int)lcn_token_start_offset( token ) ); \
  CuAssertIntEquals( tc, END, (int)lcn_token_end_offset( token ) );     \
}

static void
test_lowercase_filter( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_token_stream_t* ts;
    lcn_token_stream_t* tf;
    lcn_token_t* token;
    char* string;
    
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_char_tokenizer_create( &ts,
                                                   " EINS zwei DREI vier "
                                                   "f\374nf sechs"
                                                   " abc-123",
                                                   pool ) );

    LCN_TEST( lcn_lowercase_filter_create( &tf,
                                                     ts,
                                                     pool ) );


    NEXT_FILTER_TOKEN( "eins" );
    NEXT_FILTER_TOKEN( "zwei" );
    NEXT_FILTER_TOKEN( "drei" );
    NEXT_FILTER_TOKEN( "vier" );
    NEXT_FILTER_TOKEN( "f\374nf" );
    NEXT_FILTER_TOKEN( "sechs" );
    NEXT_FILTER_TOKEN( "abc" );
    NEXT_FILTER_TOKEN( "123" );
    END_OF_FILTER_STREAM;
    apr_pool_destroy( pool );
}

static void
test_huge_tokens( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_token_stream_t* ts;
    lcn_token_t* token;
    char* string;
    
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_char_tokenizer_create( &ts,
                                                   HUGE_STRING,
                                                   pool ) );
    
    NEXT_TOKEN( HUGE_TOKEN );
    NEXT_TOKEN( HUGE_TOKEN );
    NEXT_TOKEN( HUGE_TOKEN );
    END_OF_STREAM;
        
    apr_pool_destroy( pool );

    
}

static void
test_empty_string( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_token_stream_t* ts;
    lcn_token_t* token;
    
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_char_tokenizer_create( &ts,
                                                   "",
                                                   pool ) );

    END_OF_STREAM;

    apr_pool_destroy( pool );
}

static void
test_string( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_token_stream_t* ts;
    lcn_token_t* token;
    char* string;
    
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_char_tokenizer_create( &ts,
                                                   " eins zwei DREI vier f\374nf sechs"
                                                   " abc-123",
                                                   pool ) );

    NEXT_TOKEN( "eins" );
    CHECK_OFFSETS( 1, 5 );
    NEXT_TOKEN( "zwei" );
    CHECK_OFFSETS( 6, 10 );
    NEXT_TOKEN( "DREI" );
    CHECK_OFFSETS( 11, 15 );
    NEXT_TOKEN( "vier" );
    CHECK_OFFSETS( 16, 20 );
    NEXT_TOKEN( "f\374nf" );
    CHECK_OFFSETS( 21, 25 );
    NEXT_TOKEN( "sechs" );
    CHECK_OFFSETS( 26, 31 );
    NEXT_TOKEN( "abc" );
    CHECK_OFFSETS( 32, 35 );
    NEXT_TOKEN( "123" );
    CHECK_OFFSETS( 36, 39 );
    END_OF_STREAM;
    apr_pool_destroy( pool );
}

static void
test_whitespace_tokenizer( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_analyzer_t* analyzer;
    lcn_token_stream_t* ts;
    lcn_token_t* token;
    char* string;
    
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_whitespace_analyzer_create( &analyzer,
                                                        pool ) );
    LCN_TEST( lcn_analyzer_token_stream( analyzer,
                                         &ts,
                                         " eins zwei drei vier fünf sechs"
                                         " abc-123",
                                         pool ) );

    NEXT_TOKEN( "eins" );
    CHECK_OFFSETS( 1, 5 );
    NEXT_TOKEN( "zwei" );
    CHECK_OFFSETS( 6, 10 );
    NEXT_TOKEN( "drei" );
    CHECK_OFFSETS( 11, 15 );
    NEXT_TOKEN( "vier" );
    CHECK_OFFSETS( 16, 20 );
    NEXT_TOKEN( "fünf" );
    CHECK_OFFSETS( 21, 25 );
    NEXT_TOKEN( "sechs" );
    CHECK_OFFSETS( 26, 31 );
    NEXT_TOKEN( "abc-123" );
    CHECK_OFFSETS( 32, 39 );
    END_OF_STREAM;
    apr_pool_destroy( pool );
}

CuSuite *make_char_tokenizer_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, test_huge_tokens );
    SUITE_ADD_TEST( s, test_empty_string );
    SUITE_ADD_TEST( s, test_string );
    SUITE_ADD_TEST( s, test_whitespace_tokenizer );
    SUITE_ADD_TEST( s, test_lowercase_filter );
    return s;
}
