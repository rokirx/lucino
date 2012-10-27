#include "test_all.h"
#include "score_doc_comparator.h"
#include "lcn_analysis.h"

static void
test_int_comparator( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;
    lcn_score_doc_comparator_t *comparator;
    lcn_score_doc_t *sa, *sb;
    lcn_score_t s1, s2;

    s1.float_val = 1.0;
    s2.float_val = 1.0;

    apr_pool_create( &pool, main_pool );

    create_index_by_dump( tc, "sort.txt", "sort_test", pool );

    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "sort_test", pool ) );

    LCN_TEST( lcn_score_doc_int_comparator_create( &comparator, index_reader, "int_field", 0, pool ));

    LCN_TEST( lcn_score_doc_create( &sa, 5, s1, pool ));
    LCN_TEST( lcn_score_doc_create( &sb, 6, s2, pool ));



    CuAssertIntEquals(tc, -1, lcn_score_doc_comparator_compare( comparator, sa, sb ));

    sa->doc = 1; sb->doc = 2;
    CuAssertIntEquals(tc, 1, lcn_score_doc_comparator_compare( comparator, sa, sb ));

    sa->doc = 1; sb->doc = 9;
    CuAssertIntEquals(tc, 1, lcn_score_doc_comparator_compare( comparator, sa, sb ));

    sa->doc = 10; sb->doc = 9;
    CuAssertIntEquals(tc, 0, lcn_score_doc_comparator_compare( comparator, sa, sb ));

    LCN_TEST( lcn_index_reader_close( index_reader ));

    apr_pool_destroy( pool );
}



CuSuite* make_score_doc_comparator_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_int_comparator );

    return s;
}
