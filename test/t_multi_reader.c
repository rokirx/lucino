#define _GNU_SOURCE
#include "test_all.h"
#include "lucene.h"
#include "lcn_index.h"
#include "lcn_util.h"
#include "lcn_analysis.h"
#include "lcn_search.h"

static void
compare_index_readers( CuTest *tc,
                       lcn_index_reader_t *r_a,
                       lcn_index_reader_t *r_b,
                       apr_pool_t *pool )
{
    lcn_term_enum_t *te_a, *te_b;
    lcn_term_docs_t *tp_a, *tp_b;
    apr_pool_t *p;

    LCN_TEST( apr_pool_create( &p, pool ));

    /* check equality of term enumerations */

    LCN_TEST( lcn_index_reader_terms( r_a, &te_a, pool ));
    LCN_TEST( lcn_index_reader_terms( r_b, &te_b, pool ));

    LCN_TEST( lcn_index_reader_term_positions( r_a, &tp_a, pool ));
    LCN_TEST( lcn_index_reader_term_positions( r_b, &tp_b, pool ));

    while( APR_SUCCESS == lcn_term_enum_next( te_a ))
    {
        apr_status_t x;
        const lcn_term_t *t;

        CuAssertIntEquals( tc, APR_SUCCESS, lcn_term_enum_next( te_b ));

        CuAssertStrEquals(tc,
                          lcn_term_field( lcn_term_enum_term( te_a )),
                          lcn_term_field( lcn_term_enum_term( te_b )) );

        CuAssertStrEquals(tc,
                          lcn_term_text( lcn_term_enum_term( te_a )),
                          lcn_term_text( lcn_term_enum_term( te_b )) );

        t = lcn_term_enum_term( te_a );
        //fprintf(stderr, "term %s:%s\n", lcn_term_field(t), lcn_term_text(t));

        LCN_TEST( lcn_term_docs_seek_term( tp_a, lcn_term_enum_term( te_a ) ));
        LCN_TEST( lcn_term_docs_seek_term( tp_b, lcn_term_enum_term( te_b ) ));

        x = lcn_term_docs_next( tp_a );

        while( APR_SUCCESS == x )
        {
            apr_ssize_t pa, pb;
            apr_ssize_t freq;

            LCN_TEST( lcn_term_docs_next( tp_b ));
#if 0
            fprintf(stderr, "<doc:freq>: %d:%d = %d:%d\n",
                    lcn_term_docs_doc( tp_a ),
                    lcn_term_docs_freq( tp_a ),
                    lcn_term_docs_doc( tp_b ),
                    lcn_term_docs_freq( tp_b ));
#endif
            CuAssertIntEquals(tc, lcn_term_docs_doc( tp_a ), lcn_term_docs_doc( tp_b ));
            CuAssertIntEquals(tc, lcn_term_docs_freq( tp_a ), lcn_term_docs_freq( tp_b ));

            for( freq = lcn_term_docs_freq( tp_a );
                 freq > 0;
                 freq-- )
            {
                LCN_TEST( lcn_term_positions_next_position( tp_a, &pa ));
                LCN_TEST( lcn_term_positions_next_position( tp_b, &pb ));

                //fprintf(stderr, "   pos %d<->%d\n", pa, pb );
                CuAssertIntEquals( tc, pa, pb );
            }

            x = lcn_term_docs_next( tp_a );
        }
    }

    CuAssertIntEquals(tc, LCN_ERR_ITERATOR_NO_NEXT, lcn_term_enum_next( te_b ));

    apr_pool_destroy( p );
}

static void
test_doc_ids( CuTest *tc )
{
    apr_pool_t *pool;
    lcn_index_reader_t *r1, *r2, *r;
    lcn_index_reader_t *m;
    lcn_list_t *list;
    lcn_searcher_t *s1, *s2;
    lcn_query_t *q;
    lcn_hits_t *h1, *h2;
    lcn_document_t *d1, *d2;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( lcn_list_create( &list, 10, pool ) );

    create_index( tc,   0, 80, "multi_index_part_1", LCN_FALSE, pool );
    create_index( tc, 80, 160, "multi_index_part_2", LCN_FALSE, pool );
    create_index( tc,  0, 160, "multi_index",        LCN_FALSE, pool );

    LCN_TEST( lcn_index_reader_create_by_path( &r1, "multi_index_part_1"  , pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &r2, "multi_index_part_2"  , pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &r,  "multi_index"         , pool ));

    LCN_TEST( lcn_list_add( list, r1 ));
    LCN_TEST( lcn_list_add( list, r2 ));

    LCN_TEST( lcn_multi_reader_create_by_sub_readers( &m, list, pool ));

    LCN_TEST( lcn_term_query_create_by_chars( &q, "text", "programmatically", pool ));

    LCN_TEST( lcn_index_searcher_create_by_reader( &s1, m, pool ));
    LCN_TEST( lcn_index_searcher_create_by_reader( &s2, r, pool ));

    LCN_TEST( lcn_searcher_search( s1, &h1, q, NULL, pool ));
    LCN_TEST( lcn_searcher_search( s2, &h2, q, NULL, pool ));

    LCN_TEST( lcn_hits_doc( h1, &d1, 0, pool ) );
    LCN_TEST( lcn_hits_doc( h2, &d2, 0, pool ) );

    CuAssertIntEquals( tc, 97, lcn_document_id( d1 ));
    CuAssertIntEquals( tc, 97, lcn_document_id( d2 ));

    LCN_TEST( lcn_index_reader_close( m ));
    LCN_TEST( lcn_index_reader_close( r ));

    apr_pool_destroy( pool );
}

