#include "test_all.h"
#include "lcn_search.h"
#include "query_tokenizer.h"
#include "query_parser.h"

#define READ(tid, tname ) \
    CuAssertIntEquals( tc, APR_SUCCESS, lcn_query_tokenizer_next_token( t, &token_id, &token ) ); \
    CuAssertIntEquals_Msg( tc, "check token_id", tid, token_id ); \
    CuAssertStrEquals( tc, tname, token.text )

static void
test_tokenizer(CuTest* tc)
{
    lcn_query_tokenizer_t *t;
    int token_id;
    lcn_query_token_t token;
    apr_pool_t *p;
    char *eofbuf, *text;

    LCN_TEST( apr_pool_create( &p, main_pool ) );

    LCN_TEST( lcn_query_tokenizer_create( &t, (text = apr_pstrdup(p,"the:call")), p ));
    eofbuf = text + strlen(text);
    READ( LCNQ_FIELD_PREFIX, "the"  );
    READ( LCNQ_SEP,          ":"    );
    READ( LCNQ_TERM_TEXT,    "call" );
    CuAssertIntEquals( tc, LCN_ERR_ITERATOR_NO_NEXT, lcn_query_tokenizer_next_token( t, &token_id, &token ) );

    LCN_TEST( lcn_query_tokenizer_create( &t, (text = apr_pstrdup(p,"+the:call +the:ring -the:home")), p ));
    eofbuf = text + strlen(text);
    READ( LCNQ_CLAUSE, "+"  );
    READ( LCNQ_FIELD_PREFIX, "the"  );
    READ( LCNQ_SEP,          ":"    );
    READ( LCNQ_TERM_TEXT,    "call" );
    READ( LCNQ_SPACE,    " " );
    READ( LCNQ_CLAUSE, "+"  );
    READ( LCNQ_FIELD_PREFIX, "the"  );
    READ( LCNQ_SEP,          ":"    );
    READ( LCNQ_TERM_TEXT,    "ring" );
    READ( LCNQ_SPACE,    " " );
    READ( LCNQ_CLAUSE, "-"  );
    READ( LCNQ_FIELD_PREFIX, "the"  );
    READ( LCNQ_SEP,          ":"    );
    READ( LCNQ_TERM_TEXT,    "home" );

    CuAssertIntEquals( tc, LCN_ERR_ITERATOR_NO_NEXT, lcn_query_tokenizer_next_token( t, &token_id, &token ) );

    apr_pool_destroy( p );
}

CuSuite *
make_query_tokenizer_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s, test_tokenizer );

    return s;

}
