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

static void
add_new_hits( lcn_list_t* results, lcn_list_t* hits, char* entry )
{
    unsigned int i;
    lcn_bool_t is_in_results = LCN_FALSE;

    for( i = 0; i < lcn_list_size( results ); i++ )
    {
        lcn_list_t *h = (lcn_list_t*) lcn_list_get( results, i );
        unsigned int j;

        for( j = 0; j < lcn_list_size( h ); j++ )
        {
            char *e = (char*) lcn_list_get( h, j );

            if ( 0 == strcmp( e, entry ))
            {
                is_in_results = LCN_TRUE;
                break;
            }
        }
    }

    if ( ! is_in_results )
    {
        lcn_list_add( hits, entry );
    }
}

static void
test_ordered_queries( CuTest *tc, lcn_list_t *queries )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;
    unsigned int i, j, k;
    lcn_list_t *results;
    lcn_query_t* o_query;
    lcn_hits_t* hits;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_list_create( &results, 3, pool ));

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_2",
                                                 pool ) );

    /* first collect hits for each query in a result list */
    /* duplicates are removed                             */

    for( i = 0; i < lcn_list_size( queries ); i++ )
    {
        lcn_hits_t *hits;
        lcn_query_t *query = (lcn_query_t*) lcn_list_get( queries, i );
        lcn_list_t *hit_list;
        unsigned int j;

        LCN_TEST( lcn_list_create( &hit_list, 2, pool ));
        LCN_TEST( lcn_searcher_order_by( searcher, LCN_ORDER_BY_NATURAL ));
        LCN_TEST( lcn_searcher_search( searcher, &hits, query, NULL, pool ) );


        for( j = 0; j < lcn_hits_length( hits ); j++ )
        {
            lcn_document_t *doc;
            char *content;

            LCN_TEST( lcn_hits_doc( hits, &doc, j, pool ) );
            LCN_TEST( lcn_document_get( doc, &content, "id", pool ) );
            add_new_hits( results, hit_list, content );
        }

        LCN_TEST( lcn_list_add( results, hit_list ));
    }

    /* build an ordered query */

    LCN_TEST( lcn_ordered_query_create( &o_query, pool ) );

    for( i = 0; i < lcn_list_size( queries ); i++ )
    {
        lcn_query_t *q = (lcn_query_t*) lcn_list_get( queries, i );
        LCN_TEST( lcn_ordered_query_add( o_query, q ));
    }

    /* now search via an ordered query and compare results */

    LCN_TEST( lcn_searcher_order_by( searcher, LCN_ORDER_BY_NATURAL ));
    LCN_TEST( lcn_searcher_search( searcher, &hits, o_query, NULL, pool ) );

    j = k = 0;

    for( i = 0; i < lcn_hits_length( hits ); i++ )
    {
        lcn_document_t *doc;
        char *content, *ref_content;
        lcn_list_t *hit_list;

        LCN_TEST( lcn_hits_doc( hits, &doc, i, pool ) );
        LCN_TEST( lcn_document_get( doc, &content, "id", pool ) );

        hit_list = (lcn_list_t*) lcn_list_get( results, j );

        while( k == lcn_list_size( hit_list ) )
        {
            hit_list = (lcn_list_t*) lcn_list_get( results, ++j );
            k = 0;
        }

        ref_content = (char*) lcn_list_get( hit_list, k++ );
        CuAssertStrEquals( tc, content, ref_content );
    }
}