static void
test_multi_reader( CuTest *tc )
{
    apr_pool_t *pool;
    lcn_index_reader_t *r1, *r2, *r, *or1, *or2, *or;
    lcn_index_reader_t *m;
    lcn_list_t *list;

    LCN_TEST( apr_pool_create( &pool, main_pool ));

    LCN_TEST( lcn_list_create( &list, 10, pool ) );

    create_index( tc,   0, 80, "multi_index_part_1", LCN_FALSE, pool );
    create_index( tc, 80, 160, "multi_index_part_2", LCN_FALSE, pool );
    create_index( tc,  0, 160, "multi_index",        LCN_FALSE, pool );

    create_index( tc,   0, 80, "o_multi_index_part_1", LCN_TRUE, pool );
    create_index( tc, 80, 160, "o_multi_index_part_2", LCN_TRUE, pool );
    create_index( tc,  0, 160, "o_multi_index",        LCN_TRUE, pool );

    LCN_TEST( lcn_index_reader_create_by_path( &r1, "multi_index_part_1"  , pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &r2, "multi_index_part_2"  , pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &r,  "multi_index"         , pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &or1, "o_multi_index_part_1", pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &or2, "o_multi_index_part_2", pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &or, "o_multi_index"       , pool ));

    compare_index_readers( tc, r1, or1, pool );
    compare_index_readers( tc, r2, or2, pool );
    compare_index_readers( tc, r,  or,  pool );
    

    LCN_TEST( lcn_list_add( list, or1 ));
    LCN_TEST( lcn_list_add( list, or2 ));

    LCN_TEST( lcn_multi_reader_create_by_sub_readers( &m, list, pool ));

    compare_index_readers( tc, or, m, pool );
    compare_index_readers( tc,  r, m, pool );
    compare_index_readers( tc, or, r, pool );

    LCN_TEST( lcn_list_add( list, or1 ));
    LCN_TEST( lcn_list_add( list, r2 ));

    LCN_TEST( lcn_multi_reader_create_by_sub_readers( &m, list, pool ));

    compare_index_readers( tc, or, m, pool );
    compare_index_readers( tc,  r, m, pool );

    apr_pool_destroy( pool );
}

static void
test_multi_seek_enum( CuTest* tc )
{
    apr_pool_t *pool;
    lcn_index_reader_t *reader;
    lcn_term_t *term;
    lcn_term_enum_t *term_enum;
    apr_status_t next;
    lcn_term_docs_t *term_docs = NULL;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_reader_create_by_path( &reader, "multi_index", pool ));
    LCN_TEST( lcn_index_reader_term_docs( reader, &term_docs, pool ));
    LCN_TEST( lcn_term_create ( &term, "", "", LCN_TERM_NO_TEXT_COPY, pool ));
    LCN_TEST( lcn_index_reader_terms_from( reader, &term_enum, term, pool ));

    while( APR_SUCCESS == (next = lcn_term_enum_next( term_enum )))
    {
        lcn_term_t *t;
        t = (lcn_term_t*)lcn_term_enum_term( term_enum );
        LCN_TEST( lcn_term_docs_seek_term_enum( term_docs, term_enum ));
    }

    LCN_TEST( lcn_term_docs_close( term_docs ));
    LCN_TEST( lcn_term_enum_close( term_enum ));
    LCN_TEST( lcn_index_reader_close( reader ));

    apr_pool_destroy( pool );
}



CuSuite *make_multi_reader_suite (void)
{
    CuSuite *s= CuSuiteNew();
    //SUITE_ADD_TEST( s, test_multi_reader );
    SUITE_ADD_TEST( s, test_doc_ids );
    //SUITE_ADD_TEST( s, test_multi_seek_enum );
    return s;
}
