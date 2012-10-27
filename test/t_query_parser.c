#include "test_all.h"
#include "lcn_search.h"

static void
check_query( CuTest *tc,
             char *qstring,
             apr_pool_t *pool )
{
    apr_pool_t *cp;
    lcn_query_t *query;
    char *qstr;

    LCN_TEST( apr_pool_create( &cp, pool ));
    LCN_TEST( lcn_parse_query( &query, qstring, cp ) );

    LCN_TEST( lcn_query_to_string( query, &qstr, "", cp ));
    CuAssertStrEquals_Msg( tc, qstring, qstring, qstr );

    apr_pool_destroy( cp );
}

static void
check_error( CuTest *tc,
             char *qstring,
             apr_pool_t *pool )
{
    apr_pool_t *cp;
    lcn_query_t *query;
    apr_status_t s;

    LCN_TEST( apr_pool_create( &cp, pool ));
    s = lcn_parse_query( &query, qstring, cp );

    if ( s == LCN_ERR_EMPTY_QUERY )
    {
        return;
    }

    CuAssertIntEquals( tc, LCN_ERR_QUERY_PARSER_SYNTAX_ERROR, s );

    apr_pool_destroy( cp );
}


static void
test_parser(CuTest* tc)
{
    apr_pool_t* p;

    LCN_TEST( apr_pool_create( &p, main_pool ) );

    check_query( tc, "the:call", p );
    check_query( tc, "the:call the:me", p );

    check_query( tc, "the:call -the:me", p );
    check_query( tc, "-the:call the:me", p );
    check_query( tc, "-the:call -the:me", p );

    check_query( tc, "the:call +the:me", p );
    check_query( tc, "+the:call the:me", p );
    check_query( tc, "+the:call +the:me", p );
    check_query( tc, "-the:call +the:me", p );
    check_query( tc, "+the:call -the:me", p );

    check_query( tc, "the:call the:me the:soon", p );
    check_query( tc, "the:call the:me +the:soon", p );
    check_query( tc, "the:call +the:me the:soon", p );
    check_query( tc, "the:call +the:me +the:soon", p );
    check_query( tc, "+the:call the:me the:soon", p );
    check_query( tc, "+the:call the:me +the:soon", p );
    check_query( tc, "+the:call +the:me the:soon", p );
    check_query( tc, "+the:call +the:me +the:soon", p );

    check_query( tc, "-the:call the:me the:soon", p );
    check_query( tc, "-the:call the:me +the:soon", p );
    check_query( tc, "-the:call +the:me the:soon", p );

    check_query( tc, "the:call -the:me the:soon", p );
    check_query( tc, "the:call -the:me -the:soon", p );
    check_query( tc, "the:call -the:me +the:soon", p );
    check_query( tc, "the:call -the:me +the:soon", p );
    check_query( tc, "the:call +the:me -the:soon", p );

    check_query( tc, "-the:call -the:me -the:soon", p );
    check_query( tc, "-the:call -the:me +the:soon", p );
    check_query( tc, "-the:call +the:me the:soon", p );

    check_query( tc, "-the:call +the:me +the:soon", p );
    check_query( tc, "+the:call -the:me the:soon", p );
    check_query( tc, "+the:call -the:me +the:soon", p );
    check_query( tc, "+the:call +the:me -the:soon", p );
    check_query( tc, "+the:call +the:me +the:soon", p );

    check_query( tc, "+the:call +the:me +the:soon", p );
    check_query( tc, "the:call*", p );
    check_query( tc, "the:\"call\"", p );

    check_query( tc, "the:\"call me soon\"", p );
    check_query( tc, "+the:ring (the:\"call me soon\" -the:you)", p );
    check_query( tc, "+the:ring (the:\"call me soon\" -the:you) +the:me*", p );

    apr_pool_destroy( p );
}

static void
test_parser_errors(CuTest* tc)
{
    apr_pool_t* p;

    LCN_TEST( apr_pool_create( &p, main_pool ) );

    check_error( tc, "", p );
    check_error( tc, ".", p );
    check_error( tc, " ", p );
    check_error( tc, "    ", p );
    check_error( tc, "   x", p );

    check_error( tc, "   x:y", p );
    check_error( tc, ":y", p );
    check_error( tc, "?245", p );
    check_error( tc, "abcd", p );
    check_error( tc, "term:á", p );

    apr_pool_destroy( p );
}



CuSuite *make_query_parser_suite (void)
{
    CuSuite *s= CuSuiteNew();
    SUITE_ADD_TEST(s, test_parser );
    SUITE_ADD_TEST(s, test_parser_errors );
    return s;
}
