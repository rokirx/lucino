#include "test_all.h"
#include "lcn_search.h"
#include "lcn_index.h"
#include "lcn_analysis.h"
#include "top_docs.h"
#include "sort_field.h"
#include "field_sorted_hit_queue.h"

static void
test_create_queue( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_field_sorted_hit_queue_t * queue;
    lcn_list_t *sort_fields;
    lcn_sort_field_t *sort_field;
    lcn_index_reader_t *index_reader;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &sort_fields, 1, pool ));
    create_index_by_dump( tc, "sort.txt", "sort_test", pool );
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "sort_test", pool ) );

    LCN_TEST( lcn_sort_field_create ( &sort_field,
                                      "int_field",
                                      LCN_SORT_FIELD_INT,
                                      LCN_FALSE, /* reverse */
                                      pool ));

    LCN_TEST( lcn_list_add( sort_fields, sort_field ));

    LCN_TEST( lcn_field_sorted_hit_queue_create( &queue,
                                                 index_reader,
                                                 sort_fields,
                                                 3,
                                                 pool ));
    apr_pool_destroy( pool );
}

static void
test_sorting( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_field_sorted_hit_queue_t * queue;
    lcn_list_t *sort_fields;
    lcn_sort_field_t *sort_field;
    lcn_index_reader_t *index_reader;
    lcn_score_doc_t d1, d2, d3;
    lcn_score_doc_t *score_doc;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &sort_fields, 1, pool ));
    create_index_by_dump( tc, "sort.txt", "sort_test", pool );
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "sort_test", pool ) );

    LCN_TEST( lcn_sort_field_create ( &sort_field,
                                      "int_field",
                                      LCN_SORT_FIELD_INT,
                                      LCN_FALSE, /* reverse */
                                      pool ));

    LCN_TEST( lcn_list_add( sort_fields, sort_field ));

    LCN_TEST( lcn_field_sorted_hit_queue_create( &queue,
                                                 index_reader,
                                                 sort_fields,
                                                 30,
                                                 pool ));

    d1.doc = 0;
    d2.doc = 1;
    d3.doc = 2;

    lcn_priority_queue_put( lcn_cast_priority_queue( queue ), &d2 );
    lcn_priority_queue_put( lcn_cast_priority_queue( queue ), &d1 );
    lcn_priority_queue_put( lcn_cast_priority_queue( queue ), &d3 );

    score_doc = lcn_priority_queue_pop( lcn_cast_priority_queue( queue ) );
    CuAssertIntEquals( tc, 0, score_doc->doc );

    score_doc = lcn_priority_queue_pop( lcn_cast_priority_queue( queue ) );
    CuAssertIntEquals( tc, 1, score_doc->doc );

    score_doc = lcn_priority_queue_pop( lcn_cast_priority_queue( queue ) );
    CuAssertIntEquals( tc, 2, score_doc->doc );

    apr_pool_destroy( pool );
}



CuSuite* make_field_sorted_hit_queue_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_create_queue );
    SUITE_ADD_TEST( s, test_sorting );

    return s;
}
