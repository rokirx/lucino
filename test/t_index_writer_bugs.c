#define _GNU_SOURCE
#include "test_all.h"
#include "lucene.h"
#include "lcn_index.h"
#include "lcn_util.h"
#include "lcn_analysis.h"


static void
test_indexing_segfault(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_writer_t *index_writer;
    lcn_document_t *document;
    lcn_field_t *field;
    lcn_field_type_t indexed_type = {0};
    lcn_analyzer_t *analyzer;
    char *buf;
    unsigned int i;

    delete_files( tc, "test_index_writer" );

    LCN_TEST( apr_pool_create( &pool, main_pool ) );

    buf = (char*) apr_pcalloc(pool, 10000 * sizeof(char) );

    for( i = 0; i < 9999; i++ )
    {
        buf[i] = 'x';
    }

    buf[9999] = '\0';

    CuAssertIntEquals(tc, 9999, strlen(buf));

    LCN_TEST( lcn_index_writer_create_by_path( &index_writer, "test_index_writer", LCN_TRUE, pool ) );
    LCN_TEST( lcn_simple_analyzer_create( &analyzer, pool ) );
    LCN_TEST( lcn_field_type_set_indexed( &indexed_type, LCN_TRUE ));
    LCN_TEST( lcn_document_create( &document, pool ) );

    LCN_TEST( lcn_field_create( &field,
                                "titel",
                                "FC",
                                &indexed_type,
                                pool ));
    LCN_TEST( lcn_document_add_field( document, field ));
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );


    LCN_TEST( lcn_document_create( &document, pool ) );
    LCN_TEST( lcn_field_create( &field,
                                "titel",
                                buf,
                                &indexed_type,
                                pool ) );
    LCN_TEST( lcn_document_add_field( document, field ));
    LCN_TEST( lcn_index_writer_add_document( index_writer, document ) );

    LCN_TEST( lcn_index_writer_close( index_writer ) );
    LCN_TEST( lcn_index_writer_optimize( index_writer ) );

    delete_files( tc, "test_index_writer" );

    apr_pool_destroy( pool );
}


CuSuite *make_index_writer_bugs_suite (void)
{
    CuSuite *s= CuSuiteNew();
    SUITE_ADD_TEST(s, test_indexing_segfault);
    return s;
}
