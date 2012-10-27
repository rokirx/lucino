#include "test_all.h"
#include "top_doc_collector.h"

#define COLLECT( ORDER, FLOAT_VAL )                     \
{                                                       \
    score.float_val = FLOAT_VAL;                        \
    lcn_hit_collector_collect( tdc, ORDER, score );     \
}


static void
test_top_doc_collector( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_hit_collector_t* tdc;
    lcn_top_docs_t* top_docs;
    lcn_score_t score;
    lcn_hit_queue_t *hq;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    LCN_TEST( lcn_hit_queue_create( &hq, 10, pool ) );
    LCN_TEST( lcn_top_doc_collector_create_with_hit_queue( &tdc, 10, hq, pool ));

    COLLECT( 0, 5.7f );
    COLLECT( 1, 84.f );
    COLLECT( 3, 0.01f );
    COLLECT( 4, 3.9f );
    COLLECT( 5, 0.1f );
    COLLECT( 6, 3.6f );
    COLLECT( 2, 93.0f );
    COLLECT( 7, 9.1f );
    COLLECT( 8, 29.7f );
    COLLECT( 0, 5.7f );

    lcn_top_doc_collector_top_docs( tdc,
                                    &top_docs,
                                    pool );
    CuAssertDblEquals( tc, 93.f, top_docs->max_score.float_val, 0 );

}

CuSuite*
make_top_doc_collector_suite( void )
{
  CuSuite* s = CuSuiteNew();

  SUITE_ADD_TEST( s, test_top_doc_collector );

  return s;
}
