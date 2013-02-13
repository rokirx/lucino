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

    CuAssertTrue(tc, strcmp( "z", "\304" ) < 0); /* Auml */
    CuAssertTrue(tc, strcmp( "\304", "z" ) > 0);

    CuAssertTrue(tc, strcmp( "\304", "\326" ) < 0); /* Auml Ouml */
    CuAssertTrue(tc, strcmp( "\326", "\304" ) > 0);

    CuAssertTrue(tc, strcmp( "\326", "\334" ) < 0);
    CuAssertTrue(tc, strcmp( "\334", "\326" ) > 0);

    CuAssertTrue(tc, strcmp( "\334", "\337" ) < 0);
    CuAssertTrue(tc, strcmp( "\337", "\334" ) > 0);

    CuAssertTrue(tc, strcmp( "\337", "\344") < 0);
    CuAssertTrue(tc, strcmp( "\344", "\337" ) > 0);

    CuAssertTrue(tc, strcmp( "\344", "\366" ) < 0);
    CuAssertTrue(tc, strcmp( "\366", "\344" ) > 0);

    CuAssertTrue(tc, strcmp( "\366", "\374" ) < 0);
    CuAssertTrue(tc, strcmp( "\374", "\366" ) > 0);

    apr_pool_destroy( p );
}

CuSuite *make_term_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST(s,TestCuTerm);

    return s;
}
