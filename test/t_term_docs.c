#include "lcn_index.h"
#include "test_all.h"
#include "lcn_index.h"
#include "term_docs.h"

int doc_freqs[47][2] = {
    { 5,  1 },
    { 6,  1 },
    { 8,  1 },
    { 13, 1 },
    { 18, 1 },
    { 19, 1 },
    { 38, 1 },
    { 47, 1 },
    { 55, 1 },
    { 59, 1 },
    { 70, 1 },
    { 73, 2 },
    { 123, 1 },
    { 124, 1 },
    { 126, 1 },
    { 130, 1 },
    { 133, 1 },
    { 136, 2 },
    { 137, 1 },
    { 141, 1 },
    { 143, 1 },
    { 147, 1 },
    { 156, 2 },
    { 163, 1 },
    { 166, 1 },
    { 171, 1 },
    { 172, 1 },
    { 180, 1 },
    { 202, 1 },
    { 203, 1 },
    { 229, 1 },
    { 244, 1 },
    { 250, 1 },
    { 252, 1 },
    { 253, 1 },
    { 254, 1 },
    { 285, 1 },
    { 290, 1 },
    { 302, 1 },
    { 306, 1 },
    { 307, 2 },
    { 310, 1 },
    { 313, 1 },
    { 317, 1 },
    { 321, 1 },
    { 323, 1 },
    { 328, 1 }
};


static void
check_term_docs_read( CuTest* tc,
                      lcn_index_reader_t *index_reader,
                      apr_pool_t *pool )
{
    apr_pool_t *td_pool;
    lcn_term_docs_t *term_docs;
    lcn_term_t *term;
    unsigned int docs_read;
    lcn_int_array_t* freqs, *docs;
    unsigned int i = 0;


    LCN_TEST( apr_pool_create( &td_pool, pool ));
    LCN_TEST( lcn_term_create( &term, "text", "a", LCN_TERM_TEXT_COPY, td_pool ));
    LCN_TEST( lcn_index_reader_term_docs_from( index_reader, &term_docs, term, td_pool ));
    LCN_TEST( lcn_int_array_create( &freqs, 47, pool ) );
    LCN_TEST( lcn_int_array_create( &docs, 47, pool ) );
    LCN_TEST( lcn_index_reader_term_docs_from( index_reader, &term_docs, term, td_pool ));

    LCN_TEST( lcn_term_docs_read( term_docs, docs, freqs, &docs_read ) );


    for( i = 0; i < 47; i++ )
    {
        CuAssertIntEquals( tc, doc_freqs[i][0], docs->arr[i] );
        CuAssertIntEquals( tc, doc_freqs[i][1], freqs->arr[i] );
    }

    apr_pool_destroy( td_pool );
}

static void
check_term_docs(CuTest* tc,
                lcn_index_reader_t *index_reader,
                apr_pool_t *pool )
{
    {
        apr_pool_t *td_pool;
        lcn_term_docs_t *term_docs;
        lcn_term_t *term;
        apr_status_t next_status = APR_SUCCESS;
        unsigned int i = 0;

        LCN_TEST( apr_pool_create( &td_pool, pool ));
        LCN_TEST( lcn_term_create( &term, "text", "a", LCN_TERM_TEXT_COPY, td_pool ));
        LCN_TEST( lcn_index_reader_term_docs_from( index_reader, &term_docs, term, td_pool ));

        while(APR_SUCCESS == next_status)
        {
            next_status = lcn_term_docs_next( term_docs );

            if ( APR_SUCCESS != next_status )
            {
                break;
            }

            CuAssertIntEquals(tc, doc_freqs[i][0], lcn_term_docs_doc( term_docs ) );
            CuAssertIntEquals(tc, doc_freqs[i][1], lcn_term_docs_freq( term_docs ) );

            i++;
        }

        CuAssertIntEquals(tc, 47, i );

        LCN_TEST( lcn_term_docs_close( term_docs ));

        apr_pool_destroy( td_pool );
    }
}

static void
TestCuSegmentTermDocs(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;
    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "test_index_2", pool ) );

    check_term_docs( tc, index_reader, pool );
    check_term_docs_read( tc, index_reader, pool );

    LCN_TEST(lcn_index_reader_close( index_reader ));
    apr_pool_destroy( pool );
}

static void
TestCuMultiTermDocs(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;
    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "test_index_1", pool ) );

    check_term_docs( tc, index_reader, pool );

    LCN_TEST(lcn_index_reader_close( index_reader ));
    apr_pool_destroy( pool );
}

#define CHECK_POS( d, p )                                                       \
        CuAssertIntEquals(tc, d, lcn_term_docs_doc( term_positions ) );         \
        LCN_TEST( lcn_term_positions_next_position( term_positions, &position ) );   \
        CuAssertIntEquals(tc, p, position )