static void
test_sort_by_query( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_searcher_t* searcher;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_index_searcher_create_by_path( &searcher,
                                                 "test_index_2",
                                                 pool ) );

    {
        lcn_list_t *list;
        lcn_query_t *t_query1, *t_query2, *t_query3;

        LCN_TEST( lcn_term_query_create_by_chars( &t_query1, "text", "phrasequery", pool ));
        LCN_TEST( lcn_term_query_create_by_chars( &t_query2, "text", "booleanquery", pool ));
        LCN_TEST( lcn_term_query_create_by_chars( &t_query3, "text", "and", pool ));

        lcn_query_set_name( t_query1, "first named query" );
        lcn_query_set_name( t_query3, "third named query" );

        LCN_TEST( lcn_list_create( &list, 2, pool ));
        LCN_TEST( lcn_list_add( list, t_query1 ));
        LCN_TEST( lcn_list_add( list, t_query2 ));

        test_ordered_queries( tc, list );

        LCN_TEST( lcn_list_create( &list, 2, pool ));
        LCN_TEST( lcn_list_add( list, t_query2 ));
        LCN_TEST( lcn_list_add( list, t_query1 ));

        test_ordered_queries( tc, list );

        LCN_TEST( lcn_list_create( &list, 2, pool ));
        LCN_TEST( lcn_list_add( list, t_query1 ));
        LCN_TEST( lcn_list_add( list, t_query2 ));
        LCN_TEST( lcn_list_add( list, t_query3 ));

        test_ordered_queries( tc, list );

        LCN_TEST( lcn_list_create( &list, 2, pool ));
        LCN_TEST( lcn_list_add( list, t_query3 ));
        LCN_TEST( lcn_list_add( list, t_query1 ));
        LCN_TEST( lcn_list_add( list, t_query2 ));

        { /* test lcn_ordered_query_name_list */
            lcn_query_t *oq;
            lcn_list_t *name_list;

            LCN_TEST( lcn_ordered_query_create( &oq, pool ) );

            LCN_TEST( lcn_ordered_query_add( oq, t_query1 ));
            LCN_TEST( lcn_ordered_query_add( oq, t_query2 ));
            LCN_TEST( lcn_ordered_query_add( oq, t_query3 ));

            lcn_ordered_query_name_list( oq, &name_list, pool );

            CuAssertStrEquals( tc, "first named query", (char*) lcn_list_get( name_list, 0 ));
            CuAssertStrEquals( tc, "", (char*) lcn_list_get( name_list, 1 ));
            CuAssertStrEquals( tc, "third named query", (char*) lcn_list_get( name_list, 2 ));
        }

        test_ordered_queries( tc, list );
    }

    {
        lcn_list_t *list;
        lcn_query_t *q1, *q2, *q3;
        lcn_term_t* term;

        LCN_TEST( lcn_multi_phrase_query_create( &q1, pool ) );

        lcn_term_create( &term, "text", "an", LCN_TRUE, pool );
        LCN_TEST( lcn_multi_phrase_query_add_term( q1, term ));

        lcn_term_create( &term, "text", "index", LCN_TRUE, pool );
        LCN_TEST( lcn_multi_phrase_query_add_term( q1, term ));

        LCN_TEST( lcn_term_query_create_by_chars( &q2, "text", "an", pool ));
        LCN_TEST( lcn_term_query_create_by_chars( &q3, "text", "index", pool ));

        LCN_TEST( lcn_list_create( &list, 2, pool ));
        LCN_TEST( lcn_list_add( list, q1 ));
        LCN_TEST( lcn_list_add( list, q2 ));
        LCN_TEST( lcn_list_add( list, q3 ));

        test_ordered_queries( tc, list );

        LCN_TEST( lcn_list_create( &list, 2, pool ));
        LCN_TEST( lcn_list_add( list, q2 ));
        LCN_TEST( lcn_list_add( list, q1 ));
        LCN_TEST( lcn_list_add( list, q3 ));

        test_ordered_queries( tc, list );

        LCN_TEST( lcn_list_create( &list, 2, pool ));
        LCN_TEST( lcn_list_add( list, q3 ));
        LCN_TEST( lcn_list_add( list, q2 ));
        LCN_TEST( lcn_list_add( list, q1 ));

        test_ordered_queries( tc, list );
    }

    apr_pool_destroy( pool );
}

static void
test_clone_type( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_query_t* o_query, *clone;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_ordered_query_create( &o_query, pool ));
    CuAssertIntEquals( tc, LCN_QUERY_TYPE_ORDERED, lcn_query_type( o_query ));
    LCN_TEST( lcn_query_clone( o_query, &clone, pool ));
    CuAssertIntEquals( tc, LCN_QUERY_TYPE_ORDERED, lcn_query_type( clone ));

    apr_pool_destroy( pool );
}

CuSuite*
make_ordered_query_suite()
{
    CuSuite* s = CuSuiteNew();

    SUITE_ADD_TEST( s, test_sort_by_query );
    SUITE_ADD_TEST( s, test_clone_type );

    return s;
}
