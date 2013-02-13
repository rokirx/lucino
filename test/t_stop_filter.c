#include "lucene.h"
#include "lucene_util.h"
#include "test_all.h"
#include "lcn_util.h"
#include "analysis/stop_filter.h"
#include "analysis/char_tokenizer.h"

#define NEXT_TOKEN( TEXT )                                              \
  {                                                                     \
    LCN_TEST( lcn_stop_filter_next( stop_filter, &token ) );            \
    lcn_token_term_text( token, &string, pool );                        \
    CuAssertStrEquals( tc, TEXT, string );                              \
  }

#define END_OF_STREAM                                                   \
  {                                                                     \
    CuAssertIntEquals( tc, LCN_ERR_TOKEN_STREAM_EOS,                    \
                       lcn_stop_filter_next( stop_filter, &token ) );   \
  }

static void
test_cu_empty_string( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_token_stream_t* ts;
    lcn_stop_filter_t* stop_filter;
    lcn_token_t* token;
    lcn_list_t* stop_words;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &stop_words, 10, pool ) );

    lcn_list_add( stop_words, "zwei" );
    lcn_list_add( stop_words, "vier" );


    LCN_TEST( lcn_char_tokenizer_create( &ts,
                                                   "",
                                                   pool ) );


    LCN_TEST( lcn_stop_filter_create_list( &stop_filter,
                                                ts,
                                                stop_words,
                                                pool ) );

    END_OF_STREAM;

    apr_pool_destroy( pool );
}

static void
test_stopword_filter( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_token_stream_t* ts;
    lcn_stop_filter_t* stop_filter;
    lcn_token_t* token;
    lcn_list_t* stop_words;
    char* string;
    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &stop_words, 10, pool ) );

    lcn_list_add( stop_words, "zwei" );
    lcn_list_add( stop_words, "vier" );


    LCN_TEST( lcn_char_tokenizer_create( &ts,
                                                   " eins zwei drei vier"
                                                   " f\374nf sechs",
                                                   pool ) );


    LCN_TEST( lcn_stop_filter_create_list( &stop_filter,
                                                ts,
                                                stop_words,
                                                pool ) );

    
    NEXT_TOKEN( "eins" );
    NEXT_TOKEN( "drei" );
    NEXT_TOKEN( "f\374nf" );
    NEXT_TOKEN( "sechs" );
    END_OF_STREAM;

    apr_pool_destroy( pool );
}

static void
test_all_tokens_filtered( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_token_stream_t* ts;
    lcn_stop_filter_t* stop_filter;
    lcn_token_t* token;
    
    char* stop_words[] = { "eins", "zwei", "drei", "vier", "f\374nf", "sechs", 0 };

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_char_tokenizer_create( &ts,
                                         " eins zwei drei vier"
                                         " f\374nf sechs",
                                         pool ) );


    LCN_TEST( lcn_stop_filter_create_argv( &stop_filter,
                                                     ts,
                                                     stop_words,
                                                     pool ) );

    
    END_OF_STREAM;

    apr_pool_destroy( pool );
}

CuSuite *make_stop_filter_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, test_stopword_filter );
    SUITE_ADD_TEST( s, test_cu_empty_string );
    SUITE_ADD_TEST( s,test_all_tokens_filtered );
    return s;
}

