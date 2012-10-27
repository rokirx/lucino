#include "test_all.h"
#include "lucene.h"

static void
TestCuTerm(CuTest* tc)
{
    apr_pool_t *p;
    lcn_term_t *t1;
    lcn_term_t *t2;
    char text1[] = "test";
    char text2[] = "sest";
    
    apr_pool_create( &p, main_pool );

    LCN_TEST( lcn_term_create( &t1, "xid", text1, LCN_TERM_TEXT_COPY, p) );
    LCN_TEST( lcn_term_create( &t2, "xid", text2, LCN_TERM_NO_TEXT_COPY, p) );

    CuAssertStrEquals( tc, "test", lcn_term_text( t1 ) );
    CuAssertStrEquals( tc, "sest", lcn_term_text( t2 ) );
    CuAssertTrue( tc, text2 == lcn_term_text( t2 ) );
    CuAssertTrue( tc, text1 != lcn_term_text( t1 ) );

    CuAssertStrEquals( tc, "xid", lcn_term_field( t1 ) );
    CuAssertStrEquals( tc, "xid", lcn_term_field( t2 ) );
    CuAssertTrue( tc, lcn_term_field( t1 ) == lcn_term_field( t2 ) );

    CuAssertTrue(tc, lcn_term_compare( t1, t2 ) == 1);

    CuAssertTrue(tc, strcmp( "1", "9" ) < 0);
    CuAssertTrue(tc, strcmp( "9", "1" ) > 0);

    CuAssertTrue(tc, strcmp( "9", "A" ) < 0);
    CuAssertTrue(tc, strcmp( "A", "9" ) > 0);

    CuAssertTrue(tc, strcmp( "A", "Z" ) < 0);
    CuAssertTrue(tc, strcmp( "Z", "A" ) > 0);

    CuAssertTrue(tc, strcmp( "Z", "a" ) < 0);
    CuAssertTrue(tc, strcmp( "a", "Z" ) > 0);

    CuAssertTrue(tc, strcmp( "a", "z" ) < 0);
    CuAssertTrue(tc, strcmp( "z", "a" ) > 0);

    CuAssertTrue(tc, strcmp( "z", "Ä" ) < 0);
    CuAssertTrue(tc, strcmp( "Ä", "z" ) > 0);

    CuAssertTrue(tc, strcmp( "Ä", "Ö" ) < 0);
    CuAssertTrue(tc, strcmp( "Ö", "Ä" ) > 0);

    CuAssertTrue(tc, strcmp( "Ö", "Ü" ) < 0);
    CuAssertTrue(tc, strcmp( "Ü", "Ö" ) > 0);

    CuAssertTrue(tc, strcmp( "Ü", "ß" ) < 0);
    CuAssertTrue(tc, strcmp( "ß", "Ü" ) > 0);

    CuAssertTrue(tc, strcmp( "ß", "ä" ) < 0);
    CuAssertTrue(tc, strcmp( "ä", "ß" ) > 0);

    CuAssertTrue(tc, strcmp( "ä", "ö" ) < 0);
    CuAssertTrue(tc, strcmp( "ö", "ä" ) > 0);

    CuAssertTrue(tc, strcmp( "ö", "ü" ) < 0);
    CuAssertTrue(tc, strcmp( "ü", "ö" ) > 0);

    apr_pool_destroy( p );
}

CuSuite *make_term_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s,TestCuTerm);

    return s;
}