static void
check_term_pos( CuTest*tc, lcn_index_reader_t *index_reader, apr_pool_t *pool )
{
    apr_pool_t *tp_pool;
    lcn_term_docs_t *term_positions;
    lcn_term_t *term;
    apr_ssize_t position;
    apr_status_t next_status = APR_SUCCESS;

    LCN_TEST( apr_pool_create( &tp_pool, pool ));
    LCN_TEST( lcn_index_reader_term_positions( index_reader, &term_positions, tp_pool ) );
    LCN_TEST( lcn_term_create( &term, "text", "a", LCN_TERM_TEXT_COPY, tp_pool ));
    LCN_TEST( lcn_term_docs_seek_term( term_positions, term ));
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 5, 1 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 6, 1 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 8, 9 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 13, 8 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 18, 3 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 19, 3 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 38, 1 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 47, 2 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 55, 1 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 59, 5 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 70, 9 );
    next_status = lcn_term_docs_next( term_positions );
    CHECK_POS( 73, 1 );
    CHECK_POS( 73, 13 );

    LCN_TEST( lcn_term_docs_close( term_positions ) );
    apr_pool_destroy( tp_pool );
}


static void
TestCuSegmentTermPositions(CuTest* tc)
{

    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "test_index_2", pool ) );

    check_term_pos( tc, index_reader, pool );

    LCN_TEST( lcn_index_reader_close( index_reader ) );
    apr_pool_destroy( pool );
}

static void
TestCuMultiTermPositions(CuTest* tc)
{

    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "test_index_1", pool ) );

    check_term_pos( tc, index_reader, pool );

    LCN_TEST( lcn_index_reader_close( index_reader ) );
    apr_pool_destroy( pool );
}

static void
test_segment_term_docs_read( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_index_reader_t* reader;
    lcn_term_docs_t* td;
    lcn_term_t* term;

    lcn_int_array_t* docs, *freqs;
    unsigned int read_entries;
    int i;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_int_array_create( &docs, 32, pool ) );
    LCN_TEST( lcn_int_array_create( &freqs, 32, pool ) );
    LCN_TEST( lcn_term_create( &term, "text", "a", 1, pool ) );
    LCN_TEST( lcn_index_reader_create_by_path( &reader,
                                               "test_index_2",
                                               pool ) );

    LCN_TEST( lcn_index_reader_term_docs_from( reader,
                                               &td,
                                               term,
                                               pool ) );

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 32, read_entries );

    for( i = 0; i < 32; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[i][1], freqs->arr[i] );
    }

    LCN_TEST( lcn_int_array_create( &docs, 100, pool ) );
    LCN_TEST( lcn_int_array_create( &freqs, 100, pool ) );

    LCN_TEST( lcn_index_reader_term_docs_from( reader,
                                               &td,
                                               term,
                                               pool ) );

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 47, read_entries );

    for( i = 0; i < 47; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[i][1], freqs->arr[i] );
    }

    apr_pool_destroy( pool );
}

static void
test_multi_term_docs_read( CuTest* tc )
{
    apr_pool_t* pool;
    lcn_index_reader_t* reader;
    lcn_term_docs_t* td;
    lcn_term_t* term;

    lcn_int_array_t* docs, *freqs;
    unsigned int read_entries;
    int i;

    apr_pool_create( &pool, main_pool );

    LCN_TEST( lcn_int_array_create( &docs, 32, pool ) );
    LCN_TEST( lcn_int_array_create( &freqs, 32, pool ) );

    LCN_TEST( lcn_term_create( &term, "text", "a", LCN_TERM_TEXT_COPY, pool ) );
    LCN_TEST( lcn_index_reader_create_by_path( &reader,
                                               "test_index_1",
                                               pool ) );

    LCN_TEST( lcn_index_reader_term_docs_from( reader,
                                               &td,
                                               term,
                                               pool ) );

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 12, read_entries );

    for( i = 0; i < 12; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[i][1], freqs->arr[i] );
    }

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 10, read_entries );

    for( i = 0; i < 10; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[12+i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[12+i][1], freqs->arr[i] );
    }

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 9, read_entries );

    for( i = 0; i < 9; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[22+i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[22+i][1], freqs->arr[i] );
    }

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 10, read_entries );

    for( i = 0; i < 10; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[31+i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[31+i][1], freqs->arr[i] );
    }

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 3, read_entries );

    for( i = 0; i < 3; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[41+i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[41+i][1], freqs->arr[i] );
    }

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 3, read_entries );

    for( i = 0; i < 3; i++ )
    {
        CuAssertIntEquals(tc, doc_freqs[44+i][0], docs->arr[i] );
        CuAssertIntEquals(tc, doc_freqs[44+i][1], freqs->arr[i] );
    }

    LCN_TEST( lcn_term_docs_read( td,
                                  docs,
                                  freqs,
                                  &read_entries ) );

    CuAssertIntEquals(tc, 0, read_entries );

    apr_pool_destroy( pool );
}


CuSuite *make_term_docs_suite (void)
{
    CuSuite *s= CuSuiteNew();

    SUITE_ADD_TEST( s, TestCuSegmentTermDocs);
    SUITE_ADD_TEST( s, TestCuMultiTermDocs);
    SUITE_ADD_TEST( s, TestCuSegmentTermPositions);
    SUITE_ADD_TEST( s, TestCuMultiTermPositions);
    SUITE_ADD_TEST( s, test_segment_term_docs_read );
    SUITE_ADD_TEST( s, test_multi_term_docs_read );

    return s;
}
