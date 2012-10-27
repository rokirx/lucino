#include "test_all.h"
#include "lcn_search.h"
#include "lcn_bitvector.h"


#define CHECK_HIT( HITS, NTH, FIELD, TEXT )                     \
{                                                               \
    lcn_document_t* doc;                                        \
    char* content;                                              \
                                                                \
    LCN_TEST( lcn_hits_doc( HITS, &doc, NTH, pool ) );          \
    LCN_TEST( lcn_document_get( doc, &content, FIELD, pool ) ); \
    CuAssertStrEquals( tc, TEXT, content );                     \
}

static lcn_bool_t
test_less_than( lcn_priority_queue_t* pq, void* a, void* b )
{
    lcn_score_doc_t *score_a = (lcn_score_doc_t*) a;
    lcn_score_doc_t *score_b = (lcn_score_doc_t*) b;

    if ( score_a->doc == 13 )
    {
        return 0;
    }

    if ( score_b->doc == 13 )
    {
        return 1;
    }

    return score_a->doc > score_b->doc;
}


static void
test_sort_by_bitvector_with_hc( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    lcn_query_t* query;
    lcn_hits_t* hits;
    lcn_hit_queue_t *hit_queue;
    lcn_priority_queue_t *pq;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher, "test_index_2", pool ) );
    LCN_TEST( lcn_term_query_create_by_chars( &query, "text", "a", pool ));

    LCN_TEST( lcn_hit_queue_create( &hit_queue, 50, pool ));

    pq = (lcn_priority_queue_t*) hit_queue;
    pq->less_than = test_less_than;

    LCN_TEST( lcn_searcher_search_custom_hit_queue( searcher, &hits, query, NULL, hit_queue, pool ));

    CuAssertIntEquals( tc, 47, lcn_hits_length( hits ) );

    CHECK_HIT( hits, 0, "id", "KW13" );
    CHECK_HIT( hits, 1, "id", "KW5" );
    CHECK_HIT( hits, 2, "id", "KW6" );
    CHECK_HIT( hits, 3, "id", "KW8" );

    lcn_index_searcher_close( searcher );

    apr_pool_destroy( pool );
}

CuSuite*
make_custom_hit_queue_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_sort_by_bitvector_with_hc );

    return s;
}
