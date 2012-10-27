#include "lucene.h"
#include "lucene_util.h"
#include "test_all.h"
#include "analysis/char_tokenizer.h"
#include "analysis/lowercase_tokenizer.h"

#define NEXT_TOKEN( TEXT )                              \
{                                                       \
    LCN_TEST( lcn_token_stream_next( ts, &token ) );    \
    lcn_token_term_text( token, &string, pool );        \
    CuAssertStrEquals( tc, TEXT, string );              \
}

#define END_OF_STREAM                                           \
{                                                               \
    CuAssertIntEquals( tc, LCN_ERR_TOKEN_STREAM_EOS,            \
                       lcn_token_stream_next( ts, &token ) );   \
}

static void
test_stem_filter( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_analyzer_t* analyzer;
    lcn_token_stream_t* input;
    lcn_token_stream_t* ts;
    lcn_token_t* token;
    char* string;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );
    LCN_TEST( lcn_analyzer_token_stream( analyzer,
                                         &input,
                                         "Mütter Väter Volker Völker "
                                         "Gebiss Gebiß heissen Gericht "
                                         "Schiffahrt Schifffahrt xxxayyyybffffffcfffd trbs trb ä üß radius radium radiums "
                                         "kenntnisse kenntnisses angebot aufgebot oelform drassstisch",
                                         pool ));

    LCN_TEST( lcn_german_stem_filter_create( &ts,
                                             input,
                                             pool ) );

    NEXT_TOKEN( "mutt" );
    NEXT_TOKEN( "vat" );
    NEXT_TOKEN( "volk" );
    NEXT_TOKEN( "volk" );
    NEXT_TOKEN( "gebiß" );
    NEXT_TOKEN( "gebiß" );
    NEXT_TOKEN( "heis" );
    NEXT_TOKEN( "gerich" );
    NEXT_TOKEN( "schiffahr" );
    NEXT_TOKEN( "schiffahr" );
    NEXT_TOKEN( "xxayybffcffd" );
    NEXT_TOKEN( "trbs" );
    NEXT_TOKEN( "trb" );
    NEXT_TOKEN( "ä" );
    NEXT_TOKEN( "üß" );
    NEXT_TOKEN( "radie" );
    NEXT_TOKEN( "radie" );
    NEXT_TOKEN( "radie" );
    NEXT_TOKEN( "kenntni" );
    NEXT_TOKEN( "kenntni" );
    NEXT_TOKEN( "anb" );
    NEXT_TOKEN( "aufb" );
    NEXT_TOKEN( "olform" );
    NEXT_TOKEN( "draßtisch" );
    END_OF_STREAM;

    apr_pool_destroy( pool );
}

CuSuite *make_german_stem_filter_suite (void)
{
    CuSuite *s= CuSuiteNew();
    SUITE_ADD_TEST( s, test_stem_filter );
    return s;
}
