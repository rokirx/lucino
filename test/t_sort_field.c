#include "test_all.h"
#include "sort_field.h"
#include "lcn_search.h"
#include "lcn_index.h"
#include "lcn_analysis.h"
#include "top_docs.h"

static void
test_to_string( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_sort_field_t *sort_field;
    char *str;

    apr_pool_create( &pool, main_pool );

    CuAssertIntEquals( tc,
                       LCN_ERR_UNSUPPORTED_OPERATION,
                       lcn_sort_field_create ( &sort_field,
                                               "str_field",
                                               LCN_SORT_FIELD_STRING,
                                               LCN_FALSE,
                                               pool ));

    LCN_TEST( lcn_sort_field_create ( &sort_field,
                                      "int_field",
                                      LCN_SORT_FIELD_INT,
                                      LCN_TRUE,
                                      pool ));

    LCN_TEST( lcn_sort_field_to_string( sort_field, &str, pool ));
    CuAssertStrEquals( tc, "<int>\"int_field\"!", str );

    LCN_TEST( lcn_sort_field_create ( &sort_field,
                                      "int_field",
                                      LCN_SORT_FIELD_INT,
                                      LCN_FALSE,
                                      pool ));

    LCN_TEST( lcn_sort_field_to_string( sort_field, &str, pool ));
    CuAssertStrEquals( tc, "<int>\"int_field\"", str );

    apr_pool_destroy( pool );
}



CuSuite* make_sort_field_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_to_string );

    return s;
}
